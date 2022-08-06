mod.d: ../../include/gvModMgr.h 
../../include/gvModMgr.h: gvModMgr.h
	@rm -f ../../include/gvModMgr.h
	@ln -fs ../src/mod/gvModMgr.h ../../include/gvModMgr.h
