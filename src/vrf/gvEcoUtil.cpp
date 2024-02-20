#ifndef GV_ECO_UTIL_CPP
#define GV_ECO_UTIL_CPP
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "proof/pdr/pdr.h"
#include "base/wln/wln.h"
#include "proof/fraig/fraig.h"
#include <unordered_map>
#include "cirMgr.h"
#include "cirGate.h"
#include "cirCut.h"

extern "C"{
  Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
  Gia_Man_t * Gia_ManFromAig( Aig_Man_t * p );
  Gia_Man_t * Gia_PolynCoreDetectTest( Gia_Man_t * pNtkMiterFraig, int fAddExtra, int fAddCones, int fVerbose );
  int Abc_NtkDarPdr( Abc_Ntk_t * pNtk, Pdr_Par_t * pPars );
  void Rtl_LibPrintStats( Rtl_Lib_t * p );
  void Rtl_LibPrintHieStats( Rtl_Lib_t * p );
  void Rtl_LibPrint( char * pFileName, Rtl_Lib_t * p );
  Gia_Man_t * Rtl_LibCollapse( Rtl_Lib_t * p, char * pTopModule, int fRev, int fVerbose );
  void Rtl_LibPrint( char * pFileName, Rtl_Lib_t * p );
  void Rtl_LibBlast( Rtl_Lib_t * pLib );
  void Rtl_LibBlast2( Rtl_Lib_t * pLib, Vec_Int_t * vRoots, int fInv );
  Gia_Man_t * Wln_BlastSystemVerilog( char * pFileName, char * pTopModule, char * pDefines, int fSkipStrash, int fInvert, int fTechMap, int fVerbose );
  void Abc_NtkShow( Abc_Ntk_t * pNtk, int fGateNames, int fSeq, int fUseReverse, int fKeepDot );
}


using namespace std;

void Net2PO( Abc_Ntk_t* pNtk)
{
    int c;
    int fCheck, fBarBufs;
    char * pFileName;

	fCheck = 1;
    fBarBufs = 0;

    //read verilog file
    //pull internal signals to po
    int i;
    Abc_Obj_t* 	pNode;
    Abc_Obj_t* 	pNet;
    char*		str;

    Abc_NtkForEachNode( pNtk, pNode, i )
    {
		str = Abc_ObjName( pNode );
		if ( Abc_ObjFaninNum(pNode) == 0 ) 
		{
			if ( !strcmp( Abc_ObjName(pNode), "1'b0" ) || !strcmp( Abc_ObjName(pNode), "1'b1") ) continue;
			if ( !Abc_ObjFanoutNum(pNode) ) continue;
			str = Abc_ObjName( Abc_ObjFanout0(pNode) );
		}

		pNet = Abc_NtkCreatePo( pNtk );
		Abc_ObjAddFanin( pNet, pNode );
    Abc_ObjAssignName( pNet, str, "_INT" );

		
    //printf( "create node after miter (old): %x\t%x\t(%s)\n", pNode, pNode->pCopy, str);
    }

}

string abcNetId2NewName(Abc_Ntk_t* pNtk, int id) {
  return "n" + to_string(id-(Abc_NtkPoNum(pNtk)-1));
}

string abcNetId2Name(int id) {
  return "n" + to_string(id);
}


