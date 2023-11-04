/****************************************************************************
  FileName     [ cirAig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define basic cir package commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ ]
****************************************************************************/
#include <cirMgr.h>
#include <cirGate.h>
#include "base/abc/abc.h"
#include "gvModMgr.h"
#include "gvAbcMgr.h"

void 
CirMgr::readCirFromAbc(string fileName, CirFileType fileType) {
    // TODO : Convert abc ntk to gv aig ntk
    CirGateV gateV;
    Gia_Man_t* pGia = NULL;            // the gia pointer of abc
    Gia_Obj_t *pObj, *pObjRi, *pObjRo; // the obj element of gia
    unsigned iPi = 0, iPpi = 0, iPo = 0, iRi = 0, iRo = 0;
    unsigned piNum = 0;
    map<int,int> PPI2RO;

    // abc function parameters
    char* pFileName = new char[100];
    cout << "filename = " << fileName << endl;
    strcpy(pFileName, fileName.c_str());
    char* pTopModule  = NULL; // the top module can be auto detected by yosys, no need to set
    char* pDefines    = NULL;
    int   fBlast      = 1; // blast the ntk to gia (abc's aig data structure)
    int   fInvert     = 0;
    int   fTechMap    = 1;
    int   fSkipStrash = 0;
    int   fCollapse   = 0;
    int   c, fVerbose = 1; // set verbose to 1 to see which yosys command is used
    int   i, *pWire;

    if(fileType == AIGER){
        pGia = Gia_AigerRead(pFileName, 0, fSkipStrash, 0);
        piNum = Gia_ManPiNum(pGia);
    }
    else if(fileType == VERILOG) {
        pGia = abcMgr->get_Abc_Frame_t()->pGia;
        pGia = Wln_BlastSystemVerilog(pFileName,pTopModule,pDefines,fSkipStrash,fInvert,fTechMap,fVerbose);
        piNum = Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1; // PI (PI + PPI) - (REG (PPI + CONST0) - CONST0)
        // cout << Gia_ManPiNum(pGia) << endl;
        // cout << Gia_ManRegNum(pGia) << endl;
        // cout << "PI NUM : " << piNum << endl;
    }

    // initialize the size of the containers
    initCir(pGia, fileType);
    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);

    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));

    // traverse the obj's in topological order
    Gia_ManForEachObj(pGia, pObj, i) {
        if(Gia_ObjIsPi(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if(gateId <= piNum){
                CirPiGate* gate = new CirPiGate(Gia_ObjId(pGia, pObj), 0);
                _piList[iPi++] = gate;
                _totGateList[gateId] = gate;
            }
            else{
                CirRoGate* gate = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                _roList[iRo++] = gate;
                _totGateList[gateId] = gate;
            }
        }
        else if(Gia_ObjIsPo(pGia, pObj)) {
            CirPoGate *gate = new CirPoGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            gate->setIn0(getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            _poList[iPo++] = gate;
            _totGateList[Gia_ObjId(pGia, pObj)] = gate;
        }
        else if(Gia_ObjIsAnd(pObj)) {
            bool inv0 = false, inv1 = false;
            CirAigGate *gate = new CirAigGate(Gia_ObjId(pGia, pObj), 0);
            _totGateList[Gia_ObjId(pGia, pObj)] = gate;
            int fanin0 = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int fanin1 = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
            if(PPI2RO.count(fanin0)) {
                // fanin0 = PPI2RO[fanin0];
                // if(fanin0 == 0) inv0 = true;
            }
            if(PPI2RO.count(fanin1)) {
                // fanin1 = PPI2RO[fanin1];
                // if(fanin1 == 0) inv1 = true;
            }
            gate->setIn0(getGate(fanin0), Gia_ObjFaninC0(pObj));
            gate->setIn1(getGate(fanin1), Gia_ObjFaninC1(pObj));
        }
        else if(Gia_ObjIsRo(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia,pObj);
            if(fileType == VERILOG){
                if(iPpi < iRo) PPI2RO[gateId] = _roList[iPpi]->getGid();
                else PPI2RO[gateId] = 0;
                iPpi++;
            }
            else if(fileType == AIGER){
                if(iRo < _roList.size()){
                    CirRoGate* gate = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                    _roList[iRo++] = gate;
                    _totGateList[gateId] = gate;
                }
                else{
                    CirRoGate* gate = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                    _roList[iRo++] = gate;
                    _totGateList[gateId] = gate;
                    // PPI2RO[gateId] = 0;
                }
            }
        }
        else if(Gia_ObjIsRi(pGia, pObj)) {
            CirRiGate *gate = new CirRiGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            string str = to_string(Gia_ObjId(pGia, pObj)); str = str + "_ns";
            char *n = new char[str.size()+1]; strcpy(n, str.c_str());
            gate->setName(n);
            gate->setIn0(getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            _riList[iRi++] = gate;
            _totGateList[Gia_ObjId(pGia, pObj)] = gate;
        }
        else if(Gia_ObjIsConst0(pObj)) {
            _totGateList[0] = CirMgr::_const0;
        }
        else {
            assert(true);
            cout << "not defined gate type" << endl;
        }
    }
    
    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        if(i == getNumLATCHs()) break;

        int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
        // int roGid = PPI2RO[Gia_ObjId(pGia,pObjRo)];
        // int roGid = Gia_ObjId(pGia,pObjRo);
        if(fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia,pObjRo)];
        else if(fileType == AIGER) roGid = Gia_ObjId(pGia,pObjRo);
        CirGate*   riGate = getGate(riGid);
        CirRoGate* roGate = static_cast<CirRoGate*>(getGate(roGid));
        roGate->setIn0(riGate, false);

        // cout << Gia_ObjId(pGia, pObjRi) << endl;
        // cout << Gia_ObjId(pGia, pObjRo) << endl;//<< " <---> "<< PPI2RO[Gia_ObjId(pGia,pObjRo)]<< endl;
        // cout << " --- \n";
    }
    // DEBUG
    _const1 = new CirAigGate(getNumTots(), 0 ); addTotGate(_const1);
    _const1->setIn0(_const0, true); _const1->setIn1(_const0, true);

    // DEBUG
    // CirGate*   riGate = getGate(1034);
    // CirRoGate* roGate = static_cast<CirRoGate*>(getGate(136));
    // roGate->setIn0(riGate, false);

    // genConnections();
    genDfsList();
    // checkFloatList();
    // checkUnusedList();

    // DEBUG
    for(int i = 0; i < _totGateList.size(); ++i){
        CirGate* g = _totGateList[i];
        // if (g->getType() == RI_GATE || g->getType() == PO_GATE) isPrint = true;
        if(g) g->printGate();
    }
    _poList[0]->printGate();

}

