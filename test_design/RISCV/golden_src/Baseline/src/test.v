// [stage 1]
// psl default clock = (posedge clk);

// psl ERROR1_flush_stage1: assert never {(flush) && !(PC1_w == branchPC)};
// psl ERROR2_flush_stage1: assert never {(flush) && !(taken1_w == 1'b0)};
// psl ERROR3_flush_stage1: assert never {(flush) && !(instruction_out1_w == 32'h00000013)}; 
// psl ERROR4_flush_stage1: assert never {(flush) && !(PC_out1_w == 32'd0)};

// psl ERROR1_memory_stall_stage1: assert never {(memory_stall) && !(PC1_w == PC1_r)};
// psl ERROR2_memory_stall_stage1: assert never {(memory_stall) && !(taken1_w == taken1_r)};
// psl ERROR3_memory_stall_stage1: assert never {(memory_stall) && !(PC_out1_w == PC_out1_r)}; 
// psl ERROR4_memory_stall_stage1: assert never {(memory_stall) && !(instruction_out1_w == instruction_out1_r)};

// psl ERROR1_load_use_hazrd_stage1: assert never {(PC_write) && !(PC_1 == instructionPC_1 - 4)};


// [stage 2]
// psl default clock = (posedge clk);

// psl ERROR1_flush_stage2: assert never {(flush) && !(Rs1_w == 5'd0)};
// psl ERROR2_flush_stage2: assert never {(flush) && !(Rs2_w == 5'd0)};
// psl ERROR3_flush_stage2: assert never {(flush) && !(Rd2_w == 5'd0)}; 
// psl ERROR6_flush_stage2: assert never {(flush) && !(immediate_w == 32'd0)};
// psl ERROR4_flush_stage2: assert never {(flush) && !(data1_w == 32'd0)};
// psl ERROR5_flush_stage2: assert never {(flush) && !(data2_w == 32'd0)};
// psl ERROR7_flush_stage2: assert never {(flush) && !(WriteBack2_w == 1'b0)}; 
// psl ERROR8_flush_stage2: assert never {(flush) && !(PC2_w == 32'd0)};
// psl ERROR9_flush_stage2: assert never {(flush) && !(is_branchInst_w == 1'b0)};

// psl ERROR1_memory_stall_stage2: assert never {(memory_stall) && !(Rs1_w == Rs1_r)};
// psl ERROR2_memory_stall_stage2: assert never {(memory_stall) && !(Rs2_w == Rs2_r)};
// psl ERROR3_memory_stall_stage2: assert never {(memory_stall) && !(Rd2_w == Rd2_r)}; 
// psl ERROR4_memory_stall_stage2: assert never {(memory_stall) && !(immediate_w == immediate_r)};
// psl ERROR5_memory_stall_stage2: assert never {(memory_stall) && !(data1_w == data1_r)};
// psl ERROR6_memory_stall_stage2: assert never {(memory_stall) && !(data2_w == data2_r)};
// psl ERROR7_memory_stall_stage2: assert never {(memory_stall) && !(PC2_w == PC2_r)}; 
// psl ERROR8_memory_stall_stage2: assert never {(memory_stall) && !(is_branchInst_w == is_branchInst_r)};
// psl ERROR9_memory_stall_stage2: assert never {(memory_stall) && !(branch_type_w == branch_type_r)};
// psl ERROR10_memory_stall_stage2: assert never {(memory_stall) && !(Execution_w == Execution_r)};
// psl ERROR11_memory_stall_stage2: assert never {(memory_stall) && !(Mem2_w == Mem2_r)}; 
// psl ERROR12_memory_stall_stage2: assert never {(memory_stall) && !(WriteBack2_w == WriteBack2_r)};

// psl ERROR1_load_use_hazard_stage2: assert never {((Mem2_r[1]) && ((Rd2_r == Rs1_w) || (Rd2_r == Rs2_w))) && !(data_hazard == 1'b1)}; 
// psl ERROR2_load_use_hazard_stage2: assert never {!((Mem2_r[1]) && ((Rd2_r == Rs1_w) || (Rd2_r == Rs2_w))) && !(data_hazard != 1'b1)}; 

// psl ERROR1_forwarding_stage2: assert never {((!flush) && (!memory_stall) && (write_address) && (WriteBack_5)) && !(register2_w[write_address] == write_data)}; 

// psl ERROR1_control_unit_stage2: assert never {(instruction_1[6:5] == 2'b11) && !(is_branchInst_w == 1'b1)}; 
// psl ERROR2_control_unit_stage2: assert never {!(instruction_1[6:5] == 2'b11) && !(is_branchInst_w != 1'b1)}; 



// [stage 3]
// psl default clock = (posedge clk);

// psl ERROR1_memory_stall_stage3: assert never {(memory_stall) && !(ALU_result3_w == ALU_result3_r)};
// psl ERROR2_memory_stall_stage3: assert never {(memory_stall) && !(Rd3_w == Rd3_r)};
// psl ERROR3_memory_stall_stage3: assert never {(memory_stall) && !(WriteBack3_w == WriteBack3_r)}; 
// psl ERROR4_memory_stall_stage3: assert never {(memory_stall) && !(writedata3_w == writedata3_r)};
// psl ERROR5_memory_stall_stage3: assert never {(memory_stall) && !(Mem3_w == Mem3_r)};

// psl ERROR1_forwarding_stage3: assert never {((WriteBack3_r) && (Rd3_r != 0) && (Rd3_r == Rs1_2)) && !(forwardA != 2'b00)};  
// psl ERROR2_forwarding_stage3: assert never {((WriteBack3_r) && (Rd3_r != 0) && (Rd3_r == Rs2_2)) && !(forwardB != 2'b00)}; 
// psl ERROR3_forwarding_stage3: assert never {((WriteBack_5) && (Rd_5 != 0) && (Rd_5 == Rs1_2)) && !(forwardA != 2'b00)};     
// psl ERROR4_forwarding_stage3: assert never {((WriteBack_5) && (Rd_5 != 0) && (Rd_5 == Rs2_2)) && !(forwardB != 2'b00)};     



// [stage 4]
// psl default clock = (posedge clk);

// psl ERROR1_memory_stall_stage4: assert never {(memory_stall) && !(memory_result4_w == memory_result4_r)};
// psl ERROR2_memory_stall_stage4: assert never {(memory_stall) && !(ALU_result4_w == ALU_result4_r)};
// psl ERROR3_memory_stall_stage4: assert never {(memory_stall) && !(Rd4_w == Rd4_r)}; 
// psl ERROR4_memory_stall_stage4: assert never {(memory_stall) && !(WriteBack4_w == WriteBack4_r)};
// psl ERROR5_memory_stall_stage4: assert never {(memory_stall) && !(Mem2Reg4_w == Mem2Reg4_r)};