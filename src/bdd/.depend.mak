bddCmd.o: bddCmd.cpp bddCmd.h ../../include/gvCmdMgr.h bddMgrV.h myHash.h \
 bddNodeV.h ../../include/gvMsg.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
bddMgrV.o: bddMgrV.cpp bddNodeV.h bddMgrV.h myHash.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
bddNodeV.o: bddNodeV.cpp bddNodeV.h bddMgrV.h myHash.h
