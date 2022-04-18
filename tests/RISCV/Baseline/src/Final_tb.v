// this is a test bench feeds initial instruction and data
// the processor output is not verified

`timescale 1 ns/10 ps

`define CYCLE 10 // You can modify your clock frequency

`define DMEM_INIT "D_mem"
`define SDFFILE   "./CHIP_syn.sdf"	// Modify your SDF file name

// For different condition (I_mem, TestBed)

`define IMEM_INIT "I_mem_hasHazard"
`include "./TestBed_hasHazard.v"
`include "./ALUPipeline/RISCV_pipeline.v"


module Final_tb;

	reg clk;
	reg rst_n;
	
	wire mem_read_D;
	wire mem_write_D;
	wire [31:4] mem_addr_D;
	wire [127:0] mem_wdata_D;
	wire [127:0] mem_rdata_D;
	wire mem_ready_D;

	wire mem_read_I;
	wire mem_write_I;
	wire [31:4] mem_addr_I;
	wire [127:0] mem_wdata_I;
	wire [127:0] mem_rdata_I;
	wire mem_ready_I;
	
	wire [29:0] ICACHE_addr;
	wire [29:0]	DCACHE_addr;
	wire [31:0]	DCACHE_wdata;
	wire 		DCACHE_wen;
	
	wire [7:0] error_num;
	wire [15:0] duration;
	wire finish;	
	wire instruction_flush;
	wire memory_stall;
	wire branchType;
	// Note the design is connected at testbench, include:
	// 1. CHIP (RISCV + D_cache + I_chache)
	// 2. slow memory for data
	// 3. slow memory for instruction
	
	CHIP chip0 (clk,
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
		.mem_read   (mem_read_D)    ,
		.mem_write  (mem_write_D)   ,
		.mem_addr   (mem_addr_D)    ,
		.mem_wdata  (mem_wdata_D)   ,
		.mem_rdata  (mem_rdata_D)   ,
		.mem_ready  (mem_ready_D)
	);

	slow_memory slow_memI(
		.clk        (clk)           ,
		.mem_read   (mem_read_I)    ,
		.mem_write  (mem_write_I)   ,
		.mem_addr   (mem_addr_I)    ,
		.mem_wdata  (mem_wdata_I)   ,
		.mem_rdata  (mem_rdata_I)   ,
		.mem_ready  (mem_ready_I)
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
	
`ifdef SDF
    initial $sdf_annotate(`SDFFILE, chip0);
`endif
	
// Initialize the data memory
	initial begin
		$display("-----------------------------------------------------\n");
	 	$display("START!!! Simulation Start .....\n");
	 	$display("-----------------------------------------------------\n");
		// $readmemb (`DMEM_INIT, slow_memD.mem ); // initialize data in DMEM
		// $readmemh (`IMEM_INIT, slow_memI.mem ); // initialize data in IMEM

		// // waveform dump
	    // // $dumpfile("Final.vcd");
	    // // $dumpvars;
	    // $fsdbDumpfile("Final.fsdb");			
		// $fsdbDumpvars(0,Final_tb,"+mda");
		// $fsdbDumpvars;
	
		// clk = 0;
		// rst_n = 1'b1;
		// #(`CYCLE*0.2) rst_n = 1'b0;
		// #(`CYCLE*8.5) rst_n = 1'b1;

		// #(`CYCLE*100000) // calculate clock cycles for all operation (you can modify it)
		// $display("============================================================================");
		// $display("\n           Error!!! There is something wrong with your code ...!          ");
		// $display("\n                       The test result is .....FAIL                     \n");
		// $display("============================================================================");
		// if (testbed.curstate == 2'b0)
		// 	$display("Possible solution: The first answer may not be correct.\n");
		// if (testbed.curstate == 2'b1)
		// 	$display("Possible solution: The clock cycles may be too small. Please modify it.\n");
	 	// $finish;
	end
		
	// always #(`CYCLE*0.5) clk = ~clk;

	// always@(finish)
	//     if(finish)
	//        #(`CYCLE) $finish;		   
	
endmodule
