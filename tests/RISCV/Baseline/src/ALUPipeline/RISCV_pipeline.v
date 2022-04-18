`include "./Pipeline_stage1.v"
`include "./Pipeline_stage2.v"
`include "./Pipeline_stage3.v"
`include "./Pipeline_stage4.v"
`include "./Pipeline_stage5.v"
`include "../Cache/2way_BTB.v"

module RISCV_Pipeline(
    input clk,
    input rst_n,
    // === I cache interface === //
    output ICACHE_ren,
    output ICACHE_wen,
    output [29:0] ICACHE_addr,
    output [31:0] ICACHE_wdata,
    input  ICACHE_stall,
    input  [31:0] ICACHE_rdata,
    // === D cache interface === //
    output DCACHE_ren,
    output DCACHE_wen,
    output [29:0] DCACHE_addr,
    output [31:0] DCACHE_wdata,
    input  DCACHE_stall,
    input  [31:0] DCACHE_rdata,
    output instruction_flush,
    output branchType
);

// ==== BTB output ==== //
wire [31:0] instructionPC_1;
wire        flush;
wire        taken;
wire [31:0] branchPC;
wire [31:0] instructionPC_3;
wire        is_branchInst_3;
wire        taken_3;
wire        prev_taken_3;
wire [31:0] target_3;
// ==================== //

// ==== L1 signals ==== //
// input
wire [31:0] instruction_in;
wire        memory_stall; 
// output
wire [31:0] PC_1;
wire [31:0] instruction_1;
wire        prev_taken_1;
// ====================//

// ==== L2 signals ==== //
// output
wire [4:0]  Rd_2;
wire [4:0]  Rs1_2;
wire [4:0]  Rs2_2;
wire [31:0] data1;
wire [31:0] data2;
wire [31:0] immediate;
wire        is_branchInst_2;
wire [1:0]  branch_type_2;
wire [31:0] PC_2;
wire        prev_taken_2;
wire [1:0]  Mem_2;
wire        WriteBack_2;
wire [4:0]  Execution_2;
wire [31:0] IF_DWrite;
wire        PC_write;
// ==================== //

// ==== L3 signals ==== //
wire        WriteBack_3;
wire [1:0]  Mem_3;
wire [31:0] ALU_result_3;
wire [31:0] writedata_3;
wire [4:0]  Rd_3;
// ==================== //

// ==== L4 signals ==== //
// output 
wire [31:0] forward_result_4;
wire [31:0] memory_result_4;
wire [31:0] ALU_result_4;
wire [4:0]  Rd_4;
wire        WriteBack_4;
wire        Mem2Reg;
// ==================== //

// ==== L5 signals ==== //
wire [4:0]  Rd_5;
wire [31:0] Wd_5;
wire        Wb_5;
// ==================== //

assign ICACHE_wen   = 1'b0;
assign ICACHE_wdata = 32'd0;
assign memory_stall = DCACHE_stall | ICACHE_stall; // either cache stall will stall the whole pipeline
assign instruction_flush = flush;
assign branchType = is_branchInst_3;

BTB btb1(
    .clk(clk),
    .rst_n(rst_n),
    .memory_stall(memory_stall),
    .instructionPC_1(instructionPC_1),
    .branchPC(branchPC),
    .flush(flush),
    .taken(taken),
    .instructionPC_3(instructionPC_3),
    .is_branchInst_3(is_branchInst_3),
    .taken_3(taken_3),
    .prev_taken_3(prev_taken_3),
    .target_3(target_3)
);

instruction_fetch stage1(
    .clk(clk),
    .rst_n(rst_n),
    .flush(flush),
    .taken(taken),
    .branchPC(branchPC),
    .memory_stall(memory_stall),
    .IF_DWrite(IF_DWrite),
    .PC_write(PC_write),
    .instruction_in(ICACHE_rdata),
    .I_addr(ICACHE_addr),
    .I_ren(ICACHE_ren),
    .PC_1(PC_1),
    .instruction_1(instruction_1),
    .prev_taken_1(prev_taken_1),
    .instructionPC_1(instructionPC_1)
);

instruction_decode stage2(
    .clk(clk),
    .rst_n(rst_n),
    .memory_stall(memory_stall),
    .WriteBack_5(Wb_5),
    .write_data(Wd_5),
    .write_address(Rd_5),
    .prev_taken_1(prev_taken_1),
    .flush(flush),
    .instruction_1(instruction_1),
    .PC_1(PC_1),
    .Rd_2(Rd_2),
    .Rs1_2(Rs1_2),
    .Rs2_2(Rs2_2),
    .data1(data1),
    .data2(data2),
    .immediate(immediate), 
    .is_branchInst_2(is_branchInst_2),
    .branch_type_2(branch_type_2),
    .PC_2(PC_2),
    .prev_taken_2(prev_taken_2),
    .Mem_2(Mem_2),
    .WriteBack_2(WriteBack_2),
    .Execution_2(Execution_2), 
    .IF_DWrite(IF_DWrite),
    .PC_write(PC_write)
);

Execution stage3(
    .clk(clk),
    .rst_n(rst_n),
    .memory_stall(memory_stall),
    .data1(data1),
    .data2(data2),
    .immediate(immediate),   
    .Rs1_2(Rs1_2),
    .Rs2_2(Rs2_2),
    .Rd_2(Rd_2),
    .is_branchInst_2(is_branchInst_2),
    .branch_type_2(branch_type_2),
    .PC_2(PC_2),
    .prev_taken_2(prev_taken_2),
    .WriteBack_2(WriteBack_2),
    .Mem_2(Mem_2),
    .Execution_2(Execution_2),  
    .writeback_data_5(Wd_5),
    .WriteBack_5(Wb_5),
    .Rd_5(Rd_5),  
    .WriteBack_3(WriteBack_3),
    .Mem_3(Mem_3),
    .ALU_result_3(ALU_result_3),
    .writedata_3(writedata_3),
    .Rd_3(Rd_3),
    .target_3(target_3),
    .instructionPC_3(instructionPC_3),
    .is_branchInst_3(is_branchInst_3),
    .taken_3(taken_3),
    .prev_taken_3(prev_taken_3)
);

Memory stage4(
    .clk(clk),
    .rst_n(rst_n),
    .memory_stall(memory_stall),
    .ALU_result_3(ALU_result_3),
    .writedata_3(writedata_3),
    .D_readData(DCACHE_rdata),
    .WriteBack_3(WriteBack_3),
    .Mem_3(Mem_3),
    .Rd_3(Rd_3),
    .memory_result_4(memory_result_4),
    .ALU_result_4(ALU_result_4),
    .forward_result_4(forward_result_4),
    .Rd_4(Rd_4),
    .WriteBack_4(WriteBack_4),
    .Mem2Reg(Mem2Reg),
    .D_addr(DCACHE_addr),
    .D_wen(DCACHE_wen),
    .D_ren(DCACHE_ren),
    .D_writeData(DCACHE_wdata)
);

Writeback stage5(
    .ALU_result_4(ALU_result_4),
    .memory_result_4(memory_result_4),
    .WriteBack_4(WriteBack_4),
    .Mem2Reg(Mem2Reg),
    .Rd_4(Rd_4),
    .Rd_5(Rd_5),
    .writeback_data_5(Wd_5),
    .WriteBack_5(Wb_5)
);

endmodule
