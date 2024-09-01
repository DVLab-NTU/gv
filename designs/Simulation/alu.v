module alu #(
    parameter INT_W  = 3,
    parameter FRAC_W = 5,
    parameter INST_W = 3,
    parameter DATA_W = INT_W + FRAC_W
)(
    input                     i_clk,
    input                     i_rst_n,
    input                     i_valid,
    input signed [DATA_W-1:0] i_data_a,
    input signed [DATA_W-1:0] i_data_b,
    input        [INST_W-1:0] i_inst,
    output                    o_valid,
    output       [DATA_W-1:0] o_data
);
    
// ref: Tony Tony (score 100)
// FIXME: I only put my bug code at "i_inst = 110"

// ---------------------------------------------------------------------------
// Wires and Registers
// ---------------------------------------------------------------------------
reg  [DATA_W:0] o_data_w, o_data_r;
reg             o_valid_w, o_valid_r;
reg  signed [(DATA_W + DATA_W):0] temp_multiply;
reg         [DATA_W-1:0]   mod_reg;
// ---- Add your own wires and registers here if needed ---- //


// ---------------------------------------------------------------------------
// Continuous Assignment
// ---------------------------------------------------------------------------
assign o_valid = o_valid_r;
assign o_data = o_data_r;
// ---- Add your own wire data assignments here if needed ---- //


// ---------------------------------------------------------------------------
// Combinational Blocks
// ---------------------------------------------------------------------------
// ---- Write your combinational block design here ---- //
always@(*) begin
    o_valid_w = 1'b0;
    if(i_valid) begin
    	case(i_inst)
    	    3'b000: begin// signed addition
    	    	o_data_w = i_data_a + i_data_b;
    	    	if(i_data_a[DATA_W-1] == 1'b0 && i_data_b[DATA_W-1] == 1'b0 && o_data_w[DATA_W-1] == 1'b1) begin 
    	    		o_data_w = 9'b001111111;
    	    	end
    	    	else if(i_data_a[DATA_W-1] == 1'b1 && i_data_b[DATA_W-1] == 1'b1 && o_data_w[DATA_W-1] == 1'b0) begin
    	    		o_data_w = 9'b010000000;
    	    	end 
    	    end
    	    3'b001: begin// signed subtraction
    	    	o_data_w = i_data_a - i_data_b;
    	    	if(i_data_a[DATA_W-1] == 1'b0 && i_data_b[DATA_W-1] == 1'b1 && o_data_w[DATA_W-1] == 1'b1) begin
    	    		o_data_w = 9'b001111111;
    	    	end
    	    	else if(i_data_a[DATA_W-1] == 1'b1 && i_data_b[DATA_W-1] == 1'b0 && o_data_w[DATA_W-1] == 1'b0) begin
    	    		o_data_w = 9'b010000000;
    	    	end
    	    end
    	    3'b010: begin// signed multiplication
    	    	temp_multiply = $signed(i_data_a) * $signed(i_data_b);
    	    	if(temp_multiply[FRAC_W-1] == 1'd1) begin
                    if ((temp_multiply[(DATA_W + DATA_W)] == 1'd0) || ((temp_multiply[(DATA_W + DATA_W)] == 1'd1) && (temp_multiply[FRAC_W-2:0] != 0))) begin
    	    		    temp_multiply = temp_multiply +5'b10000;
                    end
    	    	end
    	    	if($signed(temp_multiply[(DATA_W + DATA_W):FRAC_W]) > $signed(8'b01111111)) begin
    	    		o_data_w = 9'b001111111;
    	    	end
    	    	else if($signed(temp_multiply[(DATA_W + DATA_W):FRAC_W]) < $signed(8'b10000000)) begin
    	    		o_data_w = 9'b010000000;
    	    	end
    	    	else begin
    	    		o_data_w = temp_multiply[(DATA_W + DATA_W):FRAC_W];
    	    	end
    	    end	
    	    3'b011: begin// NAND
    	    	o_data_w = ~(i_data_a & i_data_b);
    	    end
    	    3'b100: begin// XNOR
    	    	o_data_w = ~(i_data_a ^ i_data_b);
    	    end
    	    3'b101: begin// Sigmoid
    	    	if(i_data_a >= $signed(8'b01000000))begin
    	    		o_data_w = 9'b000100000;
    	    	end
    	    	else if(i_data_a <= $signed(8'b11000000)) begin
    	    		o_data_w = 9'b000000000;
    	    	end
    	    	else begin// 1/4x+1/2
    	    		// o_data_w = {1'b0, (i_data_a >>> 2) + $signed(8'b00010000)};  // FIXME: golden -> arithmetic right shift (pad with sign bit)
					o_data_w = {1'b0, (i_data_a >> 2) + $signed(8'b00010000)};  // FIXME: bug -> logical right shift (pad with 0s)
    	    	end
    	    end
    	    3'b110: begin// Right Circular Shift
    	    	mod_reg = $unsigned(i_data_b) % (DATA_W);
                if (mod_reg == 0) begin
                    o_data_w = i_data_a;
                end
                else begin 
                    o_data_w = ({8'b0, i_data_a} >> mod_reg) | ({8'b0, i_data_a} << (DATA_W-mod_reg));
                end
                o_valid_w = 1; 
    	    end
    	    3'b111: begin// Min
    	    	o_data_w = {1'b0, ((i_data_a < i_data_b) ? i_data_a : i_data_b)};
    	    end
    	    default: begin
    	    	o_data_w = o_data_r;
    	    end
    	endcase
    	o_valid_w = 1'b1;
    end
end

// ---------------------------------------------------------------------------
// Sequential Block
// ---------------------------------------------------------------------------
// ---- Write your sequential block design here ---- //
always@(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        o_data_r  <= 0;
        o_valid_r <= 0;
    end else begin
        o_data_r  <= o_data_w;
        o_valid_r <= o_valid_w;
    end
end
endmodule
