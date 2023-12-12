FileV.o: FileV.cpp FileV.h GlobalV.h
gvBitVec.o: gvBitVec.cpp gvBitVec.h ../../include/gvIntType.h
gvSat.o: gvSat.cpp gvSat.h SolverV.h ProofV.h SolverTypesV.h GlobalV.h \
 FileV.h VarOrderV.h HeapV.h ../../include/cirGate.h \
 ../../include/cirDef.h ../../include/myHash.h gvSat.h \
 ../../include/cirMgr.h ../../include/base/abc/abc.h \
 ../../include/misc/vec/vec.h ../../include/misc/util/abc_global.h \
 ../../include/misc/util/abc_namespaces.h ../../include/misc/vec/vecInt.h \
 ../../include/misc/vec/vecFlt.h ../../include/misc/vec/vecStr.h \
 ../../include/misc/vec/vecPtr.h ../../include/misc/vec/vecVec.h \
 ../../include/misc/vec/vecAtt.h ../../include/misc/vec/vecWrd.h \
 ../../include/misc/vec/vecBit.h ../../include/misc/vec/vecMem.h \
 ../../include/misc/vec/vecWec.h ../../include/aig/hop/hop.h \
 ../../include/aig/gia/gia.h ../../include/misc/vec/vecWec.h \
 ../../include/misc/util/utilCex.h ../../include/misc/st/st.h \
 ../../include/misc/st/stmm.h ../../include/misc/nm/nm.h \
 ../../include/misc/mem/mem.h ../../include/misc/extra/extra.h gvBitVec.h \
 ../../include/gvIntType.h
ProofV.o: ProofV.cpp ProofV.h SolverTypesV.h GlobalV.h FileV.h SortV.h
reader.o: reader.cpp reader.h
SolverV.o: SolverV.cpp SolverV.h ProofV.h SolverTypesV.h GlobalV.h \
 FileV.h VarOrderV.h HeapV.h SortV.h
