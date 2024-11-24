module top( n1 , n4 , n10 , n13 , n26 , n27 , n32 , n34 , n46 , 
n52 , n54 , n58 , n61 , n64 , n65 , n66 );
    input n1 , n4 , n10 , n13 , n27 , n32 , n46 , n58 , n61 , 
n64 , n65 , n66 ;
    output n26 , n34 , n52 , n54 ;
    wire n0 , n2 , n3 , n5 , n6 , n7 , n8 , n9 , n11 , 
n12 , n14 , n15 , n16 , n17 , n18 , n19 , n20 , n21 , n22 , 
n23 , n24 , n25 , n28 , n29 , n30 , n31 , n33 , n35 , n36 , 
n37 , n38 , n39 , n40 , n41 , n42 , n43 , n44 , n45 , n47 , 
n48 , n49 , n50 , n51 , n53 , n55 , n56 , n57 , n59 , n60 , 
n62 , n63 , n67 , n68 , n69 , n70 ;
    not g0 ( n33 , n46 );
    and g1 ( n17 , n8 , n40 );
    xnor g2 ( n23 , n14 , n64 );
    nor g3 ( n47 , n17 , n49 );
    or g4 ( n29 , n61 , n33 );
    nor g5 ( n69 , n19 , n36 );
    xnor g6 ( n34 , n49 , n0 );
    nor g7 ( n18 , n40 , n8 );
    not g8 ( n7 , n58 );
    xnor g9 ( n16 , n20 , n1 );
    not g10 ( n37 , n70 );
    xnor g11 ( n8 , n70 , n44 );
    not g12 ( n53 , n24 );
    not g13 ( n40 , n27 );
    nor g14 ( n14 , n18 , n47 );
    and g15 ( n24 , n41 , n2 );
    nor g16 ( n12 , n48 , n69 );
    xnor g17 ( n35 , n29 , n63 );
    not g18 ( n60 , n55 );
    or g19 ( n63 , n66 , n7 );
    and g20 ( n55 , n67 , n11 );
    or g21 ( n68 , n11 , n67 );
    or g22 ( n22 , n1 , n60 );
    nor g23 ( n25 , n57 , n12 );
    not g24 ( n19 , n53 );
    nor g25 ( n51 , n13 , n66 );
    xnor g26 ( n39 , n25 , n50 );
    not g27 ( n41 , n13 );
    nor g28 ( n50 , n70 , n44 );
    xnor g29 ( n43 , n39 , n23 );
    xnor g30 ( n38 , n24 , n48 );
    or g31 ( n20 , n9 , n37 );
    or g32 ( n70 , n53 , n28 );
    not g33 ( n28 , n67 );
    not g34 ( n11 , n4 );
    not g35 ( n31 , n55 );
    xnor g36 ( n44 , n36 , n38 );
    nor g37 ( n9 , n56 , n51 );
    or g38 ( n15 , n42 , n20 );
    and g39 ( n67 , n62 , n3 );
    or g40 ( n21 , n13 , n30 );
    and g41 ( n49 , n15 , n22 );
    not g42 ( n30 , n10 );
    or g43 ( n48 , n66 , n5 );
    xnor g44 ( n52 , n59 , n43 );
    xnor g45 ( n0 , n8 , n27 );
    xnor g46 ( n45 , n6 , n21 );
    and g47 ( n54 , n60 , n68 );
    and g48 ( n57 , n36 , n19 );
    not g49 ( n5 , n65 );
    not g50 ( n62 , n61 );
    not g51 ( n3 , n66 );
    not g52 ( n2 , n32 );
    nor g53 ( n56 , n61 , n32 );
    xnor g54 ( n59 , n35 , n45 );
    xnor g55 ( n26 , n55 , n16 );
    and g56 ( n36 , n62 , n10 );
    and g57 ( n42 , n31 , n1 );
    nor g58 ( n6 , n32 , n5 );
endmodule
