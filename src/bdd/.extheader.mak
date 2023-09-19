bdd.d: ../../include/bddMgrV.h ../../include/bddNodeV.h ../../include/bddCmd.h ../../include/myHash.h 
../../include/bddMgrV.h: bddMgrV.h
	@rm -f ../../include/bddMgrV.h
	@ln -fs ../src/bdd/bddMgrV.h ../../include/bddMgrV.h
../../include/bddNodeV.h: bddNodeV.h
	@rm -f ../../include/bddNodeV.h
	@ln -fs ../src/bdd/bddNodeV.h ../../include/bddNodeV.h
../../include/bddCmd.h: bddCmd.h
	@rm -f ../../include/bddCmd.h
	@ln -fs ../src/bdd/bddCmd.h ../../include/bddCmd.h
../../include/myHash.h: myHash.h
	@rm -f ../../include/myHash.h
	@ln -fs ../src/bdd/myHash.h ../../include/myHash.h
