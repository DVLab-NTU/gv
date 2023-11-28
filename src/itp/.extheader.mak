itp.d: ../../include/gvSatMgr.h ../../include/gvSatCmd.h 
../../include/gvSatMgr.h: gvSatMgr.h
	@rm -f ../../include/gvSatMgr.h
	@ln -fs ../src/itp/gvSatMgr.h ../../include/gvSatMgr.h
../../include/gvSatCmd.h: gvSatCmd.h
	@rm -f ../../include/gvSatCmd.h
	@ln -fs ../src/itp/gvSatCmd.h ../../include/gvSatCmd.h
