gvAbcCmd.o: gvAbcCmd.cpp ../../include/gvMsg.h gvAbcCmd.h \
 ../../include/gvCmdMgr.h gvAbcMgr.h gvAbcNtk.h ../../include/util.h \
 ../../include/rnGen.h ../../include/myUsage.h
gvAbcNtk.o: gvAbcNtk.cpp gvAbcNtk.h
gvAbcMgr.o: gvAbcMgr.cpp gvAbcMgr.h gvAbcNtk.h
