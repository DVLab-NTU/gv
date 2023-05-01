itp.d: ../../include/gvSat.h ../../include/gvSatMgr.h ../../include/gvSatCmd.h ../../include/VarOrderV.h 
../../include/gvSat.h: gvSat.h
	@rm -f ../../include/gvSat.h
	@ln -fs ../src/itp/gvSat.h ../../include/gvSat.h
../../include/gvSatMgr.h: gvSatMgr.h
	@rm -f ../../include/gvSatMgr.h
	@ln -fs ../src/itp/gvSatMgr.h ../../include/gvSatMgr.h
../../include/gvSatCmd.h: gvSatCmd.h
	@rm -f ../../include/gvSatCmd.h
	@ln -fs ../src/itp/gvSatCmd.h ../../include/gvSatCmd.h
../../include/VarOrderV.h: VarOrderV.h
	@rm -f ../../include/VarOrderV.h
	@ln -fs ../src/itp/VarOrderV.h ../../include/VarOrderV.h
