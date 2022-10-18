#ifndef GV_AIG_MGR_C
#define GV_AIG_MGR_C

#include <iostream>
#include <iomanip>
#include "gvAigMgr.h"
#include "rnGen.h"
#include "proof/fraig/fraig.h"
#include "aig/aig/aig.h"

typedef struct Fraig_ParamsStruct_t_   Fraig_Params_t;

extern "C" 
{
    Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
    int Aig_ManCutCount( Aig_ManCut_t * p, int * pnCutsK );
}

void abcAigMgr::init()
{
    int i;
    Aig_Obj_t* pObj;
    pMan = Abc_NtkToDar(pNtk, 0, 1);

    Total_num = Aig_ManObjNum(pMan);
    PI_num = Aig_ManCiNum(pMan);
    PO_num = Aig_ManCoNum(pMan);
    Node_num = Aig_ManAndNum(pMan);

    PI_List = new Aig_Obj_t*[PI_num];
    PO_List = new Aig_Obj_t*[PO_num];

    Aig_ManForEachCi(pMan, pObj, i) {
        PI_List[i] = pObj;
    }
    Aig_ManForEachCo(pMan, pObj, i) {
        PO_List[i] = pObj;
    }
}

void abcAigMgr::printSummary(bool verbose) const
{
    cout << endl;
    cout << "Circuit Statistics" << endl;
    cout << "==================" << endl;
    cout << "  PI  " << setw(10) << right << PI_num << endl;
    cout << "  PO  " << setw(10) << right << PO_num << endl;
    cout << "  AND " << setw(10) << right << Node_num << endl;
    cout << "------------------" <<endl;
    cout << "  Total" << setw(10) << right << PI_num + PO_num + Node_num << endl;

    if (verbose) {
        Aig_Obj_t* pObj;
        int i;
        Aig_ManForEachObj(pMan, pObj, i) {
            printf("Object Id = %d, Object Type = %d\n", Aig_ObjId(pObj), Aig_ObjType(pObj));
            Aig_Obj_t* pFanin;
            int j;
            if (Aig_ObjType(pObj) == AIG_OBJ_CO)
                printf("  Fanin-0: Id = %d\n", Aig_ObjFaninId0(pObj));
            else if (Aig_ObjType(pObj) == AIG_OBJ_AND) {
                printf("  Fanin-0: Id = %d\n", Aig_ObjFaninId0(pObj));
                printf("  Fanin-1: Id = %d\n", Aig_ObjFaninId1(pObj));
            }
        }
    }
}

void abcAigMgr::printPIs() const
{
    cout << "PIs of the AIG:";
    for(int i = 0; i < PI_num; ++i) {
        std::cout << " " << Aig_ObjId(PI_List[i]);
    }
    cout << endl;
}

void abcAigMgr::printPOs() const
{
    cout << "POs of the AIG:";
    for(int i = 0; i < PO_num; ++i) {
        std::cout << " " << Aig_ObjId(PO_List[i]);
    }
    cout << endl;
}

void abcAigMgr::Aig_AllDFS()
{
    int i;
    Aig_Obj_t* pObj;
    DFS_List = new Aig_Obj_t*[Total_num];
    Vec_Ptr_t* vNodes = Vec_PtrAlloc(Aig_ManObjNum(pMan));
    vNodes = Aig_ManDfsAll(pMan);
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i){
        DFS_List[i] = pObj;
    }
    Vec_PtrFree(vNodes);
    // for(i = 0; i < Total_num; ++i) 
    //     cout << Aig_ObjId(DFS_List[i]) << " ";
    // cout << endl;
    cout << "Successfully construct DFS_List!" << endl;

}

Aig_Obj_t** abcAigMgr::Aig_NodesDFS(Aig_Obj_t** pNodes, int nNodes) const
{
    int i;
    Aig_Obj_t* pObj;
    Vec_Ptr_t* vNodes = Vec_PtrAlloc(Aig_ManObjNum(pMan));
    vNodes = Aig_ManDfsNodes(pMan, pNodes, nNodes);
    Aig_Obj_t** dfsList = new Aig_Obj_t*[Vec_PtrSize(vNodes)];
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i){
        dfsList[i] = pObj;
    }
    Vec_PtrFree(vNodes);
    return dfsList;
}

