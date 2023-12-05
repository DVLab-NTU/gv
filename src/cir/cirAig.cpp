/****************************************************************************
  FileName     [ cirAig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define GV cir/aig functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cirGate.h>
#include <cirMgr.h>

#include <map>

#include "gvAbcMgr.h"
#include "gvAbcNtk.h"
#include "gvYosysMgr.h"
#include "util.h"

const bool CirMgr::readCirFromAbc(string fileName, CirFileType fileType) {
    Gia_Man_t* pGia = NULL;  // the gia pointer of abc
    map<unsigned, string> id2Name;
    ABCParam param;
    strcpy(param.pFileName, fileName.c_str());
    cout << "filename = " << fileName << endl;

    if (fileType == AIGER) {
        abcMgr->readAig(param);
        pGia = abcMgr->pGia;
    } else if (fileType == VERILOG) {
        param.fBlast   = 1;
        param.fTechMap = 1;
        abcMgr->readVerilog(param);
        pGia = abcMgr->pGia;
    }

    if (!pGia) {
        cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
        return false;
    }

    if (fileType == VERILOG) {
        yosysMgr->createMapping(fileName);
        abcMgr->buildAigName(id2Name);
    }

    // initialize the size of the containers
    initCir(pGia, fileType);
    abcMgr->travPreprocess();
    abcMgr->travAllObj(_piList, _poList, _roList, _riList, _totGateList, id2Name, fileType);

    // traverse the obj's in topological order
    // int i;
    // Gia_ManForEachObj(pGia, pObj, i) {
    //     if (Gia_ObjIsPi(pGia, pObj)) {
    //         int gateId = Gia_ObjId(pGia, pObj);
    //         if (gateId <= _piList.size()) {
    //             CirPiGate* gate      = new CirPiGate(Gia_ObjId(pGia, pObj), 0);
    //             _piList[iPi++]       = gate;
    //             _totGateList[gateId] = gate;
    //         } else {
    //             CirRoGate* gate      = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
    //             _roList[iRo++]       = gate;
    //             _totGateList[gateId] = gate;
    //         }
    //     } else if (Gia_ObjIsPo(pGia, pObj)) {
    //         string poName   = "";
    //         CirPoGate* gate = new CirPoGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
    //         poName          = (id2Name.count(gate->getGid())) ? id2Name[gate->getGid()] : to_string(Gia_ObjId(pGia, pObj));
    //         char* n         = new char[poName.size() + 1];
    //         strcpy(n, poName.c_str());
    //         gate->setName(n);
    //         gate->setIn0(getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
    //         _poList[iPo++]                      = gate;
    //         _totGateList[Gia_ObjId(pGia, pObj)] = gate;
    //     } else if (Gia_ObjIsAnd(pObj)) {
    //         CirAigGate* gate = new CirAigGate(Gia_ObjId(pGia, pObj), 0);
    //         int fanin0       = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
    //         int fanin1       = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
    //         if (PPI2RO.count(fanin0)) fanin0 = PPI2RO[fanin0];
    //         if (PPI2RO.count(fanin1)) fanin1 = PPI2RO[fanin1];
    //         gate->setIn0(getGate(fanin0), Gia_ObjFaninC0(pObj));
    //         gate->setIn1(getGate(fanin1), Gia_ObjFaninC1(pObj));
    //         _totGateList[Gia_ObjId(pGia, pObj)] = gate;
    //     } else if (Gia_ObjIsRo(pGia, pObj)) {
    //         int gateId = Gia_ObjId(pGia, pObj);
    //         if (fileType == VERILOG) {
    //             PPI2RO[gateId] = 0;
    //             if (iPpi < iRo) PPI2RO[gateId] = _roList[iPpi]->getGid();
    //             iPpi++;
    //         } else if (fileType == AIGER) {
    //             CirRoGate* gate      = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
    //             _roList[iRo++]       = gate;
    //             _totGateList[gateId] = gate;
    //         }
    //     } else if (Gia_ObjIsRi(pGia, pObj)) {
    //         CirRiGate* gate = new CirRiGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
    //         string str      = to_string(Gia_ObjId(pGia, pObj));
    //         str             = str + "_ns";
    //         char* n         = new char[str.size() + 1];
    //         strcpy(n, str.c_str());
    //         gate->setName(n);
    //         gate->setIn0(getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
    //         _riList[iRi++]                      = gate;
    //         _totGateList[Gia_ObjId(pGia, pObj)] = gate;
    //     } else if (Gia_ObjIsConst0(pObj)) {
    //         _totGateList[0] = CirMgr::_const0;
    //     } else {
    //         cout << "not defined gate type" << endl;
    //         assert(true);
    //     }
    // }

    // Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
    //     if (i == getNumLATCHs()) break;

    //     int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
    //     // int roGid = PPI2RO[Gia_ObjId(pGia,pObjRo)];
    //     // int roGid = Gia_ObjId(pGia,pObjRo);
    //     if (fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia, pObjRo)];
    //     else if (fileType == AIGER) roGid = Gia_ObjId(pGia, pObjRo);
    //     CirGate* riGate   = getGate(riGid);
    //     CirRoGate* roGate = static_cast<CirRoGate*>(getGate(roGid));
    //     roGate->setIn0(riGate, false);
    // }
    // CONST1 Gate
    _const1 = new CirAigGate(getNumTots(), 0);
    addTotGate(_const1);
    _const1->setIn0(_const0, true);
    _const1->setIn1(_const0, true);

    genDfsList();
    return true;
}

void CirMgr::initCir(Gia_Man_t* pGia, const CirFileType& fileType) {
    // Create lists
    int piNum = 0, regNum = 0, poNum = 0, totNum = 0;
    if (fileType == VERILOG) {
        piNum  = Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1;
        regNum = Gia_ManRegNum(pGia) - 1;
    } else if (fileType == AIGER) {
        piNum  = Gia_ManPiNum(pGia);
        regNum = Gia_ManRegNum(pGia);
    }
    poNum  = Gia_ManPoNum(pGia);
    totNum = Gia_ManObjNum(pGia);

    _piList.resize(piNum);
    _riList.resize(regNum);
    _roList.resize(regNum);
    _poList.resize(poNum);
    _totGateList.resize(totNum);
}

// CirGate*
// CirMgr::createGate(const GateType& type){
//     // CirGate* gate = _totGateList[id];
//     CirGate* gate;
//     unsigned l = 0, id = getNumTots();
//     cirMgr->addTotGate(gate);
//     switch (type) {
//       case PI_GATE: gate = new CirPiGate(id, l); break;
//       case AIG_GATE: gate = new CirAigGate(id, l); break;
//       //case LATCH: gate = new CirRoGate(litId/2, lineNo+1); break;
//       default: cerr << "Error: Unknown gate type (" << type << ")!!\n";
//                exit(-1);
//    }
//    return gate;
// }

CirGate* CirMgr::createNotGate(CirGate* in0) {
    CirGate* notGate = new CirAigGate(getNumTots(), 0);
    addTotGate(notGate);
    notGate->setIn0(in0, true);
    notGate->setIn1(_const1, false);
    return notGate;
}

CirGate* CirMgr::createAndGate(CirGate* in0, CirGate* in1) {
    CirGate* andGate = new CirAigGate(getNumTots(), 0);
    addTotGate(andGate);
    andGate->setIn0(in0, false);
    andGate->setIn1(in1, false);
    return andGate;
}

CirGate* CirMgr::createOrGate(CirGate* in0, CirGate* in1) {
    CirGate* tmpGate = new CirAigGate(getNumTots(), 0);
    addTotGate(tmpGate);
    tmpGate->setIn0(in0, true);
    tmpGate->setIn1(in1, true);
    return createNotGate(tmpGate);
}

CirGate* CirMgr::createXorGate(CirGate* in0, CirGate* in1) {
    CirGate* tmpGate0 = new CirAigGate(getNumTots(), 0);
    addTotGate(tmpGate0);
    CirGate* tmpGate1 = new CirAigGate(getNumTots(), 0);
    addTotGate(tmpGate1);
    CirGate* tmpGate2 = new CirAigGate(getNumTots(), 0);
    addTotGate(tmpGate2);
    tmpGate0->setIn0(in0, true);
    tmpGate0->setIn1(in1, false);
    tmpGate1->setIn0(in0, false);
    tmpGate1->setIn1(in1, true);
    tmpGate2->setIn0(tmpGate0, true);
    tmpGate2->setIn1(tmpGate1, true);
    return createNotGate(tmpGate2);
}
