module Memory(
    input         clk,
    input         rst_n,
    input         memory_stall,
    input [31:0]  ALU_result_3,
    input [31:0]  writedata_3,
    
    input [31:0]  D_readData,
    
    input         WriteBack_3,
    input [1:0]   Mem_3,
    input [4:0]   Rd_3,
    
    output [31:0] memory_result_4,
    output [31:0] ALU_result_4,
    output [31:0] forward_result_4,

    output [4:0]  Rd_4,
    output        WriteBack_4,
    output        Mem2Reg,
    
    output [29:0] D_addr,
    output        D_wen,
    output        D_ren,
    output [31:0] D_writeData
);

reg [31:0] memory_result4_r, memory_result4_w;
reg [31:0] ALU_result4_r, ALU_result4_w;
reg [4:0]  Rd4_r, Rd4_w;
reg        WriteBack4_r, WriteBack4_w;
reg        Mem2Reg4_r, Mem2Reg4_w;

assign memory_result_4  = memory_result4_r;
assign ALU_result_4     = ALU_result4_r;
assign Rd_4             = Rd4_r;
assign WriteBack_4      = WriteBack4_r;
assign Mem2Reg          = Mem2Reg4_r; // lw 

assign D_addr           = ALU_result_3[31:2];
assign D_wen            = Mem_3[0];
assign D_ren            = Mem_3[1];
assign D_writeData      = {writedata_3[7:0],writedata_3[15:8],writedata_3[23:16],writedata_3[31:24]}; //with little_end
assign forward_result_4 = Mem_3[1] ? {D_readData[7:0],D_readData[15:8],D_readData[23:16],D_readData[31:24]} : ALU_result_3;

always @(*) begin // data memory
    memory_result4_w = memory_stall ? memory_result4_r : {D_readData[7:0],D_readData[15:8],D_readData[23:16],D_readData[31:24]}; //with little_end
    ALU_result4_w    = memory_stall ? ALU_result4_r : ALU_result_3;
    Rd4_w            = memory_stall ? Rd4_r : Rd_3;
    WriteBack4_w     = memory_stall ? WriteBack4_r : WriteBack_3;    
    Mem2Reg4_w       = memory_stall ? Mem2Reg4_r : Mem_3[1];
end  

always @(posedge clk) begin
    if(!rst_n) begin
        memory_result4_r <= 32'd0;
        ALU_result4_r    <= 32'd0;
        Rd4_r            <= 5'd0;
        WriteBack4_r     <= 1'b0;
        Mem2Reg4_r       <= 1'b0;
    end
    else begin
        memory_result4_r <= memory_result4_w;
        ALU_result4_r    <= ALU_result4_w;
        Rd4_r            <= Rd4_w;
        WriteBack4_r     <= WriteBack4_w;
        Mem2Reg4_r       <= Mem2Reg4_w;
    end
end

// Add assertion here (ncverilog can read "psl" comments, and yosys cannot read it)
// psl default clock = (posedge clk);

// psl ERROR1_memory_stall_stage4: assert never {(memory_stall) && !(memory_result4_w == memory_result4_r)};
// psl ERROR2_memory_stall_stage4: assert never {(memory_stall) && !(ALU_result4_w == ALU_result4_r)};
// psl ERROR3_memory_stall_stage4: assert never {(memory_stall) && !(Rd4_w == Rd4_r)}; 
// psl ERROR4_memory_stall_stage4: assert never {(memory_stall) && !(WriteBack4_w == WriteBack4_r)};
// psl ERROR5_memory_stall_stage4: assert never {(memory_stall) && !(Mem2Reg4_w == Mem2Reg4_r)};


endmodule
