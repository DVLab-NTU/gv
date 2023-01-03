#ifndef GV_AIG_MGR_C
#define GV_AIG_MGR_C

#include "gvAigMgr.h"
#include "aig/aig/aig.h"
#include <stdlib.h>  
#include <time.h>
#include <math.h>
#include <algorithm>
#include "proof/fraig/fraig.h"
#include "rnGen.h"
#include <iomanip>
#include <iostream>
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "gvCnf.h"
using namespace std;

typedef struct Fraig_ParamsStruct_t_ Fraig_Params_t;

extern void bmGateWay(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2, int p_equivalence);

extern "C"
{
    Aig_Man_t* Abc_NtkToDar(Abc_Ntk_t* pNtk, int fExors, int fRegisters);
    int        Aig_ManCutCount(Aig_ManCut_t* p, int* pnCutsK);
    Aig_Obj_t * Aig_ManDup_rec( Aig_Man_t * pNew, Aig_Man_t * p, Aig_Obj_t * pObj );
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

void abcAigMgr::simTraversal2( Vec_Ptr_t* vNodes, size_t** simValue, size_t num_words, bool verbose)
{
	int j;
    size_t fa0, fa1;
    Aig_Obj_t* pObj;
    for(size_t i = 0; i < num_words; ++i) {
        Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, j) {
            if (Aig_ObjIsAnd(pObj)) {
                fa0 = simValue[Aig_ObjFaninId0(pObj)][i];
                fa1 = simValue[Aig_ObjFaninId1(pObj)][i];
                if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
                if (Aig_ObjFaninC1(pObj)) fa1 = ~fa1;
                simValue[Aig_ObjId(pObj)][i] = fa0 & fa1;
            }
            else if (Aig_ObjIsCo(pObj)) {
                fa0 = simValue[Aig_ObjFaninId0(pObj)][i];
                if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
                simValue[Aig_ObjId(pObj)][i] = fa0;
            }
        }
    }
    
}


void aigRandomSim2( Aig_Man_t *pMan1, Aig_Man_t *pMan2, size_t** simValue1, size_t** simValue2, size_t num_words) {
    assert(pMan1->nObjs[AIG_OBJ_CI] == pMan2->nObjs[AIG_OBJ_CI]); // the CI of two circuits should be matched
    int i, j;
    Aig_Obj_t* pObj;
    abcAigMgr* tmpAigMgr;
    
    RandomNumGen2  rnGen(3);
    Vec_Ptr_t* vNodes1 = Vec_PtrAlloc(Aig_ManObjNum(pMan1)); // create a vector of the size of aigobj
    Vec_Ptr_t* vNodes2 = Vec_PtrAlloc(Aig_ManObjNum(pMan2)); // create a vector of the size of aigobj
    vNodes1 = Aig_ManDfsAll(pMan1); // get all the nodes in aig
    vNodes2 = Aig_ManDfsAll(pMan2); // get all the nodes in aig
    

    // initialize sim values to be 0
    for (unsigned i = 0; i < Aig_ManObjNum(pMan1); ++i) {
        for (unsigned j = 0; j < Aig_ManObjNum(pMan1); ++j) {
            simValue1[i][j] = 0;
            simValue2[i][j] = 0;
        }
    }

    unsigned pos = 64;
    size_t** sim_64;
  	sim_64 = new size_t* [Aig_ManCiNum(pMan1)] ();
    for(unsigned i = 0; i < Aig_ManCiNum(pMan1); ++i) {
        sim_64[i] = new size_t [num_words] ();
    }
    random_vec_t random_vec;
    for (unsigned i = 0; i < Aig_ManCiNum(pMan1) ; ++i) {
        random_vec = rnGen(num_words);
        for(unsigned j = 0; j < num_words; ++j){
            sim_64[i][j] =  (random_vec[j]);
        }
    }

    for(unsigned i = 0; i < num_words; ++i) {
        Aig_ManForEachCi(pMan1, pObj, j)
        {
            simValue1[pObj->Id][i] =  sim_64[j][i];
        }
    }

    for(unsigned i = 0; i < num_words; ++i) {
        Aig_ManForEachCi(pMan2, pObj, j)
        {
            simValue2[pObj->Id][i] =  sim_64[j][i];
            sim_64[j][i] = 0;
        }
    }
    tmpAigMgr->simTraversal2(vNodes1, simValue1, num_words, 0);
    tmpAigMgr->simTraversal2(vNodes2, simValue2, num_words, 0);
}

