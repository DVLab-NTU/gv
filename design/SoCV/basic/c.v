module a (i, clk, z0, z1, z2, z3);
input i, clk;
output z0, z1, z2, z3;

reg x, y;
reg prev_i;
reg [2:0] counter;

assign z0 = (!x && !y && !i);
assign z1 = (counter == 3'd5 &&  x);
assign z2 = (counter == 3'd5 && !x);
assign z3 = (counter != 3'd0 && prev_i == x);

always @(posedge clk) begin
   x <= !x;
   y <= x;
   prev_i <= i;
   if (i == x) begin
     counter <= counter + 3'd1;
   end
   else begin
     counter <= 3'd0;
   end
end
endmodule

