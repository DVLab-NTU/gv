module top( n0 , n3 , n4 , n10 , n11 , n15 , n21 , n33 , n35 , 
n36 , n37 , n39 , n41 , n61 , n66 , n70 , n71 , n73 , n79 , 
n80 );
    input n0 , n3 , n11 , n15 , n21 , n35 , n36 , n37 , n41 , 
n61 , n73 , n80 ;
    output n4 , n10 , n33 , n39 , n66 , n70 , n71 , n79 ;
    wire n1 , n2 , n5 , n6 , n7 , n8 , n9 , n12 , n13 , 
n14 , n16 , n17 , n18 , n19 , n20 , n22 , n23 , n24 , n25 , 
n26 , n27 , n28 , n29 , n30 , n31 , n32 , n34 , n38 , n40 , 
n42 , n43 , n44 , n45 , n46 , n47 , n48 , n49 , n50 , n51 , 
n52 , n53 , n54 , n55 , n56 , n57 , n58 , n59 , n60 , n62 , 
n63 , n64 , n65 , n67 , n68 , n69 , n72 , n74 , n75 , n76 , 
n77 , n78 , n81 , n82 , n83 ;
    not g0 ( n72 , n74 );
    nor g1 ( n76 , n3 , n21 );
    nor g2 ( n53 , n73 , n34 );
    and g3 ( n58 , n31 , n19 );
    not g4 ( n19 , n36 );
    and g5 ( n67 , n18 , n43 );
    or g6 ( n59 , n69 , n50 );
    xnor g7 ( n66 , n77 , n20 );
    or g8 ( n32 , n13 , n28 );
    and g9 ( n60 , n66 , n70 );
    nand g10 ( n10 , n33 , n71 );
    not g11 ( n47 , n15 );
    or g12 ( n29 , n65 , n24 );
    not g13 ( n48 , n80 );
    or g14 ( n16 , n46 , n42 );
    or g15 ( n44 , n72 , n55 );
    or g16 ( n22 , n68 , n53 );
    nor g17 ( n64 , n0 , n21 );
    or g18 ( n43 , n78 , n48 );
    and g19 ( n9 , n39 , n60 );
    and g20 ( n79 , n4 , n9 );
    and g21 ( n17 , n59 , n58 );
    or g22 ( n81 , n64 , n63 );
    nor g23 ( n52 , n44 , n5 );
    nor g24 ( n40 , n72 , n58 );
    not g25 ( n83 , n11 );
    and g26 ( n46 , n67 , n47 );
    not g27 ( n6 , n3 );
    not g28 ( n69 , n37 );
    not g29 ( n8 , n41 );
    nor g30 ( n68 , n61 , n21 );
    not g31 ( n55 , n59 );
    or g32 ( n74 , n19 , n22 );
    xnor g33 ( n39 , n16 , n40 );
    and g34 ( n49 , n59 , n75 );
    nor g35 ( n25 , n17 , n52 );
    or g36 ( n56 , n58 , n51 );
    or g37 ( n26 , n34 , n48 );
    not g38 ( n13 , n77 );
    not g39 ( n57 , n41 );
    or g40 ( n77 , n83 , n29 );
    not g41 ( n34 , n61 );
    nor g42 ( n5 , n46 , n82 );
    nor g43 ( n65 , n35 , n21 );
    nor g44 ( n24 , n73 , n45 );
    nor g45 ( n20 , n28 , n46 );
    and g46 ( n31 , n7 , n26 );
    and g47 ( n70 , n77 , n12 );
    and g48 ( n1 , n14 , n27 );
    or g49 ( n38 , n62 , n54 );
    or g50 ( n75 , n37 , n38 );
    and g51 ( n82 , n23 , n2 );
    nor g52 ( n63 , n73 , n78 );
    nor g53 ( n54 , n3 , n57 );
    not g54 ( n78 , n0 );
    or g55 ( n7 , n61 , n57 );
    nor g56 ( n30 , n73 , n6 );
    xnor g57 ( n4 , n49 , n56 );
    or g58 ( n27 , n45 , n48 );
    not g59 ( n28 , n23 );
    or g60 ( n23 , n47 , n81 );
    or g61 ( n50 , n76 , n30 );
    or g62 ( n14 , n35 , n8 );
    and g63 ( n51 , n16 , n74 );
    and g64 ( n2 , n1 , n83 );
    not g65 ( n45 , n35 );
    and g66 ( n33 , n25 , n75 );
    not g67 ( n42 , n32 );
    not g68 ( n12 , n2 );
    or g69 ( n71 , n32 , n44 );
    and g70 ( n62 , n80 , n3 );
    or g71 ( n18 , n0 , n8 );
endmodule
