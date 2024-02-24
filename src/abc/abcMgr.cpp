#include "abcMgr.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "abcExt.h"
#include "base/abc/abc.h"
#include "bdd/cudd/cudd.h"
#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "proof/pdr/pdr.h"
#include "sat/cnf/cnf.h"
#include "util.h"

AbcMgr *abcMgr;

AbcMgr::AbcMgr() : pGia(NULL) {
    init();
}

AbcMgr::~AbcMgr() {
    reset();
}

void AbcMgr::init() {
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
}

void AbcMgr::reset() {
    delete pAbc;
}

void AbcMgr::execCmd(char *cmd) {
    // calling abc's command
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), cmd);
}

static string gateName(const string &name, const int &bit) {
    return name + "[" + to_string(bit) + "]";
}

void AbcMgr::readVerilog(const ABCParam &opt) {
    char *pFileName  = opt.pFileName;
    char *pTopModule = opt.pTopModule;
    char *pDefines   = opt.pDefines;
    int fInvert      = opt.fInvert;
    int fTechMap     = opt.fTechMap;
    int fSkipStrash  = opt.fSkipStrash;
    int fLibInDir    = opt.fLibInDir;
    int fVerbose     = opt.fVerbose;
    pGia             = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines, fSkipStrash, fInvert, fTechMap, fLibInDir, fVerbose);
}

void AbcMgr::readAig(const ABCParam &opt) {
    char *pFileName = opt.pFileName;
    int fSkipStrash = opt.fSkipStrash;
    int fGiaSimple  = opt.fGiaSimple;
    int fCheck      = opt.fCheck;
    // int fCheck = 1;
    pGia = Gia_AigerRead(pFileName, fGiaSimple, fSkipStrash, fCheck);
}

void AbcMgr::buildAigName(map<unsigned, string> &id2Name) {
    ifstream mapFile;
    string buffer;
    string type;
    string name;
    int idx = 0, bit = 0;

    mapFile.open(".map.txt");
    assert(mapFile.is_open());

    while (mapFile >> type) {
        mapFile >> buffer;
        myStr2Int(buffer, idx);
        mapFile >> buffer;
        myStr2Int(buffer, bit);
        mapFile >> buffer;
        name = buffer;
        if (type == "input") id2Name[Gia_ObjId(pGia, Gia_ManPi(pGia, idx))] = gateName(name, bit);
        else if (type == "output") id2Name[Gia_ObjId(pGia, Gia_ManPo(pGia, idx))] = gateName(name, bit);
        else if (type == "latch") {
            Gia_Obj_t *ro = Gia_ObjRiToRo(pGia, Gia_ManRi(pGia, idx));
            Gia_Obj_t *ri = Gia_ManRi(pGia, idx);

            id2Name[Gia_ObjId(pGia, ro)] = gateName(name, bit);
            id2Name[Gia_ObjId(pGia, ri)] = gateName(name, bit) + "_ns";
        }
    }
}

void AbcMgr::travPreprocess() {
    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);

    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));
}

void AbcMgr::travAllObj(const FileType &fileType, map<unsigned, string> id2Name) {
    Gia_Obj_t *pObj, *pObjRi, *pObjRo;  // the obj element of gia
    size_t i, iPi = 0, iPpi = 0, iPo = 0, iRi = 0, iRo = 0;
    map<int, int> PPI2RO;

    Gia_ManForEachObj(pGia, pObj, i) {
        int gateId = Gia_ObjId(pGia, pObj);
        if (Gia_ObjIsPi(pGia, pObj)) {
            if (inputIsPi(gateId)) parseInput(iPi++, gateId);
            else parseRo(iRo++, gateId, AIGER);
        } else if (Gia_ObjIsPo(pGia, pObj)) {
            int in0Id     = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int inv       = Gia_ObjFaninC0(pObj);
            string poName = (id2Name.count(gateId)) ? id2Name[gateId] : to_string(gateId);
            parseOutput(iPo++, gateId, in0Id, inv, poName);
        } else if (Gia_ObjIsAnd(pObj)) {
            int in0Id  = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int in1Id  = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
            int in0Inv = Gia_ObjFaninC0(pObj);
            int in1Inv = Gia_ObjFaninC1(pObj);
            if (PPI2RO.count(in0Id)) in0Id = PPI2RO[in0Id];
            if (PPI2RO.count(in1Id)) in1Id = PPI2RO[in1Id];
            parseAig(gateId, in0Id, in0Inv, in1Id, in1Inv);
        } else if (Gia_ObjIsRo(pGia, pObj)) {
            if (fileType == VERILOG) {
                PPI2RO[gateId] = (iPpi < iRo) ? parseRo(iPpi, 0, VERILOG) : 0;
                iPpi++;
            } else if (fileType == AIGER) {
                parseRo(iRo++, gateId, AIGER);
            }
        } else if (Gia_ObjIsRi(pGia, pObj)) {
            int in0Id = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int inv   = Gia_ObjFaninC0(pObj);
            parseRi(iRi++, gateId, in0Id, inv);
        } else if (Gia_ObjIsConst0(pObj)) {
            parseConst0();
        } else {
            cout << "not defined gate type" << endl;
            assert(true);
        }
    }

    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        if (i == iRo) break;
        int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
        if (fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia, pObjRo)];
        else if (fileType == AIGER) roGid = Gia_ObjId(pGia, pObjRo);
        parseRiRo(riGid, roGid);
    }
    parseConst1();
}

