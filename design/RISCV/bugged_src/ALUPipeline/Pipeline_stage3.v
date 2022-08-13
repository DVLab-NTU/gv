<<<<<<< HEAD:tests/RISCV/Baseline/src/ALUPipeline/Pipeline_stage3.v
module Execution(
    input         clk,
    input         rst_n,
    input         memory_stall,
    input [31:0]  data1,
    input [31:0]  data2,
    input [31:0]  immediate,   
    input [4:0]   Rs1_2,
    input [4:0]   Rs2_2,
    input [4:0]   Rd_2,
    
    input         is_branchInst_2,
    input [1:0]   branch_type_2,
    input [31:0]  PC_2,
    input         prev_taken_2,
    
    input         WriteBack_2,
    input [1:0]   Mem_2,
    input [4:0]   Execution_2,  // {ALUOp, ALUsrc}
    
    input [31:0]  writeback_data_5,
    input         WriteBack_5,
    input [4:0]   Rd_5,  
    
    output        WriteBack_3,
    output [1:0]  Mem_3,
    output [31:0] ALU_result_3,
    output [31:0] writedata_3, // memory write data
    output [4:0]  Rd_3,

    output [31:0] target_3,
    output [31:0] instructionPC_3,
    output        is_branchInst_3,
    output        taken_3,
    output        prev_taken_3
);

parameter ADD       = 4'd0;
parameter SUB       = 4'd1;
parameter AND       = 4'd2;
parameter OR        = 4'd3;
parameter XOR       = 4'd4;
parameter SLL       = 4'd5;
parameter SRL       = 4'd6;
parameter SRA       = 4'd7;
parameter SLT       = 4'd8;

parameter JAL       = 2'd0;
parameter JALR      = 2'd1;
parameter BEQ       = 2'd2;
parameter BNE       = 2'd3;

// regs
reg [1:0]  Mem_r, Mem_w;
reg        WriteBack_r, WriteBack_w;
reg [4:0]  Rd_r, Rd_w;
reg [31:0] ALU_result_r, ALU_result_w;
reg [31:0] writedata_r, writedata_w;

// wires
reg [31:0] ALU_in1;
reg [31:0] ALU_in2;
reg [31:0] temp;
reg [1:0]  forwardA;
reg [1:0]  forwardB;
reg [31:0] branch_target;
reg        branch_taken;

assign WriteBack_3      = WriteBack_r;
assign Mem_3            = Mem_r;
assign ALU_result_3     = ALU_result_r;
assign writedata_3      = writedata_r;
assign Rd_3             = Rd_r;

assign target_3         = branch_target;
assign instructionPC_3  = PC_2;
assign is_branchInst_3  = is_branchInst_2;
assign taken_3          = branch_taken;
assign prev_taken_3     = prev_taken_2;

always @(*) begin // forwarding unit
    
    if(WriteBack_r && Rd_r != 0 && Rd_r == Rs1_2) begin
        forwardA = 2'b10;
    end
    else if(WriteBack_5 && Rd_5 != 0 && Rd_5 == Rs1_2) begin
        forwardA = 2'b01;
    end
    else begin
        forwardA = 2'b00;
    end

    if(WriteBack_r && Rd_r != 0 && Rd_r == Rs2_2) begin
        forwardB = 2'b10;
    end
    else if(WriteBack_5 && Rd_5 != 0 && Rd_5 == Rs2_2) begin
        forwardB = 2'b01;
    end
    else begin
        forwardB = 2'b00;
    end
end

// ===== ALU input 1 ===== //
always @(*) begin
    case (forwardA)
        2'b00: begin
            ALU_in1 = data1;
        end
        2'b01: begin
            ALU_in1 = writeback_data_5;
        end
        2'b10: begin
            ALU_in1 = ALU_result_r;
        end
        default: begin
            ALU_in1 = data1;
        end
    endcase
end

// ===== ALU input 2 ===== //
always @(*) begin
    case (forwardB)
        2'b00: begin
            temp = data2;
        end
        2'b01: begin
            temp = writeback_data_5;
        end
        2'b10: begin
            temp = ALU_result_r;
        end
        default: begin
            temp = data2;
        end
    endcase
    
    ALU_in2 = Execution_2[0] ? immediate : temp;
