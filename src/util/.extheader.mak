util.d: ../../include/gvMsg.h ../../include/gvUsage.h ../../include/gvIntType.h ../../include/myHashMap.h ../../include/myHashSet.h ../../include/util.h ../../include/rnGen.h ../../include/myUsage.h 
../../include/gvMsg.h: gvMsg.h
	@rm -f ../../include/gvMsg.h
	@ln -fs ../src/util/gvMsg.h ../../include/gvMsg.h
../../include/gvUsage.h: gvUsage.h
	@rm -f ../../include/gvUsage.h
	@ln -fs ../src/util/gvUsage.h ../../include/gvUsage.h
../../include/gvIntType.h: gvIntType.h
	@rm -f ../../include/gvIntType.h
	@ln -fs ../src/util/gvIntType.h ../../include/gvIntType.h
../../include/myHashMap.h: myHashMap.h
	@rm -f ../../include/myHashMap.h
	@ln -fs ../src/util/myHashMap.h ../../include/myHashMap.h
../../include/myHashSet.h: myHashSet.h
	@rm -f ../../include/myHashSet.h
	@ln -fs ../src/util/myHashSet.h ../../include/myHashSet.h
../../include/util.h: util.h
	@rm -f ../../include/util.h
	@ln -fs ../src/util/util.h ../../include/util.h
../../include/rnGen.h: rnGen.h
	@rm -f ../../include/rnGen.h
	@ln -fs ../src/util/rnGen.h ../../include/rnGen.h
../../include/myUsage.h: myUsage.h
	@rm -f ../../include/myUsage.h
	@ln -fs ../src/util/myUsage.h ../../include/myUsage.h
