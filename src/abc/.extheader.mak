abc.d: ../../include/abcCmd.h ../../include/abcMgr.h 
../../include/abcCmd.h: abcCmd.h
	@rm -f ../../include/abcCmd.h
	@ln -fs ../src/abc/abcCmd.h ../../include/abcCmd.h
../../include/abcMgr.h: abcMgr.h
	@rm -f ../../include/abcMgr.h
	@ln -fs ../src/abc/abcMgr.h ../../include/abcMgr.h
