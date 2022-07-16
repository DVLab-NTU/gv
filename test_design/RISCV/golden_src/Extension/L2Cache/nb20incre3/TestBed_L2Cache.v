`timescale 1 ns/10 ps
`define TestPort    30'h3FF      // 1023
`define BeginSymbol 32'h00000168
`define EndSymbol   32'hFFFFFD5D
`define CheckNum    10'd161

module	TestBed(
	clk,
	rst,
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

	output	[7:0]	error_num;
	output	[15:0]	duration;
	output			finish;
	reg		[7:0]	error_num;
	reg		[15:0]	duration;
	reg				finish;
	
	reg		[31:0]	answer;

	reg		[1:0]	curstate;
	reg		[1:0]	nxtstate;
	reg		[9:0]	curaddr;
	reg		[9:0]	nxtaddr;
	reg		[7:0]	nxt_error_num;
	reg		[15:0]	nxtduration;
	
	reg				state,state_next;

	wire    [31:0]  data_modify;
		
	parameter	state_idle 	= 2'b00;
	parameter	state_check = 2'b01;
	parameter	state_report= 2'b10;	

	assign data_modify = {data[7:0],data[15:8],data[23:16],data[31:24]}; // convert little-endian format to readable format
		
	always@( posedge clk or negedge rst )						// State-DFF
	begin
		if( ~rst )
		begin
			curstate <= state_idle;
			curaddr  <= 0;
			duration <= 0;
			error_num <= 8'd255;
			
			state <= 0;
		end
		else
		begin
			curstate <= nxtstate;
			curaddr  <= nxtaddr;
			duration <= nxtduration;
			error_num <= nxt_error_num;
			
			state <= state_next;
		end
	end
			
	always@(*)	// FSM for test
	begin
		finish = 1'b0;
		case( curstate )
		state_idle: 	begin
							nxtaddr = 0;
							nxtduration = 0;
							nxt_error_num = 255;	
							if( addr==`TestPort && data_modify==`BeginSymbol && wen )
							begin
								nxt_error_num = 0;
								nxtstate = state_check;
							end	 	
							else nxtstate = state_idle;
						end
		state_check:	begin
							nxtduration = duration + 1;
							nxtaddr = curaddr;						
							nxt_error_num = error_num;	
							if( addr==`TestPort && wen && state==0 )
							begin
								nxtaddr = curaddr + 1;
								if( data_modify != answer )
									nxt_error_num = error_num + 8'd1;
							end
							nxtstate = curstate;
							if( curaddr==`CheckNum )	
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

	always@( negedge clk )						
	begin
		if(curstate == state_report) begin
			$display("--------------------------- Simulation FINISH !!---------------------------");
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
	
	
	always@(*)	// ROM for correct result
	begin
		answer = 0;
		case( curaddr )
		10'd0   : answer = 32'd0     ;
		10'd1   : answer = 32'd1     ;
		10'd2   : answer = 32'd2     ;
		10'd3   : answer = 32'd3     ;
		10'd4   : answer = 32'd1     ;
		10'd5   : answer = 32'd2     ;
		10'd6   : answer = 32'd3     ;
		10'd7   : answer = 32'd4     ;
		10'd8   : answer = 32'd1     ;
		10'd9   : answer = 32'd2     ;
		10'd10  : answer = 32'd3     ;
		10'd11  : answer = 32'd4     ;
		10'd12  : answer = 32'd2     ;
		10'd13  : answer = 32'd3     ;
		10'd14  : answer = 32'd4     ;
		10'd15  : answer = 32'd5     ;
		10'd16  : answer = 32'd3     ;
		10'd17  : answer = 32'd4     ;
		10'd18  : answer = 32'd5     ;
		10'd19  : answer = 32'd6     ;
		10'd20  : answer = 32'd5     ;
		10'd21  : answer = 32'd6     ;
		10'd22  : answer = 32'd7     ;
		10'd23  : answer = 32'd8     ;
		10'd24  : answer = 32'd8     ;
		10'd25  : answer = 32'd9     ;
		10'd26  : answer = 32'd10    ;
		10'd27  : answer = 32'd11    ;
		10'd28  : answer = 32'd13    ;
		10'd29  : answer = 32'd14    ;
		10'd30  : answer = 32'd15    ;
		10'd31  : answer = 32'd16    ;
		10'd32  : answer = 32'd21    ;
		10'd33  : answer = 32'd22    ;
		10'd34  : answer = 32'd23    ;
		10'd35  : answer = 32'd24    ;
		10'd36  : answer = 32'd34    ;
		10'd37  : answer = 32'd35    ;
		10'd38  : answer = 32'd36    ;
		10'd39  : answer = 32'd37    ;
		10'd40  : answer = 32'd55    ;
		10'd41  : answer = 32'd56    ;
		10'd42  : answer = 32'd57    ;
		10'd43  : answer = 32'd58    ;
		10'd44  : answer = 32'd89    ;
		10'd45  : answer = 32'd90    ;
		10'd46  : answer = 32'd91    ;
		10'd47  : answer = 32'd92    ;
		10'd48  : answer = 32'd144   ;
		10'd49  : answer = 32'd145   ;
		10'd50  : answer = 32'd146   ;
		10'd51  : answer = 32'd147   ;
		10'd52  : answer = 32'd233   ;
		10'd53  : answer = 32'd234   ;
		10'd54  : answer = 32'd235   ;
		10'd55  : answer = 32'd236   ;
		10'd56  : answer = 32'd377   ;
		10'd57  : answer = 32'd378   ;
		10'd58  : answer = 32'd379   ;
		10'd59  : answer = 32'd380   ;
		10'd60  : answer = 32'd610   ;
		10'd61  : answer = 32'd611   ;
		10'd62  : answer = 32'd612   ;
		10'd63  : answer = 32'd613   ;
		10'd64  : answer = 32'd987   ;
		10'd65  : answer = 32'd988   ;
		10'd66  : answer = 32'd989   ;
		10'd67  : answer = 32'd990   ;
		10'd68  : answer = 32'd1597  ;
		10'd69  : answer = 32'd1598  ;
		10'd70  : answer = 32'd1599  ;
		10'd71  : answer = 32'd1600  ;
		10'd72  : answer = 32'd2584  ;
		10'd73  : answer = 32'd2585  ;
		10'd74  : answer = 32'd2586  ;
		10'd75  : answer = 32'd2587  ;
		10'd76  : answer = 32'd4181  ;
		10'd77  : answer = 32'd4182  ;
		10'd78  : answer = 32'd4183  ;
		10'd79  : answer = 32'd4184  ;
		10'd80  : answer = 32'd4184  ;
		10'd81  : answer = 32'd4183  ;
		10'd82  : answer = 32'd4182  ;
		10'd83  : answer = 32'd4181  ;
		10'd84  : answer = 32'd2587  ;
		10'd85  : answer = 32'd2586  ;
		10'd86  : answer = 32'd2585  ;
		10'd87  : answer = 32'd2584  ;
		10'd88  : answer = 32'd1600  ;
		10'd89  : answer = 32'd1599  ;
		10'd90  : answer = 32'd1598  ;
		10'd91  : answer = 32'd1597  ;
		10'd92  : answer = 32'd990   ;
		10'd93  : answer = 32'd989   ;
		10'd94  : answer = 32'd988   ;
		10'd95  : answer = 32'd987   ;
		10'd96  : answer = 32'd613   ;
		10'd97  : answer = 32'd612   ;
		10'd98  : answer = 32'd611   ;
		10'd99  : answer = 32'd610   ;
		10'd100 : answer = 32'd380   ;
		10'd101 : answer = 32'd379   ;
		10'd102 : answer = 32'd378   ;
		10'd103 : answer = 32'd377   ;
		10'd104 : answer = 32'd236   ;
		10'd105 : answer = 32'd235   ;
		10'd106 : answer = 32'd234   ;
		10'd107 : answer = 32'd233   ;
		10'd108 : answer = 32'd147   ;
		10'd109 : answer = 32'd146   ;
		10'd110 : answer = 32'd145   ;
		10'd111 : answer = 32'd144   ;
		10'd112 : answer = 32'd92    ;
		10'd113 : answer = 32'd91    ;
		10'd114 : answer = 32'd90    ;
		10'd115 : answer = 32'd89    ;
		10'd116 : answer = 32'd58    ;
		10'd117 : answer = 32'd57    ;
		10'd118 : answer = 32'd56    ;
		10'd119 : answer = 32'd55    ;
		10'd120 : answer = 32'd37    ;
		10'd121 : answer = 32'd36    ;
		10'd122 : answer = 32'd35    ;
		10'd123 : answer = 32'd34    ;
		10'd124 : answer = 32'd24    ;
		10'd125 : answer = 32'd23    ;
		10'd126 : answer = 32'd22    ;
		10'd127 : answer = 32'd21    ;
		10'd128 : answer = 32'd16    ;
		10'd129 : answer = 32'd15    ;
		10'd130 : answer = 32'd14    ;
		10'd131 : answer = 32'd13    ;
		10'd132 : answer = 32'd11    ;
		10'd133 : answer = 32'd10    ;
		10'd134 : answer = 32'd9     ;
		10'd135 : answer = 32'd8     ;
		10'd136 : answer = 32'd8     ;
		10'd137 : answer = 32'd7     ;
		10'd138 : answer = 32'd6     ;
		10'd139 : answer = 32'd6     ;
		10'd140 : answer = 32'd5     ;
		10'd141 : answer = 32'd5     ;
		10'd142 : answer = 32'd5     ;
		10'd143 : answer = 32'd4     ;
		10'd144 : answer = 32'd4     ;
		10'd145 : answer = 32'd4     ;
		10'd146 : answer = 32'd4     ;
		10'd147 : answer = 32'd3     ;
		10'd148 : answer = 32'd3     ;
		10'd149 : answer = 32'd3     ;
		10'd150 : answer = 32'd3     ;
		10'd151 : answer = 32'd3     ;
		10'd152 : answer = 32'd2     ;
		10'd153 : answer = 32'd2     ;
		10'd154 : answer = 32'd2     ;
		10'd155 : answer = 32'd2     ;
		10'd156 : answer = 32'd1     ;
		10'd157 : answer = 32'd1     ;
		10'd158 : answer = 32'd1     ;
		10'd159 : answer = 32'd0     ;
		10'd160 : answer = `EndSymbol;
		endcase
	end
endmodule