end

// ===== Branch Information ===== //
always @(*) begin
    case (branch_type_2)
        JAL: begin
            branch_target   = $signed(PC_2) + $signed(immediate);
            branch_taken    = 1'b1;
        end
        JALR: begin
            branch_target   = $signed(ALU_in1) + $signed(immediate);
            branch_taken    = 1'b1;
        end
        BEQ: begin
            branch_target   = (ALU_result_w == 0) ? $signed(PC_2) + $signed(immediate) : $signed(PC_2) + $signed(4);
            branch_taken    = (ALU_result_w == 0) ? 1'b1 : 1'b0;
        end
        BNE: begin
            branch_target   = (ALU_result_w != 0) ? $signed(PC_2) + $signed(immediate) : $signed(PC_2) + $signed(4);
            branch_taken    = (ALU_result_w != 0) ? 1'b1 : 1'b0;
        end
    endcase
end

// ===== ALU control ===== //
always @(*) begin 
    if(memory_stall) begin
        ALU_result_w = ALU_result_r;        
    end
    else begin
        case(Execution_2[4:1])
            ADD: begin
                if(!branch_type_2[1]) begin // JALR, JAL
                    ALU_result_w = $signed(PC_2) + $signed(4);
                end
                else begin
                    ALU_result_w = $signed(ALU_in1) + $signed(ALU_in2);
                end
            end
            SUB: begin
                ALU_result_w = $signed(ALU_in1) - $signed(ALU_in2);
            end
            AND: begin
                ALU_result_w = ALU_in1 & ALU_in2;
            end
            OR: begin
                ALU_result_w = ALU_in1 | ALU_in2;
            end
            XOR: begin
                ALU_result_w = ALU_in1 ^ ALU_in2;
            end
            SLL: begin
                ALU_result_w = ALU_in1 << ALU_in2;
            end
            SRL: begin
                ALU_result_w = ALU_in1 >> ALU_in2;
            end
            SRA: begin
                ALU_result_w = $signed(ALU_in1) >>> ALU_in2;
            end
            SLT: begin
                ALU_result_w = ($signed(ALU_in1) < $signed(ALU_in2)) ? 1 : 0;
            end
            default: begin
                ALU_result_w = 32'd0;
            end
        endcase    
    end
end

// ===== passing signals ===== //
always @(*) begin
    Mem_w           = memory_stall ? Mem_r          : Mem_2;
    WriteBack_w     = memory_stall ? WriteBack_r    : WriteBack_2;
    Rd_w            = memory_stall ? Rd_r           : Rd_2;
    
    writedata_w     = memory_stall ? writedata_r    : temp;
end

always @(posedge clk) begin
    if(!rst_n) begin
        Mem_r           <= 2'd0;
        WriteBack_r     <= 1'b0;
        Rd_r            <= 5'd0;
        ALU_result_r    <= 32'd0;
        writedata_r     <= 32'd0;
    end
    else begin
        Mem_r           <= Mem_w;
        WriteBack_r     <= WriteBack_w;
        Rd_r            <= Rd_w;
        ALU_result_r    <= ALU_result_w;
        writedata_r     <= writedata_w;
    end
end
=======
module Execution(
    input         clk,
    input         rst_n,
    input         memory_stall,
    input [31:0]  data1,
    input [31:0]  data2,
    input [31:0]  immediate,   
    input [4:0]   Rs1_2,
    input [4:0]   Rs2_2,
    input [4:0]   Rd_2,
    
    input         is_branchInst_2,
    input [1:0]   branch_type_2,
    input [31:0]  PC_2,
    input         prev_taken_2,
    
    input         WriteBack_2,
    input [1:0]   Mem_2,
    input [4:0]   Execution_2,  // {ALUOp, ALUsrc}
    
    input [31:0]  writeback_data_5,
    input         WriteBack_5,
    input [4:0]   Rd_5,  
    
    output        WriteBack_3,
    output [1:0]  Mem_3,
    output [31:0] ALU_result_3,
    output [31:0] writedata_3, // memory write data
    output [4:0]  Rd_3,

    output [31:0] target_3,
    output [31:0] instructionPC_3,
    output        is_branchInst_3,
    output        taken_3,
    output        prev_taken_3
);

