module Writeback( // no sequential circuits
    input [31:0]  ALU_result_4,
    input [31:0]  memory_result_4,
    input         WriteBack_4,
    input         Mem2Reg,
    input [4:0]   Rd_4,
    
    output [4:0]  Rd_5,
    output [31:0] writeback_data_5,
    output        WriteBack_5
);
    assign Rd_5             = Rd_4;
    assign writeback_data_5 = Mem2Reg ? memory_result_4 : ALU_result_4;
    assign WriteBack_5      = WriteBack_4;

endmodule