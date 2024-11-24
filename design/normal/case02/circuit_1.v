module top( n1 , n4 , n5 , n6 , n11 , n16 , n19 , n24 , n35 , 
n36 , n39 , n44 , n45 , n46 , n48 , n49 );
    input n1 , n4 , n5 , n11 , n19 , n24 , n35 , n39 , n45 , 
n46 , n48 , n49 ;
    output n6 , n16 , n36 , n44 ;
    wire n0 , n2 , n3 , n7 , n8 , n9 , n10 , n12 , n13 , 
n14 , n15 , n17 , n18 , n20 , n21 , n22 , n23 , n25 , n26 , 
n27 , n28 , n29 , n30 , n31 , n32 , n33 , n34 , n37 , n38 , 
n40 , n41 , n42 , n43 , n47 ;
    not g0 ( n38 , n11 );
    xnor g1 ( n43 , n47 , n49 );
    not g2 ( n10 , n30 );
    xnor g3 ( n6 , n7 , n23 );
    or g4 ( n34 , n26 , n8 );
    nor g5 ( n15 , n20 , n7 );
    xnor g6 ( n27 , n35 , n48 );
    xnor g7 ( n41 , n5 , n4 );
    xnor g8 ( n14 , n18 , n3 );
    xnor g9 ( n23 , n14 , n19 );
    xnor g10 ( n31 , n24 , n39 );
    nand g11 ( n13 , n35 , n48 );
    xnor g12 ( n25 , n0 , n11 );
    nor g13 ( n28 , n22 , n18 );
    xnor g14 ( n36 , n31 , n46 );
    and g15 ( n18 , n32 , n13 );
    not g16 ( n8 , n39 );
    and g17 ( n30 , n21 , n46 );
    nor g18 ( n22 , n45 , n1 );
    xnor g19 ( n0 , n34 , n27 );
    nor g20 ( n9 , n35 , n48 );
    xnor g21 ( n3 , n45 , n1 );
    or g22 ( n12 , n38 , n10 );
    and g23 ( n7 , n2 , n12 );
    and g24 ( n20 , n14 , n29 );
    not g25 ( n21 , n31 );
    or g26 ( n17 , n40 , n15 );
    or g27 ( n2 , n0 , n37 );
    or g28 ( n32 , n9 , n34 );
    xnor g29 ( n42 , n41 , n17 );
    xnor g30 ( n16 , n43 , n42 );
    and g31 ( n33 , n1 , n45 );
    nor g32 ( n37 , n11 , n30 );
    xnor g33 ( n44 , n10 , n25 );
    not g34 ( n26 , n24 );
    not g35 ( n29 , n19 );
    nor g36 ( n40 , n29 , n14 );
    or g37 ( n47 , n33 , n28 );
endmodule
