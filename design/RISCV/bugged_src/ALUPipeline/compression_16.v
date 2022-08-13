module Decompressor(inst_16,
                    inst_32
    );
    /* ==================== IO ==================== */
    input      [15:0] inst_16;
    output reg [31:0] inst_32;
    /* ================= WIRE/REG ================= */
    wire [1:0] C     ;
    wire [2:0] fun3  ;

    wire [2:0] rd_rs2;
    wire [2:0] rs1   ;
    wire [6:0] imm_ls;

    wire       imm_12;
    wire [4:0] imm_rs2;
    wire [4:0] rd_rs1;
    wire       imm_rs2_eqz;
    wire [1:0] sel_comb;
    wire [20:1]jal_addr;
    wire [12:1]bra_addr;
    wire [6:0] IOP,SBOP,UJOP;
    /* ================== Conti =================== */
    assign C      = inst_16[1:0];
    assign fun3   = inst_16[15:13];
    //C0 DECODER
    assign rd_rs2 = inst_16[4:2];
    assign rs1    = inst_16[9:7];
    assign imm_ls = {inst_16[5],inst_16[12:10],inst_16[6],2'b0};

    //C2 DECODER
    assign imm_12 = inst_16[12];
    assign imm_rs2= inst_16[6:2];
    assign rd_rs1 = inst_16[11:7];
    assign imm_rs2_eqz = ~|(imm_rs2);
    assign sel_comb = {imm_12,imm_rs2_eqz};

    assign jal_addr = {{10{inst_16[12]}},inst_16[8],inst_16[10:9],inst_16[6],inst_16[7],inst_16[2],inst_16[11],inst_16[5:3]};
    assign bra_addr = {{5{inst_16[12]}},inst_16[6:5],inst_16[2],inst_16[11:10],inst_16[4:3]};

    //opcode
    assign IOP  = 7'b0010011;
    assign SBOP = 7'b1100011;
    assign UJOP = 7'b1100111;
    /* ================ Combination =============== */
    always @(*) begin
        case (C)
            2'b00:begin  //C0 for C.LW or C.SW
                if (fun3[2]==1'b0) inst_32 = {{5'b0,imm_ls},{2'b01,rs1},3'b010,{2'b01,rd_rs2},7'b0000011};   //LW(I)
                else inst_32 = {{5'b0,imm_ls[6:5]},{2'b01,rd_rs2},{2'b01,rs1},3'b010,imm_ls[4:0],7'b0100011};//SW(S)
            end 
            2'b01:begin  //C1
                case (fun3)
                    3'b000:begin
                        if (imm_12) inst_32 = {{{7{imm_12}},imm_rs2},rd_rs1,3'b000,rd_rs1,IOP};//ADDI(I)
                        else inst_32 = {25'b0,IOP};                                            //NOP = ADDI(I) {12'b0,5'b0,3'b000,5'b0,7'b0010011}
                    end
                    3'b001:inst_32 = {jal_addr[20],jal_addr[10:1],jal_addr[11],jal_addr[19:12],5'b1,UJOP};                 //JAL(UJ)
                    3'b101:inst_32 = {jal_addr[20],jal_addr[10:1],jal_addr[11],jal_addr[19:12],5'b0,UJOP};                 //J = jal(UJ)
                    3'b110:inst_32 = {bra_addr[12],bra_addr[10:5],5'b0,{2'b01,rs1},3'b000,bra_addr[4:1],bra_addr[11],SBOP};//BEQZ(SB)
                    3'b111:inst_32 = {bra_addr[12],bra_addr[10:5],5'b0,{2'b01,rs1},3'b001,bra_addr[4:1],bra_addr[11],SBOP};//BNEZ(SB)
                    3'b100:begin
                        case (rd_rs1[4:3])
                            2'b00:inst_32 = {7'b0000000,imm_rs2,{2'b01,rs1},3'b101,{2'b01,rs1},IOP};    //SRLI be care for immed (I)
                            2'b01:inst_32 = {7'b0100000,imm_rs2,{2'b01,rs1},3'b101,{2'b01,rs1},IOP};    //SRAI be care for immed (I)        
                            2'b10:inst_32 = {{{7{imm_12}},imm_rs2},{2'b01,rs1},3'b111,{2'b01,rs1},IOP}; //ANDI(I)             
                            default:inst_32 = 32'b0;   //for debug
                        endcase
                    end
                    default:inst_32 = 32'b0;   //for debug
                endcase
            end
            2'b10:begin  //C2
                case (fun3[2]) // combine select signal trade off betewwn AT
                    1'b0:inst_32 = {{{7{imm_12}},imm_rs2},rd_rs1,3'b001,rd_rs1,IOP};        // SLLI
                    1'b1:begin
                        case (sel_comb)
                            2'b00:inst_32 = {12'b0,imm_rs2,3'b000,rd_rs1,IOP};              //MV = ADDI(I)
                            2'b01:inst_32 = {12'b0,rd_rs1,3'b000,5'b0,7'b1100111};          //JR = JALR(I)
                            2'b10:inst_32 = {7'b0,imm_rs2,rd_rs1,3'b000,rd_rs1,7'b0110011}; //ADD(R)                   
                            2'b11:inst_32 = {12'b0,rd_rs1,3'b000,5'b1,7'b1100111};          //JALR(I)
                        endcase
                    end 
                endcase
            end
            default:inst_32 = 32'b0; //for debug 
        endcase
    end
    
endmodule