#ifndef GV_AIG_MGR_C
#define GV_AIG_MGR_C

#include "gvAigMgr.h"
#include "aig/aig/aig.h"
#include "proof/fraig/fraig.h"
#include "rnGen.h"
#include <iomanip>
#include <iostream>

typedef struct Fraig_ParamsStruct_t_ Fraig_Params_t;

extern void bmGateWay(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2, int p_equivalence);

extern "C"
{
    Aig_Man_t* Abc_NtkToDar(Abc_Ntk_t* pNtk, int fExors, int fRegisters);
    int        Aig_ManCutCount(Aig_ManCut_t* p, int* pnCutsK);
}

void
abcAigMgr::init() {
    int        i;
    Aig_Obj_t* pObj;
    pMan = Abc_NtkToDar(pNtk, 0, 1);

    Total_num = Aig_ManObjNum(pMan);
    PI_num    = Aig_ManCiNum(pMan);
    PO_num    = Aig_ManCoNum(pMan);
    Node_num  = Aig_ManAndNum(pMan);

    PI_List = new Aig_Obj_t*[PI_num];
    PO_List = new Aig_Obj_t*[PO_num];

    Aig_ManForEachCi(pMan, pObj, i) {
        PI_List[i] = pObj;
    }
    Aig_ManForEachCo(pMan, pObj, i) {
        PO_List[i] = pObj;
    }
}

void
abcAigMgr::printSummary(bool verbose) const {
    cout << endl;
    cout << "Circuit Statistics" << endl;
    cout << "==================" << endl;
    cout << "  PI  " << setw(10) << right << PI_num << endl;
    cout << "  PO  " << setw(10) << right << PO_num << endl;
    cout << "  AND " << setw(10) << right << Node_num << endl;
    cout << "------------------" << endl;
    cout << "  Total" << setw(10) << right << PI_num + PO_num + Node_num << endl;

    if (verbose) {
        Aig_Obj_t* pObj;
        int        i;
        Aig_ManForEachObj(pMan, pObj, i) {
            printf("Object Id = %d, Object Type = %d\n", Aig_ObjId(pObj), Aig_ObjType(pObj));
            Aig_Obj_t* pFanin;
            int        j;
            if (Aig_ObjType(pObj) == AIG_OBJ_CO) printf("  Fanin-0: Id = %d\n", Aig_ObjFaninId0(pObj));
            else if (Aig_ObjType(pObj) == AIG_OBJ_AND) {
                printf("  Fanin-0: Id = %d\n", Aig_ObjFaninId0(pObj));
                printf("  Fanin-1: Id = %d\n", Aig_ObjFaninId1(pObj));
            }
        }
    }
}

void
abcAigMgr::printPIs() const {
    cout << "PIs of the AIG:";
    for (int i = 0; i < PI_num; ++i) {
        std::cout << " " << Aig_ObjId(PI_List[i]);
    }
    cout << endl;
}

void
abcAigMgr::printPOs() const {
    cout << "POs of the AIG:";
    for (int i = 0; i < PO_num; ++i) {
        std::cout << " " << Aig_ObjId(PO_List[i]);
    }
    cout << endl;
}

void
abcAigMgr::Aig_AllDFS() {
    int        i;
    Aig_Obj_t* pObj;
    DFS_List          = new Aig_Obj_t*[Total_num];
    Vec_Ptr_t* vNodes = Vec_PtrAlloc(Aig_ManObjNum(pMan));
    vNodes            = Aig_ManDfsAll(pMan);
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i) {
        DFS_List[i] = pObj;
    }
    Vec_PtrFree(vNodes);
    // for(i = 0; i < Total_num; ++i)
    //     cout << Aig_ObjId(DFS_List[i]) << " ";
    // cout << endl;
    cout << "Successfully construct DFS_List!" << endl;
}

Aig_Obj_t**
abcAigMgr::Aig_NodesDFS(Aig_Obj_t** pNodes, int nNodes) const {
    int        i;
    Aig_Obj_t* pObj;
    Vec_Ptr_t* vNodes   = Vec_PtrAlloc(Aig_ManObjNum(pMan));
    vNodes              = Aig_ManDfsNodes(pMan, pNodes, nNodes);
    Aig_Obj_t** dfsList = new Aig_Obj_t*[Vec_PtrSize(vNodes)];
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i) {
        dfsList[i] = pObj;
    }
    Vec_PtrFree(vNodes);
    return dfsList;
}

