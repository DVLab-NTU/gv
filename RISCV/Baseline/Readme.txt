src/
    Final_tb.v		testbench for project, use it when the pipelined datapath is debugged
    TestBed.v		expected write operation to data memory (DMEM) of final test

    slow_memory		same as HW4
    I_mem			Instruction memory. machine code , series and sort  ( with assembly description )
    D_mem			Data memory. initial data for slow memory
                        
    CHIP.v			Top module of your design, including RISCV_Pipeline, I_cache, and D_cache	
                    You cannot modify this module!!			