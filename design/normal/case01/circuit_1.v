module top ( a0 , a1 , b0 , b1 , c , h0 , h1 , m0 , m1 ) ;
    input a0 , a1 , b1 , b0 , c ;
    output h0 , h1 , m0 , m1 ;
    wire n1 , n2 , n3 , n4 , n5 ;
    not g1 ( n1 , b1 ) ;
    not g2 ( n2 , a0 ) ;
    and g3 ( h0 , a0 , b0 ) ;
    nand g4 ( n3 , a1 , b0 ) ;
    or g5 ( n4 , n1 , n2 ) ;
    nand g6 ( n5 , b0 , c ) ;
    xor g7 ( h1 , n3 , n4 ) ;
    xnor g8 ( m0 , b1 , n5 ) ;
    xor g9 ( m1 , b0 , c ) ;
endmodule

