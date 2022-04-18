`timescale 1 ns/10 ps

`define	TestPort1	30'h0 
`define	TestPort2	30'h1  
`define	TestPort3	30'h2  
`define	TestPort4	30'h3  
`define	TestPort5	30'h4 
`define	TestPort6	30'h5  
`define	TestPort7	30'h6  
`define	TestPort8	30'h7  
`define	TestPort9	30'h8 
`define	TestPort10	30'h9  
`define	TestPort11	30'hA  
`define	TestPort12	30'hB  
`define	TestPort13	30'hC 
`define	TestPort14	30'hD  

`define	answer1	 32'hFFFFFFFE
`define	answer2	 32'h00000002
`define	answer3	 32'h00000007
`define	answer4	 32'h00000001
`define	answer5	 32'hFFFFFFFF
`define	answer6	 32'h00000003
`define	answer7	 32'h00000000
`define	answer8	 32'hFFFFFFFF
`define	answer9	 32'hFFFFFFFF
`define	answer10 32'hFFFFFFFC
`define	answer11 32'h00000001
`define	answer12 32'hFFFFFFFF
`define	answer13 32'h00000000
`define	answer14 32'hFFFFFFFE

`define	CheckNum	6'd13

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
	reg		[7:0]	error_num;
	reg		[15:0]	duration;
	reg				finish;
	
	reg		[1:0]	curstate;
	reg		[1:0]	nxtstate;
	reg		[5:0]	curaddr;
	reg		[5:0]	nxtaddr;
	reg		[15:0]	nxtduration;
	reg		[7:0]	nxt_error_num;
	reg				state,state_next;
	reg     [15:0]  instruction_count_r, instruction_count_w;
	reg     [15:0]  stall_cycles;
	reg     [15:0]  flush_times;
	reg     [15:0]  branch_count;
	reg     [29:0]  prevAddress;
	wire     [31:0]  data_modify;
		
	parameter	state_idle 	= 2'b00;
	parameter	state_check= 2'b01;
	parameter	state_report= 2'b10;	
		
	assign data_modify = {data[7:0],data[15:8],data[23:16],data[31:24]}; // convert little-endian format to readable format

	initial begin
		duration 	= 0;
		flush_times = 0;
		stall_cycles = 0;
		branch_count = 0;
	end

	always@( posedge clk or negedge rst )						// State-DFF
	begin
		if( ~rst )
		begin
			instruction_count_r <= 16'd0;
			curstate <= state_idle;
			curaddr  <= 0;
			duration <= 0;
			error_num <= 8'd255;
			
			state <= 0;
		end
		else
		begin
			instruction_count_r <= instruction_count_w;
			curstate <= nxtstate;
			curaddr  <= nxtaddr;
			duration <= nxtduration;
			error_num <= nxt_error_num;
			
			state <= state_next;
		end
	end
			
	always@(*)	// FSM for test
	begin
		if(prevAddress !== I_addr)
			instruction_count_w = instruction_count_r + 1;
		finish = 1'b0;
		case( curstate )
		state_idle: 	begin
							nxtaddr = 0;
							nxtduration = 0;
							nxt_error_num = 255;	
							if( addr==`TestPort1 && data_modify==`answer1 && wen )
							begin
								nxt_error_num = 0;
								nxtstate = state_check;
								nxtaddr = 1;
							end	 	
							else nxtstate = state_idle;
						end
		state_check:	begin
							nxtduration = duration + 1;
							nxtaddr = curaddr;						
							nxt_error_num = error_num;	
							if( addr==`TestPort2 && wen && state==0 )
							begin
								nxtaddr = addr + 1; 
								if( data_modify != `answer2 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end
							else if( addr==`TestPort3 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer3 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end
							else if( addr==`TestPort4 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer4 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort5 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer5 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort6 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer6 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort7 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer7 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end
							else if( addr==`TestPort8 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer8 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end
							else if( addr==`TestPort9 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer9 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end
							else if( addr==`TestPort10 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer10 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort11 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer11 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort12 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer12 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort13 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer13 )
									nxt_error_num = error_num + 8'd1;
							end
							else if( addr==`TestPort14 && wen && state==0 )
							begin
								nxtaddr = addr + 1;
								if( data_modify != `answer14 ) begin
									nxt_error_num = error_num + 8'd1;
								end
							end

							nxtstate = curstate;
							if( addr==`CheckNum )	
								nxtstate = state_report;
						end
		state_report:	begin
							finish = 1'b1;
							nxtaddr = curaddr;
							nxtstate = curstate;		
							nxtduration = duration;
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
		duration = duration + 1;
		if(stall)
			stall_cycles = stall_cycles + 1;
		if(flush)
			flush_times = flush_times + 1;
		if(type)
			branch_count = branch_count + 1;

		if(curstate == state_report) begin
			$display("--------------------------- Simulation FINISH !!---------------------------");
			$display("\n=========================== Performance Metric =============================\n");
			$display("Memory stall rate : %d (stalled) / %d (cycles) = %f%% \n", stall_cycles, duration, stall_cycles * 100.0 / duration );
			$display("       Flush rste : %d (flushed) / %d (instructions) = %f%% \n",flush_times, instruction_count_r, flush_times * 100.0 / instruction_count_r);
			$display("        Miss rate : %d (flushed) / %d (branch inst) = %f%% \n",flush_times, branch_count, flush_times * 100.0 / branch_count);
			$display("============================================================================\n");
			if (error_num) begin 
				$display("============================================================================");
				$display("\n (T_T) FAIL!! The simulation result is FAIL!!! there were %d errors at all.\n", error_num);
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
