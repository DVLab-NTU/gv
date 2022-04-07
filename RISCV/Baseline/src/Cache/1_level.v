module BTB(
    input clk,
    input rst_n,
    input memory_stall,

    input [31:0]  instructionPC_1,
    input [31:0] instructionPC_3,
    input        is_branchInst_3,
    input        taken_3,
    input        prev_taken_3,
    input [31:0] target_3,
    
    output [31:0] branchPC, // predicted result or updated result
    output flush,
    output taken
);
    parameter setSize = 62; // 1 + 27 + 32 + 2 

    // BTB cache
    reg [setSize - 1 : 0] btb_r [0:15];
    reg [setSize - 1 : 0] btb_w [0:15];
    
    // Wires 
    reg hit_3;
    reg hit_1;
    reg target_wrong3;
    reg taken_wrong3;
    reg flush_w;
    reg taken_w;
    reg [31:0] branchPC_w;
    reg [1:0] history_3;
    reg [1:0] history_next;
    
    integer i;

    assign branchPC     = branchPC_w;
    assign flush        = flush_w;
    assign taken        = taken_w;
    
    // ===== Branch Prediction FSM ===== //
    always @(*) begin
        history_3   =   btb_w[instructionPC_3[4:1]][1:0];
        
        case (history_3)
            2'b00: begin
                if(taken_3)
                    history_next = 2'b01;
                else
                    history_next = 2'b00;
            end
            2'b01: begin
                if(taken_3)
                    history_next = 2'b10;
                else
                    history_next = 2'b00;
            end
            2'b10: begin
                if(taken_3)
                    history_next = 2'b11;
                else
                    history_next = 2'b01;
            end
            2'b11: begin
                if(taken_3)
                    history_next = 2'b11;
                else
                    history_next = 2'b10;
            end
        endcase
    end

    // ===== Update BTB ===== //
    always @(*) begin
        for(i = 0; i < 16; i = i + 1) begin
            btb_w[i] = btb_r[i];
        end
        hit_1       =   btb_r[instructionPC_1[4:1]][61] & 
                          (btb_r[instructionPC_1[4:1]][60:34] == instructionPC_1[31:5]);
        hit_3       =   btb_r[instructionPC_3[4:1]][61] & 
                          (btb_r[instructionPC_3[4:1]][60:34] == instructionPC_3[31:5]);
        
        target_wrong3     = prev_taken_3 & (btb_r[instructionPC_3[4:1]][33:2] != target_3);
        taken_wrong3      = is_branchInst_3 & (prev_taken_3 != taken_3);

        if(!memory_stall && is_branchInst_3) begin
            if(!hit_3) begin
                if(taken_3) begin
                    btb_w[instructionPC_3[4:1]][61]     = 1'b1;
                    btb_w[instructionPC_3[4:1]][60:34]  = instructionPC_3[31:5];
                    btb_w[instructionPC_3[4:1]][33:2]   = target_3;
                    btb_w[instructionPC_3[4:1]][1:0]    = 2'b10;     
                end
            end
            else begin
                //btb_w[instructionPC_3[2:0]][62]     = 1'b1;
                //btb_w[instructionPC_3[2:0]][61:33]  = instructionPC_3[31:3];
                
                if(!target_wrong3) begin // target correct !!
                    btb_w[instructionPC_3[4:1]][1:0]      = history_next;     
                end
                else begin
                    btb_w[instructionPC_3[4:1]][33:2]     = target_3;
                    btb_w[instructionPC_3[4:1]][1:0]      = 2'b10; 
                end
            end
        end        
    end

    // ===== BranchPC logic ===== //
    always @(*) begin
        
        taken_w = hit_1 & btb_r[instructionPC_1[4:1]][1];
        
        if(taken_wrong3 || target_wrong3) begin // Previous branch taken wrong 
            branchPC_w  = target_3; // new PC
            flush_w     = 1'b1;
        end
        else begin
            if(taken_w)
                branchPC_w = btb_r[instructionPC_1[4:1]][33:2]; // Predicted taken !!
            else 
                branchPC_w = instructionPC_1 + 4; // Predicted NOT Taken !!
            flush_w = 1'b0;
        end
    end

    always @(posedge clk) begin
        if(!rst_n) begin
            for(i = 0; i < 16; i = i + 1) begin
                btb_r[i] <= 62'd0;
            end 
        end
        else begin
            for(i = 0; i < 16; i = i + 1) begin
                btb_r[i] <= btb_w[i];
            end 
        end
    end

endmodule