CirMgr* ECO_FileRead(string oldName, string newName) {
    unordered_map<Abc_Obj_t *, string> aigNode2Gate;
    unordered_map<Abc_Obj_t *, bool> aigNode2GateComp;
    unordered_map<Abc_Obj_t *, vector<string> > aigNode2GateMiter;
    unordered_map<Abc_Obj_t *, vector<bool> > aigNode2GateCompMiter;
    unordered_map<string, vector<string> > aigNodeName2GateMiter;
    unordered_map<string, vector<bool> > aigNodeName2GateCompMiter;
    unordered_map<string, Abc_Obj_t *> name2Gate;
    Fraig_Params_t Params, * pParams = &Params;
    int fAllNodes;
    int fExdc;
    Abc_Obj_t * pObj;
    int i;
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
    fExdc     = 0;
    fAllNodes = 0;

    // prepare the two circuits for eco
    Abc_Ntk_t* pNtkOld = Io_Read( const_cast<char*>(oldName.c_str()), IO_FILE_VERILOG, 0, 0 );
    Abc_Ntk_t* pNtkNew = Io_Read( const_cast<char*>(newName.c_str()), IO_FILE_VERILOG, 0, 0 );
    
    // set the ntk name
    pNtkOld->pName = "Old";
    pNtkNew->pName = "New";

    // pull the internal signal to po
    // Net2PO(pNtkOld, 1);
    // Net2PO(pNtkNew, 0);
    
    // debug : traverse & print the nets in the old circuit
    Abc_Obj_t* 	pNode;
    
    //strash
    Abc_Ntk_t* pNtkOldStrash = Abc_NtkStrash( pNtkOld, fAllNodes, !fAllNodes, 0 );
    Abc_Ntk_t* pNtkNewStrash = Abc_NtkStrash( pNtkNew, fAllNodes, !fAllNodes, 0 );

    

    // build the circuit of new/old circuit
    CirMgr* cirMgrOld = new CirMgr;
    CirMgr* cirMgrNew = new CirMgr;

    cirMgrOld->readCirFromAbcNtk(pNtkOldStrash);
    cirMgrNew->readCirFromAbcNtk(pNtkNewStrash);

    // enumerate cut
    CirCutMan* pCutManOld=new CirCutMan;
    CirGate::setGlobalRef();
    for(int i=0; i<cirMgrOld->getNumTots(); i++) {
      CirGate* gate=cirMgrOld->getGate(i);
      if(!gate) continue;
      if(gate && gate->getType()!=AIG_GATE) continue;
      
      pCutManOld->gateGetCuts(gate, 5);
    }
    CirCutMan* pCutManNew=new CirCutMan;
    for(int i=0; i<cirMgrNew->getNumTots(); i++) {
      CirGate* gate=cirMgrNew->getGate(i);
      if(!gate) continue;
      if(gate && gate->getType()!=AIG_GATE) continue;
      
      pCutManNew->gateGetCuts(gate, 5);
    }
    
    // store the node mapping...
    Abc_NtkForEachNode( pNtkOld, pNode, i )
    {
      string tmp = Abc_ObjName( pNode );
      aigNode2Gate[Abc_ObjRegular(pNode->pCopy)] = tmp + "_O";
      aigNode2GateComp[Abc_ObjRegular(pNode->pCopy)] = Abc_ObjIsComplement(pNode->pCopy);
    }

    Abc_NtkForEachNode( pNtkNew, pNode, i )
    {
      string tmp = Abc_ObjName( pNode );
      aigNode2Gate[Abc_ObjRegular(pNode->pCopy)] = tmp + "_N";
      aigNode2GateComp[Abc_ObjRegular(pNode->pCopy)] = Abc_ObjIsComplement(pNode->pCopy);
    }

    // compute the miter
    Abc_Ntk_t* pNtkMiter = Abc_NtkMiter( pNtkOldStrash, pNtkNewStrash, 0, 0, 0, 0 );
    Abc_NtkForEachNode( pNtkOldStrash, pNode, i )
    {
      if(aigNode2Gate.find(pNode)==aigNode2Gate.end()) continue;
      aigNode2GateMiter[pNode->pCopy].push_back(aigNode2Gate[pNode]);
      aigNode2GateCompMiter[pNode->pCopy].push_back(aigNode2GateComp[pNode]);
    }
    Abc_NtkForEachNode( pNtkNewStrash, pNode, i )
    {
      if(aigNode2Gate.find(pNode)==aigNode2Gate.end()) continue;
      aigNode2GateMiter[pNode->pCopy ].push_back(aigNode2Gate[pNode]);
      aigNode2GateCompMiter[pNode->pCopy].push_back(aigNode2GateComp[pNode]);
    }

    // print map information
    Abc_NtkForEachNode(pNtkMiter, pNode, i) {
      int n=aigNode2GateMiter[pNode].size();
      for(int j=0; j<n; j++) {
        name2Gate[Abc_ObjName(pNode)] = pNode;
        aigNodeName2GateMiter[Abc_ObjName(pNode)].push_back(aigNode2GateMiter[pNode][j]);
        aigNodeName2GateCompMiter[Abc_ObjName(pNode)].push_back(aigNode2GateCompMiter[pNode][j]);
      }
    }

    // pull the internal signal to PO
    Net2PO(pNtkMiter);
    Abc_Ntk_t* pNtkMiterFraig = Abc_NtkFraig( pNtkMiter, &Params, fAllNodes, fExdc );

    // map the names
    Abc_NtkForEachPo( pNtkMiterFraig, pNode, i )
    {
      string tmp = Abc_ObjName( pNode );
      if(tmp.substr(tmp.length()-4, 4)!="_INT") continue;
      tmp = tmp.substr(0, tmp.length()-4);
      if(name2Gate.find(tmp)==name2Gate.end()) continue;
      int n=aigNode2GateMiter[name2Gate[tmp]].size();
      for(int j=0; j<n; j++) {
        if(aigNode2GateMiter.find(name2Gate[tmp])==aigNode2GateMiter.end()) continue;
        aigNodeName2GateMiter[abcNetId2NewName(pNtkMiterFraig, Abc_ObjId(Abc_ObjFanin0(pNode)))].push_back(aigNode2GateMiter[name2Gate[tmp]][j]);
        aigNodeName2GateCompMiter[abcNetId2NewName(pNtkMiterFraig, Abc_ObjId(Abc_ObjFanin0(pNode)))].push_back(Abc_ObjFaninC0(pNode)^aigNode2GateCompMiter[name2Gate[tmp]][j]);
      }
    }
    
    // delete the dummy po's
    vector<int> poIdx2Delete;
    Abc_NtkForEachPo( pNtkMiterFraig, pNode, i )
    {
      string tmp = Abc_ObjName( pNode );
      if(tmp.substr(tmp.length()-4, 4)=="_INT") {
        poIdx2Delete.push_back(i);
      }
      
    }
    for(auto poIdx=poIdx2Delete.rbegin(); poIdx!=poIdx2Delete.rend(); poIdx++) {
      Abc_NtkDeleteObj(Abc_NtkPo(pNtkMiterFraig, *poIdx));
    }
    // reassign the id's of the ntk
    Abc_NtkReassignIds(pNtkMiterFraig);
    
    // check if the circuit is ok...
    assert(Abc_NtkCheck(pNtkMiterFraig));

    // Abc_NtkShow( pNtkMiterFraig, 0, 0, 1, 0 );

    // Convert to gv ntk
    cirMgr = new CirMgr;
    cirMgr->readCirFromAbcNtk(pNtkMiterFraig);
    return cirMgr;
}

void
CirMgr::ReadSimVal() {
  unsigned nObjs = cirMgr->getNumTots();
  size_t patterns[nObjs];
  // cirMgr->printNetlist();
  for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      // if(cirMgr->getGate(i)->getPValue()()) continue;
      patterns[i] = _dfsList[i]->getPValue()();
  }
  
      for (unsigned i = 0; i < _dfsList.size(); ++i) {
        // cout << _dfsList[i]->getName() << endl;
        for (size_t j = 0; j < 64; ++j) {
          // cout << (patterns[i] & 0x1);
          patterns[i] >>= 1;
        }
        // cout << endl;
      }
      
}

void DoEco(CirMgr* cirMgr) {
  // cout << "simulation" << endl;
  ofstream logFile;
  // logFile.open("simout.txt", ios::out);
  // cirMgr->setSimLog(&logFile);
  cirMgr->randomSim();
  cirMgr->ReadSimVal();
  
}

#endif