void
abcAigMgr::fraig() {

    Fraig_Params_t Params, *pParams = &Params;
    Abc_Ntk_t*     pNtkRes;
    int            fAllNodes;
    int            fExdc;
    int            c;
    int            fPartition = 0;
    fExdc                     = 0;
    fAllNodes                 = 0;
    memset(pParams, 0, sizeof(Fraig_Params_t));
    pParams->nPatsRand  = 2048; // the number of words of random simulation info
    pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
    pParams->nBTLimit   = 100;  // the max number of backtracks to perform
    pParams->fFuncRed   = 1;    // performs only one level hashing
    pParams->fFeedBack  = 1;    // enables solver feedback
    pParams->fDist1Pats = 1;    // enables distance-1 patterns
    pParams->fDoSparse  = 1;    // performs equiv tests for sparse functions
    pParams->fChoicing  = 0;    // enables recording structural choices
    pParams->fTryProve  = 0;    // tries to solve the final miter
    pParams->fVerbose   = 0;    // the verbosiness flag
    pParams->fVerboseP  = 0;    // the verbosiness flag

    if (!Abc_NtkIsLogic(pNtk) && !Abc_NtkIsStrash(pNtk)) {
        cout << "Can only fraig a logic network or an AIG.\n" << endl;
        return;
    }

    // report the proof
    pParams->fVerboseP = pParams->fTryProve;
    if (Abc_NtkIsStrash(pNtk)) pNtkRes = Abc_NtkFraig(pNtk, &Params, fAllNodes, fExdc);
    else {
        pNtk    = Abc_NtkStrash(pNtk, fAllNodes, !fAllNodes, 0);
        pNtkRes = Abc_NtkFraig(pNtk, &Params, fAllNodes, fExdc);
        Abc_NtkDelete(pNtk);
    }
    if (pNtkRes == NULL) {
        cout << "Fraiging has failed.\n" << endl;
        return;
    }
    Abc_FrameReplaceCurrentNetwork(pAbc, pNtkRes);
    cout << "Fraiging has finished.\n" << endl;
}

void
abcAigMgr::randomSim(bool verbose) {
    int        i;
    Aig_Obj_t* pObj;

    RandomNumGen rnGen(0);
    Vec_Ptr_t*   vNodes = Vec_PtrAlloc(Total_num); // create a vector of the size of aigobj
    vNodes              = Aig_ManDfsAll(pMan);     // get all the nodes in aig
    size_t* simValue    = new size_t[Total_num](); // create an array to record the result of simulation

    // initialize sim values to be 0
    for (unsigned i = 0; i < Total_num; ++i) {
        simValue[i] = 0;
    }

    unsigned pos = 64, same = 0;
    size_t*  sim_64 = new size_t[PI_num]();

    for (unsigned i = 0; i < pos; ++i) {
        for (unsigned j = 0; j < PI_num; ++j) {
            sim_64[j] = ((size_t)rnGen(2) << i) | sim_64[j];
        }
    }

    Aig_ManForEachCi(this->pMan, pObj, i) {
        simValue[pObj->Id] = sim_64[i];
        sim_64[i]          = 0;
    }
    simTraversal(vNodes, simValue, verbose);
    if (verbose) printf("\n");
}

void
convertToBinary(size_t n) {
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
        if (n == 0) printf("0");
        else {
            printf("%d", n % 2);
            n /= 2;
        }
    }
}

