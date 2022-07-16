<<<<<<< HEAD:tests/RISCV/Baseline/src/ALUPipeline/RISCV_pipeline.v
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
=======
`include "./ALUPipeline/Pipeline_stage1.v"
`include "./ALUPipeline/Pipeline_stage2.v"
`include "./ALUPipeline/Pipeline_stage3.v"
`include "./ALUPipeline/Pipeline_stage4.v"
`include "./ALUPipeline/Pipeline_stage5.v"

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


/*
    [ assertion for stage 1 (fetch) ]
    
    - external (keep)
        - if (branch) then (flush): 
            - now, it must be a "jump" instruction (instructionPC_1)
            - reset "branch_is_taken (prev_taken_1)"   --> else: True, only test "flush"
            - reset "instruction_out (instruction_1)"  --> else: True, only test "flush"
            - reset "PC out          (PC_1)"           --> else: True, only test "flush"
        
        - if (load-use hazard): @ MIPS_detail.pdf (PC_write)
            - PC (IF) still retain the previous action --> Inter Lock
    
    - internal (會用到內部變數)
        - if (memory_stall): 
            - register's value stall, no change (for all output pin)
                - PC. taken, PC_out, instruction_out
            - go to check out "Pipeline_stage1.v" (因為都是 internal variable)
            - go to check out "Pipeline_stage2.v" (因為都是 internal variable)
            - go to check out "Pipeline_stage3.v" (因為都是 internal variable)
            - go to check out "Pipeline_stage4.v" (因為都是 internal variable)

    - if (nothing_special):
        - parameter will directly pass to stage 2, so no need assertion

*/


/*
    [ assertion for stage 2 (decode) ]

    -external (keep)
        - if (flush): (將適當的 register 資料清為 0)
            - reset "rs1, rs2, rd (Rs1_2, Rs2_2, Rd_2)"
            - reset "data_rs1, data_rs2, immediate (data1, data2, immediate)"
            - reset "MemtoReg (WriteBack_2)"
            - reset "PC_2 (PC_2)"
            - reset "is_branch"

    -internal (會用到內部變數)
        - if (memory_stall): 
            - register's value or signal, no change (for all output pin)
                - Rs1, Rs2, Rd, immediate, data_rs1, data_rs2
                - PC_2, is_branch, branch_type, memory
                - Execution ([3:1] = ALUop, [0] = ALUsrc)
                - write_back (MemtoReg)
            - 註: ALUop: 3 bits, ALU 操作碼
            - 註: ALUsrc: 1 bit, 決定 ALU 的輸入是"由 Register bank 讀出"還是"立即值 (immediate)"
            - 註: 寫在 Pipeline_stage2.v (因為都是 internal variable)

        - if (load-use hazard): "Hazard Detection Unit" detect to be 1 (PC_write)
            - (Load) && (I2 指令的暫存器編號 (rs) == I1 指令的目的地暫存器編號 (rd)) --> hazard = 1, 其餘為 0 (matbe floating)
            - 註: Mem_2[1] = MemRead (Load from memory)
            - 註: Mem_2[0] = MemWrite
            - 註: Mem_2: 2 bits, (00, 01, 10) = (no read no write, write, read)
            - 註: 寫在 Pipeline_stage2.v (因為變數 Rd_r 在其他檔案也有宣告, 會亂掉)

        - if (data hazard --> forwarding): EXE, MEM, WB result "forwarding" to ID stage
            - 如果 write_back 的訊號 enable, 那 register bank 的該 address 應該要存進 WB 的 data
            - 註: 寫在 Pipeline_stage2.v 

        - if (control_unit case):
            - MIPS_detal.pdf 第 (7)(8)(9) 點: 若 stage 1 傳入為跳轉指令, 要設起 branch flag
            - MIPS_detal.pdf 第 (4) 點: 若為 Load 指令, 讀入 memory 的資料 --> MemRead (under no data hazard) 
            - MIPS_detal.pdf 第 (6) 點: 若為 Store 指令, 寫資料進 memory --> MemWrite (under no data hazard)
            - 註: 寫在 Pipeline_stage2.v 

    - if (nothing_special):
        - parameter will directly pass to stage 3, so no need assertion

*/


/*
    [ assertion for stage 3 (execute) ]
    
    - external (keep)


    - internal (會用到內部變數)
        - if (memory_stall): 
            - 當前執行的結果 stall (ALU_result)
            - signal stall
                - RegDestination (rd), Mem2Reg (writeback_2, writedata), MemWrite (Mem)
            - 註: 寫在 Pipeline_stage3.v 

        - if (forwarding_unit):
            - 如果符合下列條件, forwarding_signal 要立起來
                - instr 必須為有寫入暫存器的指令 (Control Signal — RegWrite 來判斷)
                - instr 寫入的暫存器不為 0 (instr 的 Rd 來判斷)
                - instr 寫入的暫存器與 n_instr、nn_instr 有 data dependency (instr 的 Rd 與 n_instr 的 rs1/rs2、nn_instr 的 rs1/rs2 有沒有相同)
            - 看 data 是不是按照 MEM 或 WB 的 flag 寫入
            - 註: 寫在 Pipeline_stage3.v 

    - if (nothing_special):
        - 根據不同的 decode 指令結果做不同的運算

*/
  

/*
    [ assertion for stage 4 (memory access) ]

    - external (keep)


    - internal (會用到內部變數)
        - if (memory_stall): 
            - 當前執行的結果 stall (memory/ALU result)
            - signal stall
                - RegDestination (rd), Mem2Reg (writeback)
            - 註: 寫在 Pipeline_stage4.v
    
    - if (nothing_special):
        - 按照 signal 指令賦值 (load, store)

*/


/*
    [ assertion for stage 5 (write back) ] 

    - external (keep)


    - internal (會用到內部變數)
        - 控制寫回的資料是"由 Data Memory 提供"還是"ALU 的計算結果"
        - 註: 寫在 Pipeline_stage5.v
    
*/



endmodule
>>>>>>> f65de8d3044d54d3aa91f2a19d5029f2c430844e:test_design/RISCV/bugged_src/ALUPipeline/RISCV_pipeline.v
