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
    int fVerbose     = opt.fVerbose;
    pGia             = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines, fSkipStrash, fInvert, fTechMap, fVerbose);
}

void AbcMgr::readAig(const ABCParam &opt) {
    char *pFileName = opt.pFileName;
    int fSkipStrash = opt.fSkipStrash;
    int fGiaSimple  = opt.fGiaSimple;
    // int fCheck      = opt.fCheck;
    int fCheck = 1;
    pGia       = Gia_AigerRead(pFileName, fGiaSimple, fSkipStrash, fCheck);
    // Io_Read(pFileName, IO_FILE_AIGER, fCheck, fCheck);
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

void AbcMgr::travAllObj(const CirFileType &fileType, map<unsigned, string> id2Name) {
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
        // if (i == cirMgr->getNumLATCHs()) break;
        if (i == iRo) break;
        int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
        if (fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia, pObjRo)];
        else if (fileType == AIGER) roGid = Gia_ObjId(pGia, pObjRo);
        parseRiRo(riGid, roGid);
    }
    parseConst1();
}

void AbcMgr::initCir(const CirFileType &fileType) {
    // Create lists
    int piNum = 0, regNum = 0, poNum = 0, totNum = 0;
    if (fileType == VERILOG) {
        piNum  = Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1;
        regNum = ((Gia_ManRegNum(pGia) - 1) < 0) ? 0 : Gia_ManRegNum(pGia) - 1;
    } else if (fileType == AIGER) {
        piNum  = Gia_ManPiNum(pGia);
        regNum = (Gia_ManRegNum(pGia) < 0) ? 0 : Gia_ManRegNum(pGia);
    }
    poNum  = Gia_ManPoNum(pGia);
    totNum = Gia_ManObjNum(pGia);

    initCirMgr(piNum, poNum, regNum, totNum);
}

