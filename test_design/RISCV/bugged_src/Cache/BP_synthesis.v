module BTB(
    input clk,
    input rst_n,
    input memory_stall,

    input [29:0] instruction_out_w,
    input [31:0] instructionPC_1,
    input [7:0]  instructionPC_3,
    input        is_branchInst_3,
    input        taken_3,
    input        prev_taken_3,
    input [7:0] target_3,
    
    output [31:0] branchPC, // predicted result or updated result
    output flush,
    output taken
);
    parameter setSize = 10; // 1 + 3 + 6

    // BTB cache
    reg [setSize - 1 : 0] btb_r [0:7];
    reg [setSize - 1 : 0] btb_w [0:7];
    reg hit_3_r, taken_3_r, target_wrong3_r, is_branchInst_3_r, mem_stall_r;
    reg [2:0] index_r;
    reg [2:0] tag_r;
    reg [5:0] content_r;

    // Wires 
    reg hit_3;
    reg hit_1;
    reg target_wrong3;
    reg taken_wrong3;
    reg flush_w;
    reg taken_w;
    reg [31:0] branchPC_w;
    reg [5:0] PC_4;
    
    integer i;

    assign branchPC     = branchPC_w;
    assign flush        = flush_w;
    assign taken        = taken_w;
    
    // ===== Update BTB ===== //
    always @(*) begin
        for(i = 0; i < 8; i = i + 1) begin
            btb_w[i] = btb_r[i];
        end
        hit_1       =   btb_r[instructionPC_1[4:2]][9] & 
                          (btb_r[instructionPC_1[4:2]][8:6] == instructionPC_1[7:5]);
        hit_3       =   btb_r[instructionPC_3[4:2]][9] & 
                          (btb_r[instructionPC_3[4:2]][8:6] == instructionPC_3[7:5]);
        
        target_wrong3     = btb_r[instructionPC_3[4:2]][5:0] != target_3[7:2];
        taken_wrong3      = (~taken_3);

        if(!mem_stall_r && is_branchInst_3_r) begin
            if(!hit_3_r) begin
                if(taken_3_r) begin
                    btb_w[index_r][9]     = 1'b1;
                    btb_w[index_r][8:6]   = tag_r;
                    btb_w[index_r][5:0]    = content_r;  
                end
            end
        end        
    end

    // ===== BranchPC logic ===== //
    always @(*) begin
        if(instruction_out_w[4:3] == 2'b11) begin // BNE, BEQ, JALR, JAL
            taken_w = 1'b1;
        end
        else begin
            taken_w = 1'b0;
        end    
        PC_4 = instructionPC_1[7:2] + 1;
        if(is_branchInst_3 && (taken_wrong3 || target_wrong3)) begin // Previous branch taken wrong 
            branchPC_w  = target_3; // new PC
            flush_w     = 1'b1;
        end
        else begin
            if(taken_w) begin
                if(hit_1)
                    branchPC_w = {24'd0, btb_r[instructionPC_1[4:2]][5:0], 2'd0};
                else
                    branchPC_w = {24'd0, PC_4, 2'd0};
            end
            else 
                branchPC_w = {24'd0, PC_4, 2'd0};
            flush_w = 1'b0;
        end
    end

    always @(posedge clk) begin
        if(!rst_n) begin
            for(i = 0; i < 8; i = i + 1) begin
                btb_r[i] <= 10'd0;
            end 
            hit_3_r         <= 1'b0;
            taken_3_r       <= 1'b0;
            target_wrong3_r <= 1'b0; 
            is_branchInst_3_r <= 1'b0;
            mem_stall_r     <= 1'b0;
            index_r         <= 3'd0;
            tag_r           <= 3'd0;
            content_r       <= 6'd0;
        end
        else begin
            for(i = 0; i < 8; i = i + 1) begin
                btb_r[i] <= btb_w[i];
            end 
            hit_3_r         <= hit_3;
            taken_3_r       <= taken_3;
            target_wrong3_r <= target_wrong3; 
            is_branchInst_3_r <= is_branchInst_3;
            mem_stall_r     <= memory_stall;
            index_r         <= instructionPC_3[4:2];
            tag_r           <= instructionPC_3[7:5];
            content_r       <= target_3[7:2];
        end
    end

endmodule