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

void 
CirMgr::readCirFromAbc(string fileName) {
    // TODO : Convert abc ntk to gv aig ntk
    Gia_Man_t* pGia = NULL;            // the gia pointer of abc
    Gia_Obj_t *pObj, *pObjRi, *pObjRo; // the obj element of gia

    // abc function parameters
    char* pFileName;
    strcpy(pFileName, fileName.c_str());
    char* pTopModule =
        NULL; // the top module can be auto detected by yosys, no need to set
    char* pDefines    = NULL;
    int   fBlast      = 1; // blast the ntk to gia (abc's aig data structure)
    int   fInvert     = 0;
    int   fTechMap    = 1;
    int   fSkipStrash = 0;
    int   fCollapse   = 0;
    int   c, fVerbose = 1; // set verbose to 1 to see which yosys command is used
    int   i, *pWire;

    pGia = Gia_AigerRead(pFileName, 0, fSkipStrash, 0);

    // initialize the size of the containers
    initCir(pGia);
    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);

    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));

    cout << "setting PI's" << endl;
    // create the PI's from gia
    Gia_ManForEachPi(pGia, pObj, i) {
        CirGate* gate = createGate(Gia_ObjId(pGia, pObj), PI_GATE);
        _piList[i] = static_cast<CirPiGate*>(gate);
    }

    cout << "setting PO's" << endl;
    // create the PO
    Gia_ManForEachPo(pGia, pObj, i) {
        CirGate *gate = new CirPoGate(Gia_ObjId(pGia, pObj), 0);
        _poList[i] = static_cast<CirPoGate*>(gate);
    //   _totGateList[poId] = gate;
        // create a new GVNetId corresponding to abc's id
        // GVNetId id =
        //     GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PO);
        // createNet(id, GV_NTK_OBJ_PO);
        // // map
        // _id2GVNetId[id.id] = id;
        // _id2Type[id.id]    = id.type;
    }
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
CirMgr::initCir(Gia_Man_t* pGia) {
    // TODO : Resize the list (PI/PO ...)
    // Create lists
    cout << "initializing..." << endl;
   _piList = new CirPiGate*[Gia_ManPiNum(pGia)];
   _poList = new CirPoGate*[Gia_ManPoNum(pGia)];
   _riList = new CirRiGate*[Gia_ManRegNum(pGia)];
   _roList = new CirRoGate*[Gia_ManRegNum(pGia)];
   _totGateList = new CirGate*[Gia_ManObjNum(pGia)];
}
