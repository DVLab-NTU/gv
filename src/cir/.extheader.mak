cir.d: ../../include/cirCmd.h ../../include/cirDef.h ../../include/cirFraig.h ../../include/cirGate.h ../../include/cirMgr.h 
../../include/cirCmd.h: cirCmd.h
	@rm -f ../../include/cirCmd.h
	@ln -fs ../src/cir/cirCmd.h ../../include/cirCmd.h
../../include/cirDef.h: cirDef.h
	@rm -f ../../include/cirDef.h
	@ln -fs ../src/cir/cirDef.h ../../include/cirDef.h
../../include/cirFraig.h: cirFraig.h
	@rm -f ../../include/cirFraig.h
	@ln -fs ../src/cir/cirFraig.h ../../include/cirFraig.h
../../include/cirGate.h: cirGate.h
	@rm -f ../../include/cirGate.h
	@ln -fs ../src/cir/cirGate.h ../../include/cirGate.h
../../include/cirMgr.h: cirMgr.h
	@rm -f ../../include/cirMgr.h
	@ln -fs ../src/cir/cirMgr.h ../../include/cirMgr.h
