ntk.d: ../../include/gvNtkCmd.h ../../include/gvNtk.h ../../include/gvWordLevel.h 
../../include/gvNtkCmd.h: gvNtkCmd.h
	@rm -f ../../include/gvNtkCmd.h
	@ln -fs ../src/ntk/gvNtkCmd.h ../../include/gvNtkCmd.h
../../include/gvNtk.h: gvNtk.h
	@rm -f ../../include/gvNtk.h
	@ln -fs ../src/ntk/gvNtk.h ../../include/gvNtk.h
../../include/gvWordLevel.h: gvWordLevel.h
	@rm -f ../../include/gvWordLevel.h
	@ln -fs ../src/ntk/gvWordLevel.h ../../include/gvWordLevel.h
