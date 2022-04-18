`timescale 1 ns/10 ps
`define CYCLE 10 // You can modify your clock frequency
`include "./booth.v"

module Booth_tb;

	reg clk;
	reg rst_n;
	reg [31:0] multiplicand;
    reg [31:0] multiplier;
    reg start;
    wire [63:0] product;
    wire stall;
    wire finish;

	multiplier MP(
        .multiplicand(multiplicand),
        .multiplier(multiplier),
        .clk(clk),
        .start(start),
        .rst_n(rst_n),
        .product(product),
        .stall(stall),
        .finish(finish)
    );
	
	initial begin
		$display("-----------------------------------------------------\n");
	 	$display("START!!! Simulation Start .....\n");
	 	$display("-----------------------------------------------------\n");
		
        $fsdbDumpfile("booth.fsdb");			
		$fsdbDumpvars(0,Booth_tb,"+mda");
		$fsdbDumpvars;
	
		clk = 0;
		rst_n = 1'b1;
		#(`CYCLE * 1) rst_n = 1'b0;
		#(`CYCLE * 1) rst_n = 1'b1;
        start = 1;
        multiplicand = 32'd100;
        multiplier = 32'd100;

		#(`CYCLE*100000) // calculate clock cycles for all operation (you can modify it)
		$display("============================================================================");
		$display("\n           Error!!! There is something wrong with your code ...!          ");
		$display("\n                       The test result is .....FAIL                     \n");
		$display("============================================================================");
		
	 	$finish;
	end
		
	always #(`CYCLE*0.5) clk = ~clk;

	always@(finish) begin
        if(finish === 1'b1) begin
            $display("=========== Simulation Finish !! ===========\n");
            $display("Product = %d\n", product);
            $display("============================================\n");
            $finish;
        end
    end
	
endmodule
