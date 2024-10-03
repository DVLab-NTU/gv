module adder (
    input wire clk,
    input wire rst,
    input wire [3:0] a,
    input wire [3:0] b,
    output reg [4:0] sum
);

always @(posedge clk) begin
    if (!rst) begin
        sum <= 5'b00000; // Reset state
    end
    else begin
        sum <= a + b; // Add a and b when not in reset
    end
end

endmodule