void
abcAigMgr::simTraversal(Vec_Ptr_t* vNodes, size_t* simValue, bool verbose) {
    int        i;
    size_t     fa0, fa1;
    Aig_Obj_t* pObj;
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i) {
        if (Aig_ObjIsAnd(pObj)) {
            fa0 = simValue[Aig_ObjFaninId0(pObj)];
            fa1 = simValue[Aig_ObjFaninId1(pObj)];
            if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
            if (Aig_ObjFaninC1(pObj)) fa1 = ~fa1;
            simValue[Aig_ObjId(pObj)] = fa0 & fa1;
        } else if (Aig_ObjIsCo(pObj)) {
            fa0 = simValue[Aig_ObjFaninId0(pObj)];
            if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
            simValue[Aig_ObjId(pObj)] = fa0;
        }
    }
    if (verbose) {
        cout << "Ci : " << endl;
        Aig_ManForEachCi(pMan, pObj, i) {
            cout << "Node " << pObj->Id << " : ";
            convertToBinary(simValue[pObj->Id]);
            cout << "\n";
        }
        cout << "=========================================" << endl << endl;

        cout << "Co : " << endl;
        Aig_ManForEachCo(pMan, pObj, i) {
            cout << "Node " << pObj->Id << " : ";
            convertToBinary(simValue[pObj->Id]);
            cout << "\n";
        }
        cout << "=========================================" << endl << endl;

        cout << "Other And Node : " << endl;
        Aig_ManForEachNode(pMan, pObj, i) {
            if (pObj->Type == AIG_OBJ_CI || pObj->Type == AIG_OBJ_CO) continue;
            cout << "Node " << pObj->Id << " : ";
            convertToBinary(simValue[pObj->Id]);
            cout << "\n";
        }
    }
}

void
abcAigMgr::rewireByFanins(Aig_Obj_t* pObj, Aig_Obj_t* pFan0, Aig_Obj_t* pFan1) {
    if (Aig_ObjId(pFan0) < Aig_ObjId(pFan1)) Aig_ObjConnect(pMan, pObj, pFan0, pFan1);
    else Aig_ObjConnect(pMan, pObj, pFan1, pFan0);
}

Aig_ManCut_t*
abcAigMgr::Aig_EnumerateCuts(Aig_Man_t* pAig, int nCutsMax, int nLeafMax, int fTruth, bool verbose) {
    // Aig_Man_t * pAig = Abc_NtkToDar(pNtk1, 0, 1);
    Aig_ManCut_t* pManCut;
    Aig_Obj_t*    pObj;
    Aig_Cut_t*    pCut;
    int           i, k;
    int*          pnCutsK;
    if (!Abc_NtkIsLogic(pNtk) && !Abc_NtkIsStrash(pNtk)) {
        cout << "Can only fraig a logic network or an AIG.\n" << endl;
        return pManCut;
    }
    pManCut = Aig_ComputeCuts(pAig, nCutsMax, nLeafMax, fTruth, 0);
    if (verbose) {
        Aig_ManForEachNode(pManCut->pAig, pObj, i) {
            Aig_ObjForEachCut(pManCut, pObj, pCut, k) {
                if (pCut->nFanins == 0) continue;
                cout << "cut size = " << Aig_CutLeaveNum(pCut) << endl;
            }
        }
    }
    cout << Aig_ManCutCount(pManCut, pnCutsK) << "cuts found" << endl;
    return pManCut;
}

void
aigRandomSim(Aig_Man_t* pMan, size_t* simValue) {
    int        i;
    Aig_Obj_t* pObj;
    abcAigMgr* tmpAigMgr;

    RandomNumGen rnGen(0);
    Vec_Ptr_t*   vNodes = Vec_PtrAlloc(Aig_ManObjNum(pMan)); // create a vector of the size of aigobj
    vNodes              = Aig_ManDfsAll(pMan);               // get all the nodes in aig

    // initialize sim values to be 0
    for (unsigned i = 0; i < Aig_ManObjNum(pMan); ++i) {
        simValue[i] = 0;
    }

    unsigned pos = 64, same = 0;
    size_t*  sim_64 = new size_t[Aig_ManCiNum(pMan)]();

    for (unsigned i = 0; i < pos; ++i) {
        for (unsigned j = 0; j < Aig_ManCiNum(pMan); ++j) {
            sim_64[j] = ((size_t)rnGen(2) << i) | sim_64[j];
        }
    }

    Aig_ManForEachCi(pMan, pObj, i) {
        simValue[pObj->Id] = sim_64[i];
        sim_64[i]          = 0;
    }
    tmpAigMgr->simTraversal(vNodes, simValue, 0);
}

