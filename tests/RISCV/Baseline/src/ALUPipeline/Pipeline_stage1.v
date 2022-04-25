//`include "./ALUPipeline/compression_16.v"

module instruction_fetch(
    input         clk,
    input         rst_n,
    
    input         flush,            // if jump (BEQ), 沖刷掉 stage_3 (EXE) 之前所有指令
    input         taken,
    input  [31:0] branchPC,

    input         memory_stall,     // reg 尚未存入欲讀取的值, 便先停止直到成功存入 (Control/Data Hazard)
    input  [31:0] IF_DWrite,     
    input         PC_write,             
    
    input  [31:0] instruction_in,
    output [29:0] I_addr,       
    output        I_ren,            // I read_enable
    
    output [31:0] PC_1,
    output [31:0] instruction_1,
    
    output        prev_taken_1,
    output [31:0] instructionPC_1
);

// regs
reg [31:0] PC_r, PC_w;
reg [31:0] PC_out_r, PC_out_w;
reg [31:0] instruction_out_r, instruction_out_w;
reg        taken_r, taken_w;

// wires
reg [29:0] I_addr_w;
reg I_ren_w;
wire[31:0] instruction_little; //instruction input with little_end --> 圖片由最右邊 (LSB) 讀到最左邊 (MSB), 方便操作讓最右邊開始的每個 byte 依序讀入?

assign PC_1                 = PC_out_r;
assign instruction_1        = instruction_out_r;
assign I_addr               = I_addr_w;
assign I_ren                = I_ren_w;
assign instruction_little   = {instruction_in[7:0],instruction_in[15:8],instruction_in[23:16],instruction_in[31:24]}; //instruction with little_end
assign prev_taken_1         = taken_r;
assign instructionPC_1      = PC_r;

// ===== RVC ===== //
// wires
//wire [15:0] inst_16;
//wire [31:0] inst_32;
wire  [31:0] instruction; //real output instruction 
assign instruction = instruction_little ;
//assign inst_16 = instruction_little[15:0];

//Decompressor decompressor(.inst_16(inst_16),.inst_32(inst_32));
/*
always @(*) begin
    if (instruction_little == 32'b0) instruction = 32'h00000013 ; //initialize but can ignore?
    else if (instruction_little[1]&instruction_little[0]) instruction = instruction_little ;
    else instruction = inst_32 ;
end
*/
// ===== PC ===== //
always @(*) begin
    if(memory_stall) begin
        PC_w        = PC_r;
        taken_w     = taken_r;
    end
    else begin 
        if(flush) begin
            PC_w        = branchPC;
            taken_w     = 1'b0;
        end
        else begin
            if(PC_write) begin
                PC_w        = PC_r;
                taken_w     = taken_r;
            end
            else begin
                taken_w     = taken;
                PC_w        = branchPC;
            end
        end
    end
end

// ===== PC_out ===== //
always @(*) begin
    if(memory_stall) begin
        PC_out_w = PC_out_r ;
    end
    else begin
        if(flush) begin // branch hazard(insert NOP)
            PC_out_w = 32'd0;  
        end
        else begin
            if(PC_write) begin // load-use hazard
                PC_out_w = PC_r - 4;
            end
            else begin
                PC_out_w = PC_r;
            end
        end
    end
end

// ===== instruction_out ===== //
always @(*) begin
    if(memory_stall) begin
        instruction_out_w = instruction_out_r;
    end
    else begin
        if(flush) begin
            instruction_out_w = 32'h00000013; // NOP
        end
        else begin
            if(PC_write) begin
                instruction_out_w = IF_DWrite;
            end
            else begin
                instruction_out_w = instruction;
            end    
        end
    end
end

// ===== I_cache ===== //
always @(*) begin
    I_addr_w            = PC_r[31:2];
    I_ren_w             = 1'b1;  // always reading I_cache
end

always @(posedge clk) begin
    if(!rst_n) begin
        PC_r                <= 32'd0;
        PC_out_r            <= 32'd0;
        instruction_out_r   <= 32'd0;
        taken_r             <= 1'b0;
    end
    else begin
        PC_r                <= PC_w;
        PC_out_r            <= PC_out_w;
        instruction_out_r   <= instruction_out_w;
        taken_r             <= taken_w;
    end
end  

/*
    [ assertion for stage 1 (fetch) ]

    - if (memory_stall): 
        - register's value stall, no change (for all output pin)
            - PC. taken, PC_out, instruction_out
        - go to check out "Pipeline_stage1.v" (因為都是 internal variable)
        - go to check out "Pipeline_stage2.v" (因為都是 internal variable)
        - go to check out "Pipeline_stage3.v" (因為都是 internal variable)
        - go to check out "Pipeline_stage4.v" (因為都是 internal variable)
        
*/
assert property ((memory_stall) ? (PC_w == PC_r) : 1);
assert property ((memory_stall) ? (taken_w == taken_r) : 1);
assert property ((memory_stall) ? (PC_out_w == PC_out_r) : 1);
assert property ((memory_stall) ? (instruction_out_w == instruction_out_r) : 1);

endmodule