void AbcMgr::cirToGia() {
    Gia_Man_t *pNew, *pTemp;
    Vec_Ptr_t *vNamesIn = NULL, *vNamesOut = NULL, *vNamesRegIn = NULL, *vNamesRegOut = NULL, *vNamesNode = NULL;
    Vec_Int_t *vLits = NULL, *vPoTypes = NULL;
    Vec_Int_t *vNodes, *vDrivers, *vInits = NULL;
    int iObj, iNode0, iNode1, fHieOnly = 0;
    int nTotal, nInputs, nOutputs, nLatches, nAnds, i;
    int nBad = 0, nConstr = 0, nJust = 0, nFair = 0;
    unsigned char *pDrivers, *pSymbols, *pCur;
    unsigned uLit0, uLit1, uLit;

    nInputs  = cirMgr->_numDecl[PI_GATE];
    nOutputs = cirMgr->_numDecl[PO_GATE];
    nLatches = cirMgr->_numDecl[RO_GATE];
    nAnds    = cirMgr->_numDecl[AIG_GATE];
    nTotal   = nInputs + nLatches + nAnds;
    // Pdr_Par_t *pPars;
    // Pdr_ManSolve(pNew, pPars);
    // read the parameters (M I L O A + B C J F)
    // check the parameters

    if (nTotal != nInputs + nLatches + nAnds) {
        fprintf(stdout, "The number of objects does not match.\n");
        // return NULL;
    }

    // allocate the empty AIG
    pNew             = Gia_ManStart(nTotal + nLatches + nOutputs + 1);
    pNew->nConstrs   = 0;
    pNew->fGiaSimple = 0;

    // prototype by chiu
    nTotal = cirMgr->getNumTots();
    // end

    // prepare the array of nodes
    vNodes = Vec_IntAlloc(1 + nTotal);
    // By chiu
    Vec_IntPush(vNodes, 0);

    // create the PIs
    for (i = 0; i < nInputs + nLatches; i++) {
        iObj = Gia_ManAppendCi(pNew);
        Vec_IntPush(vNodes, iObj);
    }

    // remember the beginning of latch/PO literals
    // create the AND gates
    // if (!fGiaSimple && !fSkipStrash)
    Gia_ManHashAlloc(pNew);

    for (i = 0; i < nAnds; i++) {
        // uLit  = ((i + 1 + nInputs + nLatches) << 1);
        // uLit1 = uLit - Gia_AigerReadUnsigned(&pCur);
        // uLit0 = uLit1 - Gia_AigerReadUnsigned(&pCur);
        //        assert( uLit1 > uLit0 );
        // prototype by chiu
        uLit   = cirMgr->getAig(i)->getGid() << 1;
        uLit0  = cirMgr->getAig(i)->getIn0Gate()->getGid();
        uLit1  = cirMgr->getAig(i)->getIn1Gate()->getGid();
        int c0 = cirMgr->getAig(i)->getIn0().isInv();
        int c1 = cirMgr->getAig(i)->getIn1().isInv();
        // end

        // iNode0 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit0 >> 1), uLit0 & 1);
        // iNode1 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit1 >> 1), uLit1 & 1);
        cout << "[LOG]" << Vec_IntEntry(vNodes, uLit0) << endl;
        cout << "[LOG]" << Vec_IntEntry(vNodes, uLit1) << endl;

        iNode0 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit0), c0);
        iNode1 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit1), c1);
        // assert(Vec_IntSize(vNodes) == i + 1 + nInputs + nLatches);
        // if (!fGiaSimple && fSkipStrash) {
        //     if (iNode0 == iNode1)
        //         Vec_IntPush(vNodes, Gia_ManAppendBuf(pNew, iNode0));
        //     else
        //         Vec_IntPush(vNodes, Gia_ManAppendAnd(pNew, iNode0, iNode1));
        // } else
        Vec_IntPush(vNodes, Gia_ManHashAnd(pNew, iNode0, iNode1));
        // By chiu
        for (int i = 0; i < Vec_IntSize(vNodes); ++i) {
            cout << Vec_IntGetEntry(vNodes, i) << endl;
        }
        // end
    }
    // if (!fGiaSimple && !fSkipStrash)
    Gia_ManHashStop(pNew);

    // // remember the place where symbols begin
    // pSymbols = pCur;

    // read the latch driver literals
    vDrivers = Vec_IntAlloc(nLatches + nOutputs);
    // if (pContents[3] == ' ')  // standard AIGER
    if (true)  // standard AIGER
    {
        vInits = Vec_IntAlloc(nLatches);
        pCur   = pDrivers;
        for (i = 0; i < nLatches; i++) {
            uLit0 = atoi((char *)pCur);
            while (*pCur != ' ' && *pCur != '\n')
                pCur++;
            if (*pCur == ' ') {
                pCur++;
                Vec_IntPush(vInits, atoi((char *)pCur));
                while (*pCur++ != '\n')
                    ;
            } else {
                pCur++;
                Vec_IntPush(vInits, 0);
            }
            iNode0 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit0 >> 1), (uLit0 & 1));
            Vec_IntPush(vDrivers, iNode0);
        }
        // read the PO driver literals
        for (i = 0; i < nOutputs; i++) {
            // uLit0 = atoi((char *)pCur);
            uLit0 = cirMgr->getPo(i)->getGid();
            // while (*pCur++ != '\n');
            iNode0 = Abc_LitNotCond(Vec_IntEntry(vNodes, uLit0 >> 1), (uLit0 & 1));
            Vec_IntPush(vDrivers, iNode0);
        }
    }

    // create the POs
    for (i = 0; i < nOutputs; i++)
        Gia_ManAppendCo(pNew, Vec_IntEntry(vDrivers, nLatches + i));
    for (i = 0; i < nLatches; i++)
        Gia_ManAppendCo(pNew, Vec_IntEntry(vDrivers, i));
    Vec_IntFree(vDrivers);

    // // create the latches
    // Gia_ManSetRegNum(pNew, nLatches);

    // // skipping the comments
    // Vec_IntFree(vNodes);

    // // clean the PO drivers
    // if (vPoTypes) {
    //     pNew = Gia_ManDupWithConstraints(pTemp = pNew, vPoTypes);
    //     Gia_ManStop(pTemp);
    //     Vec_IntFreeP(&vPoTypes);
    // }

    // Vec_FltFreeP(&pNew->vInArrs);
    // Vec_FltFreeP(&pNew->vOutReqs);
    // /*
    //     // check the result
    //     if ( fCheck && !Gia_ManCheck( pNew ) )
    //     {
    //         printf( "Gia_AigerRead: The network check has failed.\n" );
    //         Gia_ManStop( pNew );
    //         return NULL;
    //     }
    // */

    // if (vInits && Vec_IntSum(vInits)) {
    //     char *pInit = ABC_ALLOC(char, Vec_IntSize(vInits) + 1);
    //     Gia_Obj_t *pObj;
    //     int i;
    //     assert(Vec_IntSize(vInits) == Gia_ManRegNum(pNew));
    //     Gia_ManForEachRo(pNew, pObj, i) {
    //         if (Vec_IntEntry(vInits, i) == 0)
    //             pInit[i] = '0';
    //         else if (Vec_IntEntry(vInits, i) == 1)
    //             pInit[i] = '1';
    //         else {
    //             assert(Vec_IntEntry(vInits, i) == Abc_Var2Lit(Gia_ObjId(pNew, pObj), 0));
    //             // unitialized value of the latch is the latch literal according to http://fmv.jku.at/hwmcc11/beyond1.pdf
    //             pInit[i] = 'X';
    //         }
    //     }
    //     pInit[i] = 0;
    //     // if (!fSkipStrash) {
    //     if (true) {
    //         // pNew            = Gia_ManDupZeroUndc(pTemp = pNew, pInit, 0, fGiaSimple, 1);
    //         pNew            = Gia_ManDupZeroUndc(pTemp = pNew, pInit, 0, 0, 1);
    //         pNew->nConstrs  = pTemp->nConstrs;
    //         pTemp->nConstrs = 0;
    //         Gia_ManStop(pTemp);
    //     }
    //     ABC_FREE(pInit);
    // }
    // Vec_IntFreeP(&vInits);

    // return pNew;
}