void
abcAigMgr::set_similarity_lookup() {
    // new the dynamic array
    similarity_lookup = new size_t*[256];
    size_t tmp_i, tmp_j;
    for (size_t i = 0; i < 256; ++i) {
        similarity_lookup[i] = new size_t[256];
    }

    for (size_t i = 0; i < 256; ++i) {
        for (size_t j = 0; j < 256; ++j) {
            similarity_lookup[i][j] = 0;
            tmp_i                   = i;
            tmp_j                   = j;
            for (size_t k = 0; k < 8; ++k) {
                similarity_lookup[i][j] += (tmp_i % 2 == tmp_j % 2);
                tmp_i >>= 1;
                tmp_j >>= 1;
            }
        }
    }
}

void
abcAigMgr::delete_similarity_lookup() {
    // new the dynamic array
    size_t tmp_i, tmp_j;
    for (size_t i = 0; i < 256; ++i) {
        delete[] similarity_lookup[i];
    }
    delete[] similarity_lookup;
}

float
abcAigMgr::computeSimilarityOf2Nodes(size_t sim1, size_t sim2) {
    int count = 0, num_bit = sizeof(size_t) * 4;
    for (size_t i = 0; i < num_bit / 8; ++i) {
        count += similarity_lookup[sim1 % 256][sim2 % 256];
        sim1 >>= 8;
        sim2 >>= 8;
    }

    return float(count) / float(num_bit);
}

void
abcAigMgr::simlirarity(char* filename) {
    Abc_Ntk_t*    pNtk2;
    Aig_Man_t *   pAig1, *pAig2;
    Aig_ManCut_t *pManCut1, *pManCut2;
    Aig_Obj_t *   pObj1, *pObj2;
    Aig_Cut_t *   pCut1, *pCut2;
    size_t *      simValue1, *simValue2;
    int           i, j, k, l, m, n, num_candidate = 0, max_candidate = 10000;
    float         similarity = 0;
    bool          stop       = false;

    pCutPair = new cut_pair_t[max_candidate];

    set_similarity_lookup(); // setup simulation lookup table

    pNtk2 = Io_Read(filename, IO_FILE_AIGER, 0, 0);
    pAig1 = Abc_NtkToDar(pNtk, 0, 1);
    pAig2 = Abc_NtkToDar(pNtk2, 0, 1);

    //  cout << "creating arrays" << Aig_ManObjNum(pAig1) << " " << Aig_ManObjNum(pAig1) << " " <<endl;
    simValue1 = new size_t[Aig_ManObjNum(pAig1)]; // create an array to record the result of simulation
    simValue2 = new size_t[Aig_ManObjNum(pAig2)]; // create an array to record the result of simulation
    // cout << "arrays completed" << endl;

    // enumerate cuts for both Ntks
    cout << "enumerating cuts..." << endl;
    pManCut1 = Aig_EnumerateCuts(pAig1, 50, 10, 0, 0);
    pManCut2 = Aig_EnumerateCuts(pAig2, 50, 10, 0, 0);

    // do randomsim for cut similarity
    cout << "doing random simulation..." << endl;
    aigRandomSim(pAig1, simValue1);
    aigRandomSim(pAig2, simValue2);

    // find high similarity cuts
    cout << "computing similarity..." << endl;
    Aig_ManForEachNode(pAig1, pObj1, i) {
        Aig_ManForEachNode(pAig2, pObj2, j) {
            similarity = computeSimilarityOf2Nodes(simValue1[pObj1->Id], simValue2[pObj2->Id]);
            if (similarity > 0.85) {
                Aig_ObjForEachCut(pManCut1, pObj1, pCut1, k) {
                    if (pCut1->nFanins == 0) continue;
                    Aig_ObjForEachCut(pManCut2, pObj2, pCut2, k) {
                        if (pCut2->nFanins == 0 || pCut1->nFanins != pCut2->nFanins) continue;
                        pCutPair[num_candidate].first  = pCut1;
                        pCutPair[num_candidate].second = pCut2;
                        ++num_candidate;
                        if (num_candidate >= max_candidate) {
                            stop = true;
                            break;
                        }
                    }
                    if (stop) break;
                    // TODO bmGateWay();
                }
            }
            if (stop) break;
        }
        if (stop) break;
    }
    cout << num_candidate << " equivalent candidates found!!!" << endl;
    delete_similarity_lookup();
}

#endif