parameter ADD       = 4'd0;
parameter SUB       = 4'd1;
parameter AND       = 4'd2;
parameter OR        = 4'd3;
parameter XOR       = 4'd4;
parameter SLL       = 4'd5;
parameter SRL       = 4'd6;
parameter SRA       = 4'd7;
parameter SLT       = 4'd8;

parameter JAL       = 2'd0;
parameter JALR      = 2'd1;
parameter BEQ       = 2'd2;
parameter BNE       = 2'd3;

// regs
reg [1:0]  Mem3_r, Mem3_w;
reg        WriteBack3_r, WriteBack3_w;
reg [4:0]  Rd3_r, Rd3_w;
reg [31:0] ALU_result3_r, ALU_result3_w;
reg [31:0] writedata3_r, writedata3_w;

// wires
reg [31:0] ALU_in1;
reg [31:0] ALU_in2;
reg [31:0] temp;
reg [1:0]  forwardA;
reg [1:0]  forwardB;
reg [31:0] branch_target;
reg        branch_taken;

assign WriteBack_3      = WriteBack3_r;
assign Mem_3            = Mem3_r;
assign ALU_result_3     = ALU_result3_r;
assign writedata_3      = writedata3_r;
assign Rd_3             = Rd3_r;

assign target_3         = branch_target;
assign instructionPC_3  = PC_2;
assign is_branchInst_3  = is_branchInst_2;
assign taken_3          = branch_taken;
assign prev_taken_3     = prev_taken_2;

always @(*) begin // forwarding unit
    
    if(WriteBack3_r && Rd3_r != 0 && Rd3_r == Rs1_2) begin
        forwardA = 2'b10;
    end
    else if(WriteBack_5 && Rd_5 != 0 && Rd_5 == Rs1_2) begin
        forwardA = 2'b01;
    end
    else begin
        forwardA = 2'b00;
    end

    if(WriteBack3_r && Rd3_r != 0 && Rd3_r == Rs2_2) begin
        forwardB = 2'b10;
    end
    else if(WriteBack_5 && Rd_5 != 0 && Rd_5 == Rs2_2) begin
        forwardB = 2'b01;
    end
    else begin
        forwardB = 2'b00;
    end
end

// ===== ALU input 1 ===== //
always @(*) begin
    case (forwardA)
        2'b00: begin
            ALU_in1 = data1;
        end
        2'b01: begin
            ALU_in1 = writeback_data_5;
        end
        2'b10: begin
            ALU_in1 = ALU_result3_r;
        end
        default: begin
            ALU_in1 = data1;
        end
    endcase
end

// ===== ALU input 2 ===== //
always @(*) begin
    case (forwardB)
        2'b00: begin
            temp = data2;
        end
        2'b01: begin
            temp = writeback_data_5;
        end
        2'b10: begin
            temp = ALU_result3_r;
        end
        default: begin
            temp = data2;
        end
    endcase
    
    ALU_in2 = Execution_2[0] ? immediate : temp;
end

// ===== Branch Information ===== //
always @(*) begin
    case (branch_type_2)
        JAL: begin
            branch_target   = $signed(PC_2) + $signed(immediate);
            branch_taken    = 1'b1;
        end
        JALR: begin
            branch_target   = $signed(ALU_in1) + $signed(immediate);
            branch_taken    = 1'b1;
        end
        BEQ: begin
            branch_target   = (ALU_result3_w == 0) ? $signed(PC_2) + $signed(immediate) : $signed(PC_2) + $signed(4);
            branch_taken    = (ALU_result3_w == 0) ? 1'b1 : 1'b0;
        end
        BNE: begin
            branch_target   = (ALU_result3_w != 0) ? $signed(PC_2) + $signed(immediate) : $signed(PC_2) + $signed(4);
            branch_taken    = (ALU_result3_w != 0) ? 1'b1 : 1'b0;
        end
    endcase
