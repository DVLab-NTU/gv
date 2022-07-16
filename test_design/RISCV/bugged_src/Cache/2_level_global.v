module BTB(
    input clk,
    input rst_n,
    input memory_stall,

    input [31:0] instructionPC_1,
    input [31:0] instructionPC_3,
    input        is_branchInst_3,
    input        taken_3,
    input        prev_taken_3,
    input [31:0] target_3,
    
    output [31:0] branchPC, // predicted result or updated result
    output flush,
    output taken
);
    parameter setSize = 61; // 1 + 28 + 32

    // BTB cache
    reg [setSize - 1 : 0] btb_r [0:7];
    reg [setSize - 1 : 0] btb_w [0:7];
    
    // Wires 
    reg hit_3;
    reg hit_1;
    reg target_wrong3;
    reg taken_wrong3;
    reg flush_w;
    reg taken_w;
    reg [31:0] branchPC_w;
    
    integer i;
    wire lp_prediction;

    assign branchPC     = branchPC_w;
    assign flush        = flush_w;
    assign taken        = taken_w;
    
    level2_predictor lp(
        .clk(clk),
        .rst_n(rst_n),
        .memory_stall(memory_stall),
        .taken3(taken_3),
        .result(lp_prediction)
    );

    // ===== Update BTB ===== //
    always @(*) begin
        for(i = 0; i < 8; i = i + 1) begin
            btb_w[i] = btb_r[i];
        end
        hit_1       =   btb_r[instructionPC_1[3:1]][60] & 
                          (btb_r[instructionPC_1[3:1]][59:32] == instructionPC_1[31:4]);
        hit_3       =   btb_r[instructionPC_3[3:1]][60] & 
                          (btb_r[instructionPC_3[3:1]][59:32] == instructionPC_3[31:4]);
        
        target_wrong3     = prev_taken_3    & (btb_r[instructionPC_3[3:1]][31:0] != target_3);
        taken_wrong3      = is_branchInst_3 & (prev_taken_3 != taken_3);

        if(!memory_stall && is_branchInst_3) begin
            if(!hit_3) begin
                if(taken_3) begin
                    btb_w[instructionPC_3[3:1]][60]     = 1'b1;
                    btb_w[instructionPC_3[3:1]][59:32]  = instructionPC_3[31:4];
                    btb_w[instructionPC_3[3:1]][31:0]   = target_3;     
                end
            end
            else begin
                //btb_w[instructionPC_3[2:0]][62]     = 1'b1;
                //btb_w[instructionPC_3[2:0]][61:33]  = instructionPC_3[31:3];
                
                if(target_wrong3) begin // target correct !!
                    btb_w[instructionPC_3[3:1]][31:0]     = target_3;
                end
            end
        end        
    end

    // ===== BranchPC logic ===== //
    always @(*) begin
        
        taken_w = hit_1 & lp_prediction;
        
        if(taken_wrong3 || target_wrong3) begin // Previous branch taken wrong 
            branchPC_w  = target_3; // new PC
            flush_w     = 1'b1;
        end
        else begin
            if(taken_w)
                branchPC_w = btb_r[instructionPC_1[3:1]][31:0]; // Predicted taken !!
            else 
                branchPC_w = instructionPC_1 + 4; // Predicted NOT Taken !!
            flush_w = 1'b0;
        end
    end

    always @(posedge clk) begin
        if(!rst_n) begin
            for(i = 0; i < 8; i = i + 1) begin
                btb_r[i] <= 61'd0;
            end 
        end
        else begin
            for(i = 0; i < 8; i = i + 1) begin
                btb_r[i] <= btb_w[i];
            end 
        end
    end

endmodule

module level2_predictor(
    input clk,
    input rst_n,
    input memory_stall,
    input taken3,
    output result
);
    
    parameter history_size  = 5;
    parameter HRT_size      = 32;

    reg [history_size - 1 : 0]  globalHistory_r, globalHistory_w;
    reg [1 : 0]                 HRT_r [0 : HRT_size - 1];
    reg [1 : 0]                 HRT_w [0 : HRT_size - 1];
    reg [history_size - 1 : 0]  delay_result1_r, delay_result1_w;
    reg [history_size - 1 : 0]  delay_result2_r, delay_result2_w;
    reg [history_size - 1 : 0]  result_w;
    integer i;

    assign result = result_w;

    always @(*) begin      
        result_w = HRT_r[globalHistory_r][1];

        globalHistory_w = memory_stall ? globalHistory_r : {globalHistory_r[3:0], result_w};
        delay_result1_w = memory_stall ? delay_result1_r : globalHistory_r;
        delay_result2_w = memory_stall ? delay_result2_r : delay_result1_r;
        for(i = 0; i < HRT_size; i = i + 1)
            HRT_w[i] = HRT_r[i];

        if(!memory_stall) begin
            case (HRT_r[delay_result2_r])
                2'b00: begin
                    HRT_w[delay_result2_r] = taken3 ? 2'b01 : 2'b00;
                end
                2'b01: begin
                    HRT_w[delay_result2_r] = taken3 ? 2'b10 : 2'b00;
                end
                2'b10: begin
                    HRT_w[delay_result2_r] = taken3 ? 2'b11 : 2'b01;
                end
                2'b11: begin
                    HRT_w[delay_result2_r] = taken3 ? 2'b11 : 2'b01;
                end
            endcase
        end
    end

    always @(posedge clk) begin
        if(!rst_n) begin
            globalHistory_r <= {(history_size){1'b0}};
            delay_result1_r <= {(history_size){1'b0}};
            delay_result2_r <= {(history_size){1'b0}};
            for(i = 0; i < HRT_size; i = i + 1)
                HRT_r[i] <= {(HRT_size){1'b0}};
        end
        else begin
            globalHistory_r <= globalHistory_w;            
            delay_result1_r <= delay_result1_w;
            delay_result2_r <= delay_result2_w;
            for(i = 0; i < HRT_size; i = i + 1)
                HRT_r[i] <= HRT_w[i];
        end
    end
endmodule