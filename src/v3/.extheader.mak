v3.d: ../../include/gvV3Mgr.h 
../../include/gvV3Mgr.h: gvV3Mgr.h
	@rm -f ../../include/gvV3Mgr.h
	@ln -fs ../src/v3/gvV3Mgr.h ../../include/gvV3Mgr.h

