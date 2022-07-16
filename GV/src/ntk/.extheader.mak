ntk.d: ../../include/gvNtkCmd.h 
../../include/gvNtkCmd.h: gvNtkCmd.h
	@rm -f ../../include/gvNtkCmd.h
	@ln -fs ../src/ntk/gvNtkCmd.h ../../include/gvNtkCmd.h
