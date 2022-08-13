abc.d: ../../include/gvAbcCmd.h ../../include/gvAbcMgr.h ../../include/gvAbcNtk.h 
../../include/gvAbcCmd.h: gvAbcCmd.h
	@rm -f ../../include/gvAbcCmd.h
	@ln -fs ../src/abc/gvAbcCmd.h ../../include/gvAbcCmd.h
../../include/gvAbcMgr.h: gvAbcMgr.h
	@rm -f ../../include/gvAbcMgr.h
	@ln -fs ../src/abc/gvAbcMgr.h ../../include/gvAbcMgr.h
../../include/gvAbcNtk.h: gvAbcNtk.h
	@rm -f ../../include/gvAbcNtk.h
	@ln -fs ../src/abc/gvAbcNtk.h ../../include/gvAbcNtk.h
