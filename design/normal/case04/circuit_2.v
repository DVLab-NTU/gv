module top( n4 , n14 , n23 , n25 , n28 , n32 , n34 , n38 , n40 , 
n50 , n55 , n58 , n59 , n60 , n63 , n65 );
    input n4 , n23 , n25 , n28 , n32 , n38 , n50 , n55 , n58 , 
n60 , n63 , n65 ;
    output n14 , n34 , n40 , n59 ;
    wire n0 , n1 , n2 , n3 , n5 , n6 , n7 , n8 , n9 , 
n10 , n11 , n12 , n13 , n15 , n16 , n17 , n18 , n19 , n20 , 
n21 , n22 , n24 , n26 , n27 , n29 , n30 , n31 , n33 , n35 , 
n36 , n37 , n39 , n41 , n42 , n43 , n44 , n45 , n46 , n47 , 
n48 , n49 , n51 , n52 , n53 , n54 , n56 , n57 , n61 , n62 , 
n64 , n66 , n67 , n68 ;
    nor g0 ( n49 , n47 , n13 );
    xnor g1 ( n12 , n1 , n47 );
    xnor g2 ( n57 , n8 , n23 );
    nor g3 ( n62 , n0 , n45 );
    or g4 ( n56 , n37 , n44 );
    nand g5 ( n22 , n32 , n60 );
    not g6 ( n44 , n25 );
    or g7 ( n47 , n3 , n19 );
    or g8 ( n46 , n65 , n61 );
    or g9 ( n33 , n53 , n31 );
    or g10 ( n31 , n3 , n44 );
    or g11 ( n66 , n37 , n36 );
    not g12 ( n2 , n58 );
    not g13 ( n11 , n7 );
    not g14 ( n51 , n32 );
    not g15 ( n9 , n5 );
    xnor g16 ( n68 , n26 , n21 );
    or g17 ( n7 , n51 , n44 );
    nor g18 ( n42 , n23 , n52 );
    or g19 ( n43 , n27 , n64 );
    nor g20 ( n64 , n42 , n8 );
    xnor g21 ( n35 , n41 , n68 );
    not g22 ( n52 , n33 );
    or g23 ( n29 , n9 , n24 );
    or g24 ( n21 , n15 , n44 );
    nor g25 ( n40 , n30 , n52 );
    and g26 ( n16 , n10 , n7 );
    not g27 ( n53 , n4 );
    and g28 ( n45 , n43 , n46 );
    xnor g29 ( n24 , n56 , n12 );
    not g30 ( n15 , n55 );
    or g31 ( n26 , n3 , n2 );
    not g32 ( n19 , n60 );
    and g33 ( n27 , n23 , n52 );
    nor g34 ( n30 , n4 , n39 );
    not g35 ( n37 , n63 );
    xnor g36 ( n48 , n62 , n28 );
    not g37 ( n36 , n38 );
    and g38 ( n13 , n1 , n56 );
    not g39 ( n39 , n31 );
    xnor g40 ( n18 , n6 , n48 );
    or g41 ( n8 , n16 , n5 );
    nor g42 ( n67 , n17 , n49 );
    nor g43 ( n17 , n1 , n56 );
    xnor g44 ( n20 , n61 , n65 );
    and g45 ( n5 , n54 , n11 );
    and g46 ( n0 , n65 , n61 );
    xnor g47 ( n14 , n43 , n20 );
    xnor g48 ( n41 , n22 , n66 );
    not g49 ( n10 , n54 );
    xnor g50 ( n61 , n5 , n24 );
    or g51 ( n1 , n51 , n36 );
    and g52 ( n54 , n50 , n38 );
    xnor g53 ( n59 , n35 , n18 );
    xnor g54 ( n6 , n67 , n29 );
    not g55 ( n3 , n50 );
    xnor g56 ( n34 , n33 , n57 );
endmodule