void AbcMgr::cirToAig(IDMap &aigIdDict) {
    // ProgressBar *pProgress;
    FILE *pFile;
    Vec_Ptr_t *vNodes, *vTerms;
    Vec_Int_t *vLits = NULL;
    Abc_Obj_t *pObj, *pNode0, *pNode1;
    Abc_Ntk_t *pNtkNew;
    int nTotal, nInputs, nOutputs, nLatches, nAnds;
    int nBad = 0, nConstr = 0, nJust = 0, nFair = 0;
    int nFileSize = -1, iTerm, nDigits, i;
    char *pContents, *pDrivers = NULL, *pSymbols, *pCur, *pName, *pType;
    unsigned uLit0, uLit1, uLit;
    int RetValue;

    nInputs  = cirMgr->_numDecl[PI_GATE];
    nOutputs = cirMgr->_numDecl[PO_GATE];
    nLatches = cirMgr->_numDecl[RO_GATE];
    nAnds    = cirMgr->_numDecl[AIG_GATE];
    nTotal   = nInputs + nLatches + nAnds;

    // Pdr_Par_t *pPars;
    // check the parameters
    if (nTotal != nInputs + nLatches + nAnds) {
        fprintf(stdout, "The number of objects does not match.\n");
        ABC_FREE(pContents);
        // return NULL;
    }
    if (nConstr) {
        if (nConstr == 1)
            fprintf(stdout, "Warning: The last output is interpreted as a constraint.\n");
        else
            fprintf(stdout, "Warning: The last %d outputs are interpreted as constraints.\n", nConstr);
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
        // printf("Creating PI: %d \n", pObj->Id);
    }
    // printf(" ----- \n");
    // create the POs
    for (i = 0; i < nOutputs; i++) {
        pObj = Abc_NtkCreatePo(pNtkNew);
        // printf("Creating PO: %d \n", pObj->Id);
    }
    // printf(" ----- \n");
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

        // printf("Creating Latch pObj: %d \n", pObj->Id);
        // printf("Creating Latch pNode0: %d \n", pNode0->Id);
        // printf("Creating Latch pNode1: %d \n", pNode1->Id);
        // printf("\n");
        // assign names to latch and its input
        //        Abc_ObjAssignName( pObj, Abc_ObjNameDummy("_L", i, nDigits), NULL );
        //        printf( "Creating latch %s with input %d and output %d.\n", Abc_ObjName(pObj), pNode0->Id, pNode1->Id );
    }

    // create the AND gates
    // pProgress = Extra_ProgressBarStart(stdout, nAnds);
    printf(" ----- \n");
    for (i = 0; i < nAnds; i++) {
        // Extra_ProgressBarUpdate(pProgress, i, NULL);
        // uLit  = ((i + 1 + nInputs + nLatches) << 1);
        // uLit1 = uLit - Io_ReadAigerDecode(&pCur);
        // uLit0 = uLit1 - Io_ReadAigerDecode(&pCur);

        // prototype by chiu
        uLit  = cirMgr->getAig(i)->getGid() << 1;
        uLit0 = cirMgr->getAig(i)->getIn0Gate()->getGid();
        uLit1 = cirMgr->getAig(i)->getIn1Gate()->getGid();
        if (aigIdDict.count(uLit0))
            uLit0 = aigIdDict[uLit0];
        if (aigIdDict.count(uLit1))
            uLit1 = aigIdDict[uLit1];

        int c0 = cirMgr->getAig(i)->getIn0().isInv();
        int c1 = cirMgr->getAig(i)->getIn1().isInv();
        // end

        //        assert( uLit1 > uLit0 );
        // pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0 >> 1), uLit0 & 1);
        // pNode1 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit1 >> 1), uLit1 & 1);
        pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), c0);
        pNode1 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit1), c1);
        assert(Vec_PtrSize(vNodes) == i + 1 + nInputs + nLatches);
        Vec_PtrPush(vNodes, Abc_AigAnd((Abc_Aig_t *)pNtkNew->pManFunc, pNode0, pNode1));

        // printf("Creating Aig uLit0: %d \n", uLit0);
        // printf("Creating Aig uLit1: %d \n", uLit1);
        // printf("Creating Aig pNode0: %d \n", pNode0->Id);
        // printf("Creating Aig pNode1: %d \n", pNode1->Id);
        // printf("\n");
    }

    int offset = 0;
    if (true)  // standard AIGER
    {
        Abc_NtkForEachLatchInput(pNtkNew, pObj, i) {
            // uLit0 = atoi(pCur);
            // if (i + offset == 45 || i + offset == 85)
            //     offset++;
            uLit0 = cirMgr->getRi(i + offset)->getIn0Gate()->getGid();
            if (aigIdDict.count(uLit0))
                uLit0 = aigIdDict[uLit0];
            // while (*pCur != ' ' && *pCur != '\n') pCur++;
            // if (*pCur == ' ')  // read initial value
            if (true)  // read initial value
            // if (false)  // read initial value
            {
                int Init = -1;
                // int Init = 0;
                // pCur++;
                // Init = atoi(pCur);
                if (Init == 0)
                    Abc_LatchSetInit0(Abc_NtkBox(pNtkNew, i));
                else if (Init == 1)
                    Abc_LatchSetInit1(Abc_NtkBox(pNtkNew, i));
                else {
                    // assert(Init == Abc_Var2Lit(1 + Abc_NtkPiNum(pNtkNew) + i, 0));
                    // unitialized value of the latch is the latch literal according to http://fmv.jku.at/hwmcc11/beyond1.pdf
                    Abc_LatchSetInitDc(Abc_NtkBox(pNtkNew, i));
                }
            }
            // pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0 >> 1), (uLit0 & 1));  //^ (uLit0 < 2) );
            int c0 = cirMgr->getRi(i + offset)->getIn0().isInv();
            pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), (c0));
            Abc_ObjAddFanin(pObj, pNode0);
        }
        // read the PO driver literals
        Abc_NtkForEachPo(pNtkNew, pObj, i) {
            // uLit0 = atoi(pCur);
            // while (*pCur++ != '\n');
            uLit0  = cirMgr->getPo(i)->getIn0Gate()->getGid();
            int c0 = cirMgr->getPo(i)->getIn0().isInv();
            if (aigIdDict.count(uLit0)) uLit0 = aigIdDict[uLit0];
            // if (uLit0 >= 3) uLit0 -= 3;

            // pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0 >> 1), (uLit0 & 1));  //^ (uLit0 < 2) );
            pNode0 = Abc_ObjNotCond((Abc_Obj_t *)Vec_PtrEntry(vNodes, uLit0), c0);  //^ (uLit0 < 2) );
            Abc_ObjAddFanin(pObj, pNode0);
        }
    }

    // read the names if present
    if (true) {
        Abc_NtkShortNames(pNtkNew);
    }

    // skipping the comments
    // ABC_FREE(pContents);
    Vec_PtrFree(vNodes);

    // remove the extra nodes
    Abc_AigCleanup((Abc_Aig_t *)pNtkNew->pManFunc);

    // // update polarity of the additional outputs
    // if (nBad || nConstr || nJust || nFair)
    //     Abc_NtkInvertConstraints(pNtkNew);

    // check the result
    int fCheck = 1;
    if (fCheck && !Abc_NtkCheckRead(pNtkNew)) {
        printf("Io_ReadAiger: The network check has failed.\n");
        Abc_NtkDelete(pNtkNew);
        // return NULL;
    }

    // Start PDR
    pNtk             = pNtkNew;
    Pdr_Par_t *pPars = new Pdr_Par_t();
    Pdr_ManSetDefaultParams(pPars);
    pPars->fVerbose  = 1;
    pPars->fSolveAll = 1;
    pAbc->Status     = Abc_NtkDarPdr(pNtk, pPars);

    // return pNtkNew;
}
