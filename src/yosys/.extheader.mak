yosys.d: ../../include/gvYosysCmd.h ../../include/gvYosysMgr.h 
../../include/gvYosysCmd.h: gvYosysCmd.h
	@rm -f ../../include/gvYosysCmd.h
	@ln -fs ../src/yosys/gvYosysCmd.h ../../include/gvYosysCmd.h
../../include/gvYosysMgr.h: gvYosysMgr.h
	@rm -f ../../include/gvYosysMgr.h
	@ln -fs ../src/yosys/gvYosysMgr.h ../../include/gvYosysMgr.h
