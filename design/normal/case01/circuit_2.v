module top ( x0 , x1 , y0 , y1 , z , u0 , u1 , w0 , w1 ) ;
    input x0 , x1 , y0 , y1 , z ;
    output u0 , u1 , w0 , w1 ;
    wire n1 , n2 , n3 ;
    nand g1 ( n1 , x1 , y0 ) ;
    nand g2 ( n2 , x0 , y1 ) ;
    nand g3 ( n3 , y0 , z ) ;
    and g4 ( u0 , y0 , x0 ) ;
    xor g5 ( u1 , n1 , n2 ) ;
    xnor g6 ( w0 , y1 , n3 ) ;
    xor g7 ( w1 , y0 , z ) ;
endmodule
