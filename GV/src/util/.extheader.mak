util.d: ../../include/gvMsg.h ../../include/gvUsage.h ../../include/gvIntType.h ../../include/util.h ../../include/rnGen.h 
../../include/gvMsg.h: gvMsg.h
	@rm -f ../../include/gvMsg.h
	@ln -fs ../src/util/gvMsg.h ../../include/gvMsg.h
../../include/gvUsage.h: gvUsage.h
	@rm -f ../../include/gvUsage.h
	@ln -fs ../src/util/gvUsage.h ../../include/gvUsage.h
../../include/gvIntType.h: gvIntType.h
	@rm -f ../../include/gvIntType.h
	@ln -fs ../src/util/gvIntType.h ../../include/gvIntType.h
../../include/util.h: util.h
	@rm -f ../../include/util.h
	@ln -fs ../src/util/util.h ../../include/util.h
../../include/rnGen.h: rnGen.h
	@rm -f ../../include/rnGen.h
	@ln -fs ../src/util/rnGen.h ../../include/rnGen.h
