module a (reset, clk, z1, z2, z3, z4);
input reset, clk;
output z1, z2, z3, z4;

reg [7:0]x;
reg [7:0]y;

wire [7:0] sum;

assign z1 = (x == 8'd200);
assign z2 = !(x <= 8'd200);
assign z3 = (x > 8'd200);
assign z4 = (x == 8'd200) || (y == 8'd200);

always @(posedge clk) begin
   if (!reset) begin
      x <= 8'd1;
      y <= 8'd1;
   end
   else begin
      if (x < 8'd100) begin 
         x <= x + y;
      end
      y <= x;
   end
end
endmodule