void abcAigMgr::fraig() {

    Fraig_Params_t Params, *pParams = &Params;
    Abc_Ntk_t * pNtkRes;
    int fAllNodes;
    int fExdc;
    int c;
    int fPartition = 0;
    fExdc     = 0;
    fAllNodes = 0;
    memset( pParams, 0, sizeof(Fraig_Params_t) );
    pParams->nPatsRand  = 2048; // the number of words of random simulation info
    pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
    pParams->nBTLimit   =  100; // the max number of backtracks to perform
    pParams->fFuncRed   =    1; // performs only one level hashing
    pParams->fFeedBack  =    1; // enables solver feedback
    pParams->fDist1Pats =    1; // enables distance-1 patterns
    pParams->fDoSparse  =    1; // performs equiv tests for sparse functions
    pParams->fChoicing  =    0; // enables recording structural choices
    pParams->fTryProve  =    0; // tries to solve the final miter
    pParams->fVerbose   =    0; // the verbosiness flag
    pParams->fVerboseP  =    0; // the verbosiness flag


    if ( !Abc_NtkIsLogic(pNtk) && !Abc_NtkIsStrash(pNtk) )
    {
        cout << "Can only fraig a logic network or an AIG.\n" << endl;
        return;
    }

    // report the proof
    pParams->fVerboseP = pParams->fTryProve;
    if ( Abc_NtkIsStrash(pNtk) )
        pNtkRes = Abc_NtkFraig( pNtk, &Params, fAllNodes, fExdc );
    else {
        pNtk = Abc_NtkStrash( pNtk, fAllNodes, !fAllNodes, 0 );
        pNtkRes = Abc_NtkFraig( pNtk, &Params, fAllNodes, fExdc );
        Abc_NtkDelete( pNtk );
    }
    if ( pNtkRes == NULL ) {
        cout << "Fraiging has failed.\n" << endl;
        return;
    }
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    cout << "Fraiging has finished.\n" << endl;
}

void abcAigMgr::randomSim() {
    int i;
    Aig_Obj_t* pObj;
    
    RandomNumGen  rnGen(0);
    Vec_Ptr_t* vNodes = Vec_PtrAlloc(Total_num);
    vNodes = Aig_ManDfsAll(pMan);
    size_t* simValue = new size_t [Total_num] ();
    vector<Aig_Obj_t* > PIs;

    for (unsigned i = 0; i < Total_num; ++i) {
        simValue[i] = 0;
    }

    for (unsigned i = 0; i < PI_num; ++i) {
        PIs.push_back(Aig_ManCi(pMan, i));
    }

    unsigned pos = 64, same = 0;
  	size_t* sim_64 = new size_t [PI_num] ();

    while(same <= 10) {
        for (unsigned i = 0; i < pos ; ++i) {
  			for(unsigned j = 0; j < PI_num; ++j){
	  			sim_64[j] =  ((size_t)rnGen(2) << i) | sim_64[j];
	  		}
        }
        for (unsigned j = 0; j < PI_num; ++j) {
  			simValue[PIs[j]->Id] =  sim_64[j];
  			sim_64[j] = 0;
  		}
        simTraversal(vNodes, simValue);
        printf("\n");
  	    same++;
    }
}

void convertToBinary(unsigned int n)
{
    if (n / 2 != 0) {
        convertToBinary(n / 2);
    }
    printf("%d", n % 2);
}

void abcAigMgr::simTraversal( Vec_Ptr_t* vNodes, size_t* simValue)
{
	int i;
    size_t fa0, fa1;
    Aig_Obj_t* pObj;
    Vec_PtrForEachEntry(Aig_Obj_t*, vNodes, pObj, i) {
        if (Aig_ObjIsAnd(pObj)) {
            fa0 = simValue[Aig_ObjFaninId0(pObj)];
            fa1 = simValue[Aig_ObjFaninId1(pObj)];
            if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
            if (Aig_ObjFaninC1(pObj)) fa1 = ~fa1;
            simValue[Aig_ObjId(pObj)] = fa0 & fa1;
        }
        else if (Aig_ObjIsCo(pObj)) {
            fa0 = simValue[Aig_ObjFaninId0(pObj)];
            if (Aig_ObjFaninC0(pObj)) fa0 = ~fa0;
            simValue[Aig_ObjId(pObj)] = fa0;
        }
    }
    for (unsigned i = 0; i < Total_num; ++i) {
        convertToBinary(simValue[i]);
        cout << "\n";
    }  
    cout << "\n"; 
}


void abcAigMgr::rewireByFanins(Aig_Obj_t * pObj, Aig_Obj_t * pFan0, Aig_Obj_t * pFan1 ){
    if (Aig_ObjId(pFan0) < Aig_ObjId(pFan1)) 
        Aig_ObjConnect(pMan, pObj, pFan0, pFan1);
    else
        Aig_ObjConnect(pMan, pObj, pFan1, pFan0);
}


void abcAigMgr::Aig_EnumerateCuts( int nCutsMax, int nLeafMax, int fTruth, bool verbose ){
    Aig_Man_t * pAig = pMan;
    // Aig_ManCut_t * p;
    Aig_Obj_t * pObj;
    Aig_Cut_t * pCut;
    int i, k;
    int * pnCutsK;
    if ( !Abc_NtkIsLogic(pNtk) && !Abc_NtkIsStrash(pNtk) )
    {
        cout << "Can only fraig a logic network or an AIG.\n" << endl;
        return;
    }
    pManCut = Aig_ComputeCuts( pAig, nCutsMax, nLeafMax, fTruth, 0 );
    if(verbose)
    {
        Aig_ManForEachNode( pManCut->pAig, pObj, i )
        {
            Aig_ObjForEachCut( pManCut, pObj, pCut, k )
            {
                if ( pCut->nFanins == 0 )
                    continue;
                cout << "cut size = " << Aig_CutLeaveNum(pCut) << endl;
            }
        }
    }
    cout << Aig_ManCutCount(pManCut, pnCutsK) << "cuts found" << endl;
}

#endif