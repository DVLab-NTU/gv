module arbiter2(clk, rst, req1, req2, gnt1, gnt2); 
    input clk, rst; 
    input req1, req2; 
    output gnt1, gnt2; 
    reg state; 
    reg gnt1, gnt2; 
    always @(posedge clk or posedge rst) begin 
        if (rst) state <= 0; 
        else state <= gnt1; 
    end 
    always @(*) begin 
        if (state) begin 
            gnt1 = req1; // bug 
            // gnt1 = req1 & ~req2; // golden 
            gnt2 = req2;
        end 
        else begin 
            gnt1 = req1; 
            gnt2 = req2 & ~req1; 
        end 
    end 
endmodule 