CirGate* 
CirMgr::createGate(unsigned id, GateType type) {
    CirGate* gate = _totGateList[id];
    unsigned l = 0;
    switch (type) {
      case PI_GATE: gate = new CirPiGate(id, l); break;
      case AIG_GATE: gate = new CirAigGate(id, l); break;
      //case LATCH: gate = new CirRoGate(litId/2, lineNo+1); break;
      default: cerr << "Error: Unknown gate type (" << type << ")!!\n";
               exit(-1);
   }
   return gate;
}

void 
CirMgr::initCir(Gia_Man_t* pGia,const CirFileType& fileType) {
    // TODO : Resize the list (PI/PO ...)
    // Create lists
    cout << "initializing ..." << endl;
    //    _numDecl[PI] = Gia_ManPiNum(pGia);
    //    _piList = new CirPiGate*[_numDecl[PI]];
    //    _numDecl[PO] = Gia_ManPoNum(pGia);
    //    _numDecl[LATCH] = Gia_ManRegNum(pGia) - 1;
    //    _poList = new CirPoGate*[_numDecl[PO]];
    //    _riList = new CirRiGate*[_numDecl[LATCH]];
    //    _roList = new CirRoGate*[_numDecl[LATCH]];
    //    _numDecl[AIG] = Gia_ManAndNum(pGia);
    //    _numDecl[VARS] = Gia_ManObjNum(pGia);
    //    _totGateList = new CirGate*[_numDecl[VARS]];
    //    _numDecl[PI] = Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1;
    _poList.resize(Gia_ManPoNum(pGia));
    int piNum = Gia_ManPiNum(pGia);
    int regNum = Gia_ManRegNum(pGia);
    if(fileType == VERILOG) _piList.resize(Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1);
    else if(fileType == AIGER)  _piList.resize(Gia_ManPiNum(pGia)) ;
    // _riList.resize(Gia_ManRegNum(pGia) - 1);
    // _roList.resize(Gia_ManRegNum(pGia) - 1);
    _riList.resize(Gia_ManRegNum(pGia));
    _roList.resize(Gia_ManRegNum(pGia));
    _totGateList.resize(Gia_ManObjNum(pGia));
}
