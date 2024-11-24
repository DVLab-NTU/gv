module top( n3 , n4 , n6 , n9 , n10 , n13 , n15 , n18 , n30 , 
n31 , n33 , n38 , n40 , n44 , n48 , n49 );
    input n3 , n4 , n9 , n13 , n15 , n30 , n31 , n38 , n40 , 
n44 , n48 , n49 ;
    output n6 , n10 , n18 , n33 ;
    wire n0 , n1 , n2 , n5 , n7 , n8 , n11 , n12 , n14 , 
n16 , n17 , n19 , n20 , n21 , n22 , n23 , n24 , n25 , n26 , 
n27 , n28 , n29 , n32 , n34 , n35 , n36 , n37 , n39 , n41 , 
n42 , n43 , n45 , n46 , n47 ;
    nor g0 ( n41 , n49 , n48 );
    xnor g1 ( n33 , n24 , n27 );
    or g2 ( n43 , n45 , n42 );
    and g3 ( n19 , n22 , n11 );
    and g4 ( n39 , n1 , n21 );
    or g5 ( n22 , n17 , n20 );
    nor g6 ( n2 , n35 , n39 );
    xnor g7 ( n8 , n9 , n40 );
    not g8 ( n17 , n44 );
    and g9 ( n34 , n31 , n46 );
    nor g10 ( n35 , n15 , n38 );
    not g11 ( n45 , n3 );
    xnor g12 ( n18 , n20 , n5 );
    xnor g13 ( n27 , n8 , n47 );
    nor g14 ( n26 , n44 , n34 );
    and g15 ( n0 , n37 , n19 );
    not g16 ( n37 , n30 );
    xnor g17 ( n10 , n23 , n31 );
    xnor g18 ( n12 , n39 , n16 );
    nor g19 ( n14 , n12 , n0 );
    not g20 ( n46 , n23 );
    not g21 ( n20 , n34 );
    not g22 ( n42 , n4 );
    xnor g23 ( n28 , n43 , n7 );
    or g24 ( n11 , n26 , n28 );
    nand g25 ( n1 , n49 , n48 );
    xnor g26 ( n23 , n3 , n4 );
    or g27 ( n29 , n25 , n2 );
    nor g28 ( n32 , n37 , n19 );
    or g29 ( n21 , n43 , n41 );
    xnor g30 ( n16 , n15 , n38 );
    xnor g31 ( n24 , n29 , n13 );
    xnor g32 ( n36 , n12 , n30 );
    xnor g33 ( n6 , n19 , n36 );
    xnor g34 ( n7 , n49 , n48 );
    xnor g35 ( n5 , n28 , n44 );
    or g36 ( n47 , n32 , n14 );
    and g37 ( n25 , n15 , n38 );
endmodule
