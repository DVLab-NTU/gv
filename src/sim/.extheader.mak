sim.d: ../../include/gvSimCmd.h 
../../include/gvSimCmd.h: gvSimCmd.h
	@rm -f ../../include/gvSimCmd.h
	@ln -fs ../src/sim/gvSimCmd.h ../../include/gvSimCmd.h
