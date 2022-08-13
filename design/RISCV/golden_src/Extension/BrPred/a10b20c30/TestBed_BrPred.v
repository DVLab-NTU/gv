`timescale 1 ns/10 ps

module	TestBed(
	clk,
	rst,
	flush,
	stall,
	type,
	I_addr,
	addr,
	data,
	wen,
	error_num,
	duration,
	finish
);
	input			clk, rst;
	input	[29:0]	addr;
	input	[31:0]	data;
	input			wen;
	input           flush;
	input           stall;
	input           type;
	input   [29:0]  I_addr;

	output	[7:0]	error_num;
	output	[15:0]	duration;
	output			finish;
	reg				finish;
	reg		[1:0]	curstate;
	reg		[1:0]	nxtstate;
	reg		[7:0]	error_num,nxt_error_num;
	reg				state,state_next;
	
	reg     [15:0]  instruction_count_r, instruction_count_w;
	reg     [15:0]  stall_cycles;
	reg     [15:0]  flush_times;
	reg     [15:0]  branch_count;
	reg     [29:0]  prevAddress;

	parameter	state_A   = 2'b00;
	parameter	state_B   = 2'b01;
	parameter	state_C   = 2'b10;
	parameter	state_end = 2'b11;
	
	assign duration = 0;

	initial begin

		flush_times = 0;
		stall_cycles = 0;
		branch_count = 0;
	end

	always@( posedge clk or negedge rst )						// State-DFF
	begin
		if( ~rst )
		begin
			curstate <= state_A;
			error_num <= 8'd0;

			state <= 0;
		end
		else
		begin
			curstate <= nxtstate;
			error_num <= nxt_error_num;
			
			state <= state_next;
		end
	end
			
	always@(*)	// FSM for test
	begin
		finish = 1'b0;
		case( curstate )
		state_A   : begin
						nxt_error_num = error_num;
						nxtstate = curstate;
						if( addr==0 && wen && !state ) begin
							if (data == 0) begin
								$display("\nBranch Part A is complete.");
								nxtstate = state_B;
							end
							else begin
								nxt_error_num = error_num + 1;
							end	
						end
					end
		state_B   : begin
						nxt_error_num = error_num;
						nxtstate = curstate;
						if( addr==0 && wen && !state ) begin
							if (data == 0) begin
								$display("\nBranch Part B is complete.");
								nxtstate = state_C;
							end
							else begin
								nxt_error_num = error_num + 1;
							end	
						end
					end
		state_C   : begin
						nxt_error_num = error_num;
						nxtstate = curstate;
						if( addr==0 && wen && !state ) begin
							if (data == 0) begin
								$display("\nBranch Part C is complete.\n");
								nxtstate = state_end;
							end
							else begin
								nxt_error_num = error_num + 1;
							end	
						end
					end
		state_end :	begin
						finish = 1'b1;
						nxtstate = curstate;
						nxt_error_num = error_num;	
					end						
		endcase
	end
	
	always@(*)begin//sub-FSM (avoid the Dcache stall condition)
		case(state)
			1'b0:begin
				if(wen)
					state_next=1;
				else
					state_next=state;				
			end
			1'b1:begin
				if(!wen)
					state_next=0;
				else
					state_next=state;	
			end
		endcase
	end

	always@( negedge clk )						
	begin
		if(curstate == state_end) begin
			$display("--------------------------- Simulation FINISH !!---------------------------");
			$display("\n=========================== Performance Metric =============================\n");
			$display("Memory stall rate : %d (stalled) / %d (cycles) = %f%% \n", stall_cycles, duration, stall_cycles * 100.0 / duration );
			$display("       Flush rate : %d (flushed) / %d (instructions) = %f%% \n",flush_times, instruction_count_r, flush_times * 100.0 / instruction_count_r);
			$display("        Miss rate : %d (flushed) / %d (branch inst) = %f%% \n",flush_times, branch_count, flush_times * 100.0 / branch_count);
			$display("============================================================================\n");
			if (error_num) begin 
				$display("============================================================================");
				$display("\n (T_T) FAIL!! The simulation result is FAIL!!!\n");
				$display("============================================================================");
			end
			 else begin 
				$display("============================================================================");
				$display("\n \\(^o^)/ CONGRATULATIONS!!  The simulation result is PASS!!!\n");
				$display("============================================================================");
			end
		end
	end
endmodule