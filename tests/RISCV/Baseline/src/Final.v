`define DMEM_INIT "D_mem"
`define SDFFILE   "./CHIP_syn.sdf"
`define IMEM_INIT "I_mem_hasHazard"
`include "./CHIP.v"
`include "./slow_memory.v"
`include "./TestBed_hasHazard.v"

module Final(
    clk,
	rst_n,
	mem_in,
	//----------for slow_memD------------
	mem_rdata_D,
	mem_ready_D,

	//----------for slow_memI------------
	mem_rdata_I,
	mem_ready_I,

	//----------for TestBed--------------
	error_num,
	duration,
	finish,
);

input			clk, rst_n;
input	[127:0]	mem_rdata_D;
input			mem_ready_D;
input	[127:0]	mem_rdata_I;
input			mem_ready_I;
input   [32*64*4-1:0] mem_in;


output [7:0] error_num;
output [15:0] duration;
output finish;	


// for slow_memD
wire			mem_read_D;
wire 			mem_write_D;
wire	[31:4]	mem_addr_D;
wire 	[127:0]	mem_wdata_D;

// for slow_memI
wire 			mem_read_I;
wire 			mem_write_I;
wire	[31:4]	mem_addr_I;
wire	[127:0]	mem_wdata_I;

wire    [29:0]  ICACHE_addr;
wire	[29:0]	DCACHE_addr;
wire	[31:0]	DCACHE_wdata;
wire			DCACHE_wen;
wire  			instruction_flush;
wire          	memory_stall;
wire          	branchType;

CHIP chip0 (	clk,
				rst_n,
//----------for slow_memD------------	
				mem_read_D,
				mem_write_D,
				mem_addr_D,
				mem_wdata_D,
				mem_rdata_D,
				mem_ready_D,
//----------for slow_memI------------
				mem_read_I,
				mem_write_I,
				mem_addr_I,
				mem_wdata_I,
				mem_rdata_I,
				mem_ready_I,
//----------for TestBed--------------	
				ICACHE_addr,			
				DCACHE_addr,
				DCACHE_wdata,
				DCACHE_wen,
				instruction_flush,
				memory_stall,
				branchType
				);

	
slow_memory slow_memD(
    .clk        (clk)           ,
	.rst_n		(rst_n)			,
    .mem_read   (mem_read_D)    ,
    .mem_write  (mem_write_D)   ,
    .mem_addr   (mem_addr_D)    ,
    .mem_wdata  (mem_wdata_D)   ,
    .mem_rdata  (mem_rdata_D)   ,
    .mem_ready  (mem_ready_D)	,
	.mem_in		(mem_in)
);

slow_memory slow_memI(
    .clk        (clk)           ,
	.rst_n		(rst_n)			,
    .mem_read   (mem_read_I)    ,
    .mem_write  (mem_write_I)   ,
    .mem_addr   (mem_addr_I)    ,
    .mem_wdata  (mem_wdata_I)   ,
    .mem_rdata  (mem_rdata_I)   ,
    .mem_ready  (mem_ready_I)	,
	.mem_in		(mem_in)
);

TestBed testbed(
    .clk        (clk)           ,
    .rst        (rst_n)         ,
    .flush      (instruction_flush),
    .stall      (memory_stall)  ,
    .type       (branchType)    ,
    .I_addr     (ICACHE_addr)   ,
    .addr       (DCACHE_addr)   ,
    .data       (DCACHE_wdata)  ,
    .wen        (DCACHE_wen)    ,
    .error_num  (error_num)     ,
    .duration   (duration)      ,
    .finish     (finish)        
);

endmodule