void buildSimilarityLookup(size_t* lookupTable)
{
    // lookupTable = new size_t* [sizeof(size_t)];
    size_t tmp = 0;
    
    for(size_t i = 0; i < pow(2, sizeof(size_t) * 2); ++i)
    {
        lookupTable[i] = 0;
        tmp = i;
        for(size_t j = 0; j < sizeof(size_t) * 2; ++j)
        {
            if(tmp % 2 == 1)
            {
                ++lookupTable[i];
            }
            tmp >>= 1;
        }
    }
}

void convertToBinary2(size_t* n, size_t num_words)
{
    size_t tmp;
    for(size_t i = 0; i < num_words; ++i)
    {
        tmp = n[i];
        for(size_t j = 0; j < sizeof(size_t) * 8; ++j)
        {
            if(tmp == 0)
                printf("0");
            else{
                printf("%d", tmp % 2);
                tmp /= 2;
            }
        }
    }
}


// Create the aig cone
// This function is modified from Abc_NtkCreateCone
Aig_Man_t * Aig_ManCreateCone( Aig_Man_t * p, Aig_Obj_t * pNode)
{
    Aig_Man_t * pNew;
    Aig_Obj_t * pObj;
    int i;

    pNew = Aig_ManStart( Aig_ManObjNumMax(p) );
    pNew->pName = Abc_UtilStrsav( p->pName );
    pNew->pSpec = Abc_UtilStrsav( p->pSpec );

    // create the PIs
    Aig_ManCleanData( p );
    Aig_ManConst1(p)->pData = Aig_ManConst1(pNew);
    Aig_ManForEachCi( p, pObj, i )
        pObj->pData = Aig_ObjCreateCi(pNew);
    Aig_ManForEachNode( p, pObj, i )
        pObj->pData = Aig_And( pNew, Aig_ObjChild0Copy(pObj), Aig_ObjChild1Copy(pObj) );
    // add the PO
    Aig_ObjCreateCo( pNew, (Aig_Obj_t *)pNode->pData );
    Aig_ManCleanup( pNew );
    // check the resulting network
    if ( !Aig_ManCheck(pNew) )
        printf( "Aig_ManExtractMiter(): The check has failed.\n" );

    return pNew;
}

similarity_t computeSimilarityOf2Nodes(Aig_Man_t * pAig1, Aig_Man_t * pAig2, const size_t* sim1, const size_t* sim2, size_t** simValue1, size_t**  simValue2, const size_t * lookupTable, size_t num_words)
{
    size_t count = 0, num_bit = sizeof(size_t) * 8, cofactor_count = 0, good_vector_count = 0;
    similarity_t similarity;
    size_t equal, miter, good_vector;
    size_t mask[4];
    // int j;
    Aig_Obj_t * pObj1, *pObj2;
    // const size_t * sim1 = sim_1, * sim2 = sim_2, * output1 = output_1, * output2 = output_2;
    mask[0] = pow(2, sizeof(size_t) * 2) - 1;
    mask[1] = pow(2, sizeof(size_t) * 4) - pow(2, sizeof(size_t) * 2);
    mask[2] = pow(2, sizeof(size_t) * 6) - pow(2, sizeof(size_t) * 4);
    mask[3] = pow(2, sizeof(size_t) * 8) - pow(2, sizeof(size_t) * 6);
    
    for(size_t i = 0; i < num_words; ++i)
    {
        equal = ~(sim1[i] ^ sim2[i]);
        for(size_t j = 0; j < Aig_ManCoNum(pAig1); ++j)
        {
            
            miter = ~(simValue1[Aig_ManCo(pAig1, j)->Id][i] ^ simValue2[Aig_ManCo(pAig2, j)->Id][i]);
            good_vector = equal & miter;
            for(size_t k = 0; k < 4; ++k)
            {
                count += lookupTable[(good_vector & mask[k]) >> (k * sizeof(size_t) * 2)];
                cofactor_count += lookupTable[(equal & mask[k]) >> (k * sizeof(size_t) * 2)];
                good_vector_count += lookupTable[(miter & mask[k]) >> (k * sizeof(size_t) * 2)];
            }
        }
    }
    // cout << "] cofactor equal count = " << cofactor_count << " miter equal count  " << good_vector_count << endl;
    if(!cofactor_count)
        similarity.first = 0;
    else
        similarity.first = float(count) / float(cofactor_count);
    if(!good_vector_count)
        similarity.second = 0;
    else
        similarity.second = float(count) / float(good_vector_count);
    return similarity;
}