void AbcMgr::initCir(const FileType &fileType) {
    // Create lists
    int piNum = Gia_ManPiNum(pGia), regNum = Gia_ManRegNum(pGia), poNum = 0, totNum = 0;
    if (fileType == VERILOG) {
        piNum  = (Gia_ManRegNum(pGia)) ? piNum - regNum + 1 : piNum;
        regNum = ((Gia_ManRegNum(pGia) - 1) < 0) ? 0 : Gia_ManRegNum(pGia) - 1;
    } else if (fileType == AIGER) {
        piNum  = Gia_ManPiNum(pGia);
        regNum = (Gia_ManRegNum(pGia) < 0) ? 0 : Gia_ManRegNum(pGia);
    }
    poNum  = Gia_ManPoNum(pGia);
    totNum = Gia_ManObjNum(pGia);

    initCirMgr(piNum, poNum, regNum, totNum);
}

void AbcMgr::cirToAig(IDMap &aigIdMap) {
    Vec_Ptr_t *vNodes;
    Abc_Obj_t *pObj, *pNode0, *pNode1;
    Abc_Ntk_t *pNtkNew;
    int nTotal, nInputs, nOutputs, nLatches, nAnds;
    int nConstr = 0;
    int nDigits, i;
    char *pCur;
    unsigned uLit0, uLit1, uLit;
    int c0, c1;

    nInputs  = getNumPIs();
    nOutputs = getNumPOs();
    nLatches = getNumLATCHs();
    nAnds    = getNumAIGs();
    nTotal   = nInputs + nLatches + nAnds;

    // check the parameters
    if (nTotal != nInputs + nLatches + nAnds) {
        fprintf(stdout, "The number of objects does not match.\n");
    }

    // allocate the empty AIG
    pNtkNew           = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    pNtkNew->nConstrs = nConstr;
    // prepare the array of nodes
    vNodes          = Vec_PtrAlloc(1 + nInputs + nLatches + nAnds);
    Abc_Obj_t *zero = Abc_ObjNot(Abc_AigConst1(pNtkNew));
    Vec_PtrPush(vNodes, Abc_ObjNot(Abc_AigConst1(pNtkNew)));

    // create the PIs
    for (i = 0; i < nInputs; i++) {
        pObj = Abc_NtkCreatePi(pNtkNew);
        Vec_PtrPush(vNodes, pObj);
    }
    // create the POs
    for (i = 0; i < nOutputs; i++) {
        pObj = Abc_NtkCreatePo(pNtkNew);
    }
    // create the latches
    nDigits = Abc_Base10Log(nLatches);
    for (i = 0; i < nLatches; i++) {
        pObj = Abc_NtkCreateLatch(pNtkNew);
        Abc_LatchSetInit0(pObj);
        pNode0 = Abc_NtkCreateBi(pNtkNew);
        pNode1 = Abc_NtkCreateBo(pNtkNew);
        Abc_ObjAddFanin(pObj, pNode0);
        Abc_ObjAddFanin(pNode1, pObj);
        Vec_PtrPush(vNodes, pNode1);
    }
    // create the AND gates
    for (i = 0; i < nAnds; i++) {
        uLit0 = getAigIn0Gid(i);
        uLit1 = getAigIn1Gid(i);
        c0    = getAigIn0Cp(i);
        c1    = getAigIn1Cp(i);

        if (aigIdMap.count(uLit0)) uLit0 = aigIdMap[uLit0];
        if (aigIdMap.count(uLit1)) uLit1 = aigIdMap[uLit1];
        pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), c0);
        pNode1 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit1), c1);
        assert(Vec_PtrSize(vNodes) == i + 1 + nInputs + nLatches);
        Vec_PtrPush(vNodes, Abc_AigAnd((Abc_Aig_t *)pNtkNew->pManFunc, pNode0, pNode1));
    }
    // create the LATCH inputs
    Abc_NtkForEachLatchInput(pNtkNew, pObj, i) {
        uLit0 = getRiIn0Gid(i);
        c0    = getRiIn0Cp(i);
        if (aigIdMap.count(uLit0)) uLit0 = aigIdMap[uLit0];
        pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), (c0));
        Abc_ObjAddFanin(pObj, pNode0);
    }
    // read the PO driver literals
    Abc_NtkForEachPo(pNtkNew, pObj, i) {
        uLit0 = getPoIn0Gid(i);
        c0    = getPoIn0Cp(i);
        if (aigIdMap.count(uLit0)) uLit0 = aigIdMap[uLit0];
        pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), c0);  //^ (uLit0 < 2) );
        Abc_ObjAddFanin(pObj, pNode0);
    }

    // read the names if present
    Abc_NtkShortNames(pNtkNew);
    // skipping the comments
    // ABC_FREE(pContents);
    Vec_PtrFree(vNodes);
    // remove the extra nodes
    Abc_AigCleanup((Abc_Aig_t *)pNtkNew->pManFunc);
    // check the result
    int fCheck = 1;
    if (fCheck && !Abc_NtkCheckRead(pNtkNew)) {
        printf("Io_ReadAiger: The network check has failed.\n");
        Abc_NtkDelete(pNtkNew);
    }

    // save the new network
    pNtk = pNtkNew;
}

void AbcMgr::writeBlif(const string &fileName) {
    Abc_Ntk_t *pNtkTemp = Abc_NtkToNetlist(pNtk);
    char *pFileName     = new char[100];
    strcpy(pFileName, fileName.c_str());
    Io_WriteBlif(pNtkTemp, pFileName, 1, 0, 1);
}

void AbcMgr::runPDR(const bool &verbose) {
    // Start PDR
    Pdr_Par_t *pPars = new Pdr_Par_t();
    Pdr_ManSetDefaultParams(pPars);
    if (verbose) pPars->fVerbose = 1;
    pPars->fSolveAll = 1;
    pAbc->Status     = Abc_NtkDarPdr(pNtk, pPars);
}