end

// ===== ALU control ===== //
always @(*) begin 
    if(memory_stall) begin
        ALU_result3_w = ALU_result3_r;        
    end
    else begin
        case(Execution_2[4:1])
            ADD: begin
                if(!branch_type_2[1]) begin // JALR, JAL
                    ALU_result3_w = $signed(PC_2) + $signed(4);
                end
                else begin
                    ALU_result3_w = $signed(ALU_in1) + $signed(ALU_in2);
                end
            end
            SUB: begin
                ALU_result3_w = $signed(ALU_in1) - $signed(ALU_in2);
            end
            AND: begin
                ALU_result3_w = ALU_in1 & ALU_in2;
            end
            OR: begin
                ALU_result3_w = ALU_in1 | ALU_in2;
            end
            XOR: begin
                ALU_result3_w = ALU_in1 ^ ALU_in2;
            end
            SLL: begin
                ALU_result3_w = ALU_in1 << ALU_in2;
            end
            SRL: begin
                ALU_result3_w = ALU_in1 >> ALU_in2;
            end
            SRA: begin
                ALU_result3_w = $signed(ALU_in1) >>> ALU_in2;
            end
            SLT: begin  // set less than
                ALU_result3_w = ($signed(ALU_in1) < $signed(ALU_in2)) ? 1 : 0;
            end
            default: begin
                ALU_result3_w = 32'd0;
            end
        endcase    
    end
end

// ===== passing signals ===== //
always @(*) begin
    Mem3_w           = memory_stall ? Mem3_r          : Mem_2;
    WriteBack3_w     = memory_stall ? WriteBack3_r    : WriteBack_2;
    Rd3_w            = memory_stall ? Rd3_r           : Rd_2;
    
    writedata3_w     = memory_stall ? writedata3_r    : temp;
end

always @(posedge clk) begin
    if(!rst_n) begin
        Mem3_r           <= 2'd0;
        WriteBack3_r     <= 1'b0;
        Rd3_r            <= 5'd0;
        ALU_result3_r    <= 32'd0;
        writedata3_r     <= 32'd0;
    end
    else begin
        Mem3_r           <= Mem3_w;
        WriteBack3_r     <= WriteBack3_w;
        Rd3_r            <= Rd3_w;
        ALU_result3_r    <= ALU_result3_w;
        writedata3_r     <= writedata3_w;
    end
end

// Add assertion here (ncverilog can read "psl" comments, and yosys cannot read it)
// psl default clock = (posedge clk);

// psl ERROR1_memory_stall_stage3: assert never {(memory_stall) && !(ALU_result3_w == ALU_result3_r)};
// psl ERROR2_memory_stall_stage3: assert never {(memory_stall) && !(Rd3_w == Rd3_r)};
// psl ERROR3_memory_stall_stage3: assert never {(memory_stall) && !(WriteBack3_w == WriteBack3_r)}; 
// psl ERROR4_memory_stall_stage3: assert never {(memory_stall) && !(writedata3_w == writedata3_r)};
// psl ERROR5_memory_stall_stage3: assert never {(memory_stall) && !(Mem3_w == Mem3_r)};

// psl ERROR1_forwarding_stage3: assert never {((WriteBack3_r) && (Rd3_r != 0) && (Rd3_r == Rs1_2)) && !(forwardA != 2'b00)};  
// psl ERROR2_forwarding_stage3: assert never {((WriteBack3_r) && (Rd3_r != 0) && (Rd3_r == Rs2_2)) && !(forwardB != 2'b00)}; 
// psl ERROR3_forwarding_stage3: assert never {((WriteBack_5) && (Rd_5 != 0) && (Rd_5 == Rs1_2)) && !(forwardA != 2'b00)};     
// psl ERROR4_forwarding_stage3: assert never {((WriteBack_5) && (Rd_5 != 0) && (Rd_5 == Rs2_2)) && !(forwardB != 2'b00)};

>>>>>>> f65de8d3044d54d3aa91f2a19d5029f2c430844e:test_design/RISCV/bugged_src/ALUPipeline/Pipeline_stage3.v
endmodule