bool similarity_vec_compare(obj_similarity_t obj1, obj_similarity_t obj2)
{
    return obj1.second > obj2.second;
}

void vec_swap(vector<obj_similarity_t>& vec, int i, int j)
{
    obj_similarity_t tmp = vec[i];
    vec[i] = vec[j];
    vec[j] = tmp;

}

void abcAigMgr::simlirarity(char* filename) {
    Abc_Ntk_t *pNtk2;
    Aig_ManCut_t* pManCut1, *pManCut2;
    Aig_Obj_t * pObj1, *pObj2;
    Aig_Cut_t * pCut1, *pCut2;
    size_t ** simValue1, **simValue2;
    int i, j, k, status;
    similarity_t similarity;
    bool stop = false;
    size_t* lookupTable;
    size_t num_words = 1000; // number of words simulated
    clock_t time_start;
    obj_similarity_t tmp_obj_similarity;
    Aig_Man_t * pAigCone1, * pAigCone2, * pMiter;
    Cnf_Dat_t * pCnf;
    sat_solver * pSat = sat_solver_new();
    lit Lits[2]; // literals for temparary use
    int * pBeg, * pEnd;
    cnf_t Cnf;
    cnf_obj_t Cnf_obj = {Cnf, 0};
    vector<vector<int>> e;
    vector<int>         e_row, assump;


    lookupTable = new size_t[(size_t)pow(2, sizeof(size_t) * 2)];
   
    pNtk2 = Io_Read(filename, IO_FILE_VERILOG, 0, 0);
    pNtk2 = Abc_NtkStrash(pNtk2, 0, 1, 0);
    pAig1 = Abc_NtkToDar(pNtk, 0, 1);
    pAig2 = Abc_NtkToDar(pNtk2, 0, 1);
    
    // record the fanout information of the two aigs
    Aig_ManFanoutStart(pAig1);
    Aig_ManFanoutStart(pAig2);
    

    //  cout << "creating arrays" << Aig_ManObjNum(pAig1) << " " << Aig_ManObjNum(pAig1) << " " <<endl;
    similarity_table = new similarity_vec_t [Aig_ManObjNum(pAig1)];
    simValue1 = new size_t* [Aig_ManObjNum(pAig1)]; // create an array to record the result of simulation
    simValue2 = new size_t* [Aig_ManObjNum(pAig2)]; // create an array to record the result of simulation
    for(i = 0; i < Aig_ManObjNum(pAig1); ++i)
    {
        simValue1[i] = new size_t [num_words];
    }
    for(i = 0; i < Aig_ManObjNum(pAig2); ++i)
    {
        simValue2[i] = new size_t [num_words];
    }
    // cout << "arrays completed" << endl;

    // do randomsim for cut similarity
    cout << "doing random simulation with " << num_words << " words!" << endl;
    time_start = clock();
    aigRandomSim2(pAig1, pAig2, simValue1, simValue2, num_words);
    cout << "simulation is completed in " <<  float(clock() - time_start) / CLOCKS_PER_SEC << " seconds" << endl;

    // build the lookup table
    buildSimilarityLookup(lookupTable);



    // find high similarity cuts
    cout << "computing similarity..." << endl;
    time_start = clock();

    // Set the number of vars as #candidates in old circuit * #candidates in golden circuit
    sat_solver_setnvars(pSat, Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2)); // set the vars for eij
    Cnf_obj.cnf_data_lift(Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2));  // add the nuber of vars into the Cnf_obj

    // set the table of eij
    for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
    {
        e_row.clear(); // create each row for eij
        for(j = 0; j < Aig_ManCandNum(pAig2) - 1; ++j)
        {
            e_row.push_back(i * Aig_ManCandNum(pAig2) + j);
        }
        e.push_back(e_row); // push the row into the chart
    }
    // pSat->fPrintClause =  1; // for debug use
    // Add constraint 1 - Every row can only have one 1
    cout << "adding constraint1..." << endl;
    for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
    {
        for(j = 0; j < Aig_ManCandNum(pAig2) - 1; ++j)
        {
            Lits[0] = Abc_Var2Lit(i * Aig_ManCandNum(pAig2) + j, 1);
            for(k = j + 1; k < Aig_ManCandNum(pAig2); ++k)
            {
                Lits[1] = Abc_Var2Lit(i * Aig_ManCandNum(pAig2) + k, 1);
                sat_solver_addclause(pSat, Lits, Lits + 2);
            }
        }
    }

    // Add constraint 2 - Every column can only have one 1
    cout << "adding constraint2..." << endl;
    for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
    {
        for(j = 0; j < Aig_ManCandNum(pAig2) - 1; ++j)
        {
            Lits[0] = Abc_Var2Lit(i + Aig_ManCandNum(pAig2) * j, 1);
            for(k = j + 1; k < Aig_ManCandNum(pAig2); ++k)
            {
                Lits[1] = Abc_Var2Lit(i + Aig_ManCandNum(pAig2) * k, 1);
                sat_solver_addclause(pSat, Lits, Lits + 2);
            }
        }
    }

    // Write the circuit constraints (constraint 5 and 6)
    cout << "adding circuit constraint..." << endl;
    sat_solver_setnvars(pSat, Aig_ManObjNum(pAig1) + Aig_ManObjNum(pAig2));
    
    Aig2Cnf(pAig1, pAig2, Cnf_obj, true);
    WriteCnf(pSat, Cnf_obj);
    Cnf_obj.cnf_data_lift(Aig_ManCandNum(pAig1)); // one is const
    
    Aig2Cnf(pAig1, pAig2, Cnf_obj, false);
    WriteCnf(pSat, Cnf_obj);
    // pSat->fPrintClause =  0;
    Cnf_obj.cnf_data_lift(Aig_ManCandNum(pAig2)); // one is const

    // add constraint 4
    cout << "adding constraint 4" << endl;
    constraint4(pSat, pAig1, pAig2);

    constraint7(pSat, pAig1, pAig2);


    // add assumption
    // for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
    // {
    //     // cout << "i = " << i << " " <<  << endl;
    //     cout << "assump " <<  Abc_Var2Lit(i + i * Aig_ManCandNum(pAig2), 0) << endl;
    //     assump.push_back(Abc_Var2Lit(i + i * Aig_ManCandNum(pAig2), 0));
    // }
    assump.push_back(Abc_Var2Lit(0, 0));
    assump.push_back(Abc_Var2Lit(12, 0));
    assump.push_back(Abc_Var2Lit(24, 0));
    assump.push_back(Abc_Var2Lit(36, 0));
    assump.push_back(Abc_Var2Lit(48, 0));
    assump.push_back(Abc_Var2Lit(60, 0));
    status = sat_solver_solve(pSat, &assump.front(), &assump.back() + 1 , 0, 0, 0, 0);
    if ( status == l_False ) cout << "unsat, cut found!!!" << endl;
    if ( status == l_True ) cout << "sat, cut not found!!!" << endl;
    int *pfinal;
    int nfinal = sat_solver_final(pSat, &pfinal);
    cout << "eij assignment" << endl;
    for(i = 0; i < nfinal; ++i)
    {
        cout << ((pfinal[i] % 2 == 0) ? "" : "!") << Abc_Lit2Var(pfinal[i]) << endl;
    }
    if ( status == l_True )
    {
        for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
        {
            for(j = 0; j < Aig_ManCandNum(pAig2); ++j)
            {
                // cout << i * Aig_ManCandNum(pAig2) + j << endl;
                cout << sat_solver_var_value(pSat, i * Aig_ManCandNum(pAig2) + j) << " ";
            }
            cout << endl;
        }
        cout << "circuit 1 assignment" << endl;
        for(i = 1; i < Aig_ManCandNum(pAig1) + 1; ++i)
        {
            cout << sat_solver_var_value(pSat, Aig_ManCandNum(pAig2) + Aig_ManCandNum(pAig1) + i) <<  " ";
        }
        cout << endl;
        cout << "circuit 2 assignment" << endl;
        for(i = 0; i < Aig_ManCandNum(pAig2); ++i)
        {
            cout << sat_solver_var_value(pSat, Aig_ManCandNum(pAig2) + Aig_ManCandNum(pAig1) + Aig_ManCandNum(pAig1) + i) <<  " ";
        }
        cout << endl;
    }
    return;
    
    Cnf_CnfForClause( pCnf, pBeg, pEnd, i ) 
    {
        for(int * p = pBeg; p < pEnd; ++p)
            cout << Abc_Lit2Var(*p) << " ";
        cout << endl;
        // for(int * p = pBeg; p < pEnd; ++p)
        //     cout << *p << " ";
        // cout << endl;
        // cout << endl;
    }
    cout << "#clauses " << pCnf->nClauses << endl;
    
    return;
    
    

    // Add constraint 6 - At least one of the TFO of each PI is chosen
    Aig_ManForEachNodeReverse(pAig1, pObj1, i)
    {
        cout << "node id = " << pObj1->Id << endl;
    }
    cout << "PI num = " << Aig_ManCiNum(pAig1) << endl;

    status = sat_solver_solve(pSat, 0, 0 , 0, 0, 0, 0);
    if ( status == l_False ) cout << "bbb unsat" << endl;
    if ( status == l_True ) cout << "bbb sat" << endl;
    return;

    // cout << "hwioeshfioewhfiheiohfsiocho " << pSat -> size << " " << Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2) << endl;
    Aig_ManForEachNodeReverse( pAig1, pObj1, i )
    {
        // cout << Aig_ManLevelNum(pAig1) << " " << Aig_ObjLevel(pObj1) << endl;
        if(Aig_ManLevelNum(pAig1) == Aig_ObjLevel(pObj1)) continue; // we don't compute the similarity of CO
        Aig_ManForEachNodeReverse( pAig2, pObj2, j )
        {
            if(Aig_ManLevelNum(pAig2) == Aig_ObjLevel(pObj2)) continue; // we don't compute the similarity of CO
            // cout << "[r1 n" << pObj1->Id << "] [r2 n" << pObj2->Id;
            // similarity = computeSimilarityOf2Nodes(simValue1[pObj1->Id], simValue2[pObj2->Id], simValue1[Aig_ManCo(pAig1, 0)->Id], simValue2[Aig_ManCo(pAig2, 0)->Id], lookupTable, num_words);
            similarity = computeSimilarityOf2Nodes(pAig1, pAig2, simValue1[pObj1->Id], simValue2[pObj2->Id], simValue1, simValue2, lookupTable, num_words);
            tmp_obj_similarity.second = similarity.first;
            tmp_obj_similarity.first = pObj2->Id;
            similarity_table[pObj1->Id].push_back(tmp_obj_similarity);
            // cout << similarity_table[pObj1->Id].back() + 1.first << " " << similarity_table[pObj1->Id].back() + 1.second << endl;
            // cout << "similarity cofactor = " << similarity.first << " similarity good vector " << similarity.second<< endl;
        }
        if (stop) break;
    }
    for(i = 0; i < Aig_ManObjNum(pAig1); ++i)
    {
        sort(similarity_table[i].begin(), similarity_table[i].end(), similarity_vec_compare);
    }
    cout << "similarity is completed in " <<  float(clock() - time_start) / CLOCKS_PER_SEC << " seconds" << endl;


    match_table = new int[Aig_ManObjNum(pAig1)];
    vector<obj_similarity_t> match_priority;
    vector<obj_similarity_t> match_priority_copy;
    Aig_ManForEachNodeReverse( pAig1, pObj1, i )
    {
        if(Aig_ManLevelNum(pAig1) == Aig_ObjLevel(pObj1)) continue;
        tmp_obj_similarity.second = similarity_table[pObj1->Id].front().second;
        tmp_obj_similarity.first = pObj1 -> Id;
        match_priority.push_back(tmp_obj_similarity);
    }
    sort(match_priority.begin(), match_priority.end(), similarity_vec_compare);
    
    Aig_ManIncrementTravId(pAig2);
    for(i = 0; i < Aig_ManNodeNum(pAig1) - Aig_ManCoNum(pAig1); ++i)
    {
        for(j = 0; j < Aig_ManNodeNum(pAig2) - Aig_ManCoNum(pAig2); ++j)
        {
            if(Aig_ObjIsTravIdCurrent(pAig2, Aig_ManObj(pAig2, similarity_table[match_priority[i].first][j].first))) continue;
            match_table[match_priority[i].first] = similarity_table[match_priority[i].first][j].first;
            Aig_ObjSetTravIdCurrent(pAig2, Aig_ManObj(pAig2, similarity_table[match_priority[i].first][j].first));
            cout << match_priority[i].first << " " << similarity_table[match_priority[i].first][j].first << endl;
            pAigCone1 = Aig_ManCreateCone(pAig1, Aig_ManObj(pAig1, match_priority[i].first));
            pAigCone2 = Aig_ManCreateCone(pAig2, Aig_ManObj(pAig2, similarity_table[match_priority[i].first][j].first));
            pMiter = Aig_ManCreateMiter(pAigCone1, pAigCone2, 0); // the third parameter 0 means the miter is adding an xor
            pCnf = Cnf_Derive(pMiter,Aig_ManCoNum(pMiter));
            pSat = (sat_solver*)Cnf_DataWriteIntoSolver(pCnf, 1, 0);
            status = sat_solver_solve(pSat, 0, 0 , 0, 0, 0, 0);
            if ( status == l_False ) cout << "unsat" << endl;
            if ( status == l_True ) cout << "sat" << endl;
            // cout << "cone size = " << Aig_ManObjNum(pAigCone1) << endl;
            break;
        }
    }
    cout << "=======================================================" << endl;
    match_priority_copy = match_priority;
    for(int m = Aig_ManNodeNum(pAig2) - Aig_ManCoNum(pAig2) - 1; m > 0; --m)
    {
        match_priority = match_priority_copy;
        for(int n = m; n > 0; --n)
        {
            vec_swap(match_priority, n - 1, n);
            Aig_ManIncrementTravId(pAig2);
            for(i = 0; i < Aig_ManNodeNum(pAig1) - Aig_ManCoNum(pAig1); ++i)
            {
                for(j = 0; j < Aig_ManNodeNum(pAig2) - Aig_ManCoNum(pAig2); ++j)
                {
                    if(Aig_ObjIsTravIdCurrent(pAig2, Aig_ManObj(pAig2, similarity_table[match_priority[i].first][j].first))) continue;
                    match_table[match_priority[i].first] = similarity_table[match_priority[i].first][j].first;
                    Aig_ObjSetTravIdCurrent(pAig2, Aig_ManObj(pAig2, similarity_table[match_priority[i].first][j].first));
                    cout << match_priority[i].first << " " << similarity_table[match_priority[i].first][j].first << endl;
                    // pAigCone1 = Aig_ManObj(pAig1, match_priority[i].first);
                    break;
                }
            }
            
            cout << "=======================================================" << endl;
        }
    }
    
}

void abcAigMgr::cutMatching() {
    int i, j;
    Aig_Obj_t * pObj1, *pObj2;
    similarity_vec_t match_queue;
    obj_similarity_t tmp_obj_similarity;
    Aig_ManForEachNodeReverse( pAig1, pObj1, i )
    {
        if(Aig_ManLevelNum(pAig1) == Aig_ObjLevel(pObj1)) continue; // we don't compute the similarity of CO
        for(j = 0; j < Aig_ManNodeNum(pAig2) - Aig_ManCoNum(pAig2); ++j)
        {
            cout << "node1 " << pObj1->Id << " node2 " << similarity_table[pObj1->Id][j].first << " similarity " << similarity_table[pObj1->Id][j].second << endl;
        }
    }
}

#endif