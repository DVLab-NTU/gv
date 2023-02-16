bdd.d: ../../include/gvBddMgr.h ../../include/gvBddCmd.h ../../include/gvBddNode.h 
../../include/gvBddMgr.h: gvBddMgr.h
	@rm -f ../../include/gvBddMgr.h
	@ln -fs ../src/bdd/gvBddMgr.h ../../include/gvBddMgr.h
../../include/gvBddCmd.h: gvBddCmd.h
	@rm -f ../../include/gvBddCmd.h
	@ln -fs ../src/bdd/gvBddCmd.h ../../include/gvBddCmd.h
../../include/gvBddNode.h: gvBddNode.h
	@rm -f ../../include/gvBddNode.h
	@ln -fs ../src/bdd/gvBddNode.h ../../include/gvBddNode.h
