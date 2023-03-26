mod.d: ../../include/gvModMgr.h ../../include/gvModCmd.h 
../../include/gvModMgr.h: gvModMgr.h
	@rm -f ../../include/gvModMgr.h
	@ln -fs ../src/mod/gvModMgr.h ../../include/gvModMgr.h
../../include/gvModCmd.h: gvModCmd.h
	@rm -f ../../include/gvModCmd.h
	@ln -fs ../src/mod/gvModCmd.h ../../include/gvModCmd.h
