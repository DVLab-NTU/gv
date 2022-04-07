module multiplier(
        input [31:0]  multiplicand,
        input [31:0]  multiplier,
        input clk,
        input start,
        input rst_n,
        output [63:0] product,
        output reg stall,
        output reg finish

    );

reg signed [34:0]  HI, next_HI;   //35 bits
reg [34:0] LO, next_LO;
wire signed [36:0]  alu_out;      //37 bits
reg [5:0] counter, next_counter;
reg state, next_state;
reg signed [31:0] multipd_reg;       
reg signed [31:0] next_multiplicand; 
reg next_finish;
reg signed [35:0] part_pro;     // 36 bits
parameter Idle = 1'b0;
parameter Add = 1'b1;

// only for positive mult

assign product = {HI[29:0], LO[34:1]};
assign alu_out = {1'b0, HI} + part_pro ;  // 36 + 36 bit

always@(*)begin
    if (counter == 1'b1) begin
        case (LO[2:1])  // x
            2'b00: part_pro = {3'b100, 1'b0, 32'd0};                // 0
            2'b01: part_pro = {3'b100, 1'b0, multipd_reg};          // y
            2'b10: part_pro = {3'b011, ~multipd_reg, 1'b1} + 1;     // -2y
            2'b11: part_pro = {3'b011, 1'b1, ~multipd_reg} + 1;     // -y
        endcase
    end
    else begin
        case (LO[2:0])  // x
            3'b000: part_pro = {3'b011, 1'b0, 32'd0};               // 0
            3'b001: part_pro = {3'b011, 1'b0, multipd_reg};         // y
            3'b010: part_pro = {3'b011, 1'b0, multipd_reg};         // y
            3'b011: part_pro = {3'b011, multipd_reg, 1'b0};         // 2y
            3'b100: part_pro = {3'b010, ~multipd_reg, 1'b1} + 1;    // -2y
            3'b101: part_pro = {3'b010, 1'b1, ~multipd_reg} + 1 ;   // -y
            3'b110: part_pro = {3'b010, 1'b1, ~multipd_reg} + 1 ;   // -y
            3'b111: part_pro = {3'b011, 1'b0, 32'd0};
        endcase
    end
end

always@(*) begin
   next_multiplicand = multipd_reg;
   
   case(state)
        Idle : begin
            if (start == 1 & counter!= 18) begin
                next_state      = Add;
                next_LO         = {2'b00, multiplier, 1'b0};
                next_multiplicand = multiplicand;
                next_HI         = 35'b0;
                next_counter    = 1;
                next_finish     = 0;
                stall           = 1;
            end
           else begin
                next_state           = Idle;
                next_counter         = 0;
                next_multiplicand    = 32'd0;
                next_HI              = 35'd0;
                next_LO              = 35'd0;
                next_finish          = 0;
                stall                = 0;
           end
        end

        Add : begin     
            next_LO = {alu_out[1:0], LO[34:2]};
            next_HI = alu_out[36:2];              // 35 bits
            if (counter < 6'd17) begin // 1, 2 ,...31 
                next_finish     = 0;
                next_state      = Add;
                next_counter    = counter + 1;
                stall           = 1;
            end
            else begin  // 17
                next_state      = Idle;
                next_counter    = 18;
                next_finish     = 1;
                stall           = 1;
            end
        end
    endcase
end

always@(posedge clk)begin
    if (!rst_n) begin
        LO          <= 35'd0; 
        HI          <= 35'd0;
        counter     <= 6'd0;
        finish      <= 1'b0;
        state       <= Idle;
        multipd_reg <= 32'd0;
    end
    else begin
        LO          <= next_LO; 
        HI          <= next_HI;
        counter     <= next_counter;
        state       <= next_state;
        multipd_reg <= next_multiplicand;
        finish      <= next_finish;
    end
end

endmodule