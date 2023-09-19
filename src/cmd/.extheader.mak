cmd.d: ../../include/gvCmdComm.h ../../include/gvCmdMgr.h 
../../include/gvCmdComm.h: gvCmdComm.h
	@rm -f ../../include/gvCmdComm.h
	@ln -fs ../src/cmd/gvCmdComm.h ../../include/gvCmdComm.h
../../include/gvCmdMgr.h: gvCmdMgr.h
	@rm -f ../../include/gvCmdMgr.h
	@ln -fs ../src/cmd/gvCmdMgr.h ../../include/gvCmdMgr.h
