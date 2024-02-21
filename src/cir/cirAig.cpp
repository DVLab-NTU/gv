/****************************************************************************
  FileName     [ cirAig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define GV cir/aig functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cirGate.h>
#include <cirMgr.h>

#include <cassert>
#include <cstddef>
#include <map>

#include "abcMgr.h"
#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "yosysMgr.h"

/**
 * @brief Reads a circuit from the ABC Gia.
 *
 * @param fileName   The name of the file containing the circuit.
 * @param fileType   The type of file (AIGER or VERILOG) to be read.
 * @return           Returns true if the circuit is successfully read; otherwise, false.
 */
const bool CirMgr::readCirFromAbc(string fileName, CirFileType fileType) {
    ABCParam param;
    map<unsigned, string> id2Name;
    ifstream cirin(fileName);
    if (!cirin) {
        cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
        return false;
    }
    strcpy(param.pFileName, fileName.c_str());
    cout << "filename = " << fileName << endl;
    cirMgr->fileName = fileName;
    if (fileType == AIGER) {
        abcMgr->readAig(param);
    } else if (fileType == VERILOG) {
        param.fTechMap = 1;
        abcMgr->readVerilog(param);
        yosysMgr->createMapping(fileName);
        abcMgr->buildAigName(id2Name);
    }
    // initialize the size of the containers
    abcMgr->initCir(fileType);
    abcMgr->travPreprocess();
    abcMgr->travAllObj(fileType, id2Name);
    genDfsList();

    // DEBUG
    // IDMap aigIdMap;
    // reorderGateId(aigIdMap);
    // abcMgr->cirToAig(aigIdMap);
    // bool v = false;
    // abcMgr->runPDR(v);
    // END
    return true;
}

void 
CirMgr::initCir(Abc_Ntk_t* pNtk) {
    _piList.resize(Abc_NtkPiNum(pNtk));
    _riList.resize(Abc_NtkLatchNum(pNtk));
    _roList.resize(Abc_NtkLatchNum(pNtk));
    _poList.resize(Abc_NtkPoNum(pNtk));
    _totGateList.resize(Abc_NtkObjNum(pNtk));
}

void 
CirMgr::readCirFromAbcNtk(Abc_Ntk_t* pNtk) {
    // TODO : Convert abc ntk to gv aig ntk
    CirGateV gateV;
    // Abc_Ntk_t* pNtk = NULL;            // the gia pointer of abc
    Abc_Obj_t *pObj, *pObjRi, *pObjRo; // the obj element of gia
    unsigned iPi = 0, iPo = 0, iRi = 0, iRo = 0;
    int i;


    // initialize the size of the containers
    initCir(pNtk);
    // increment the global travel id for circuit traversing usage
    // Abc_NtkIncrementTravId(pNtk);

    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    // Abc_NodeSetTravIdCurrent(pGia, Gia_ManConst0(pGia));

    // set the const node
    _totGateList[0] = _const0;

    // traverse the obj's in topological order
    Abc_NtkForEachObj( pNtk, pObj, i) {
        char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
        if(Abc_ObjIsPi(pObj)) {
            CirPiGate* gate = new CirPiGate(Abc_ObjId(pObj), 0);
            _piList[iPi++] = gate;
            _totGateList[Abc_ObjId(pObj)] = gate;
            gate->setName(name);
        }
        else if(Abc_ObjIsPo(pObj)) {
        //  cout << "po is not in topological order, handle afterwards..." << endl;   
        }
        else if(Abc_ObjIsNode(pObj)) {
            CirAigGate *gate = new CirAigGate(Abc_ObjId(pObj), 0);
            _totGateList[Abc_ObjId(pObj)] = gate;
            gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), Abc_ObjFaninC0(pObj));
            gate->setIn1(getGate(Abc_ObjId(Abc_ObjFanin1(pObj))), Abc_ObjFaninC1(pObj));
            // char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
            gate->setName(name);
        }
        // TODO : handle latches

        // else if(Abc_ObjIsBo(pObj)) {
        //     CirRoGate* gate = new CirRoGate(Abc_ObjId(pObj), 0);
        //     _roList[iRo++] = gate;
        //     _totGateList[Abc_ObjId(pObj)] = gate;
        // }
        // else if(Abc_ObjIsBi(pObj)) {
        //     CirRiGate *gate = new CirRiGate(Abc_ObjId(pObj), 0, Abc_ObjId(Abc_ObjFanin0(pObj)));
        //     gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), Abc_ObjFaninC0(pObj));
        //     _riList[iPo++] = gate;
        //     _totGateList[Abc_ObjId(pObj)] = gate;
        // }
        else if(Abc_AigNodeIsConst(pObj)) {
            // cout << "I am const1 " << Abc_ObjId(pObj) <<  endl;
        }
        else {
            // cout << "not defined gate type" << endl;
        }
    }
    // handle the po's
    Abc_NtkForEachPo(pNtk, pObj, i) {
        char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
        CirPoGate *gate = new CirPoGate(Abc_ObjId(pObj), 0, Abc_ObjId(Abc_ObjFanin0(pObj)));
        gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), Abc_ObjFaninC0(pObj));
        _poList[iPo++] = gate;
        _totGateList[Abc_ObjId( pObj)] = gate;
        gate->setName(name);
    }
    

    cout << "generating dfs..." << endl;
    genDfsList();
    // checkFloatList();
    // checkUnusedList();

}

/**
 * @brief Reorder all the gates id for AIG.
 *
 * @param aigIdMap The mapping bewtween the old id and the new id.
 */
void CirMgr::reorderGateId(IDMap& aigIdMap) {
    unsigned nxtId = 1;
    for (int i = 0, n = cirMgr->getNumPIs(); i < n; ++i) {
        CirGate* gate   = cirMgr->getPi(i);
        unsigned gateId = gate->getGid();
        if (nxtId != gate->getGid()) {
            aigIdMap[gateId] = nxtId;
        }
        nxtId++;
    }
    for (int i = 0, n = cirMgr->getNumLATCHs(); i < n; ++i) {
        CirGate* gate   = cirMgr->getRo(i);
        unsigned gateId = gate->getGid();
        if (nxtId != gate->getGid()) {
            aigIdMap[gateId] = nxtId;
        }
        nxtId++;
    }
    for (int i = 0, n = cirMgr->getNumAIGs(); i < n; ++i) {
        CirGate* gate   = cirMgr->getAig(i);
        unsigned gateId = gate->getGid();
        if (!gate->isGlobalRef()) {
            assert(true);
            cout << "Redundant AIG Node !!\n";
        }
        if (nxtId != gate->getGid()) {
            aigIdMap[gateId] = nxtId;
        }
        nxtId++;
    }
    for (int i = 0, n = cirMgr->getNumPOs(); i < n; ++i) {
        CirGate* gate   = cirMgr->getPo(i);
        unsigned gateId = gate->getGid();
        if (nxtId != gate->getGid()) {
            aigIdMap[gateId] = nxtId;
        }
        nxtId++;
    }
}

/**
 * @brief Creates a NOT gate in the circuit.
 *
 * @param in0 The input gate for the NOT gate.
 * @return    Returns a pointer to the created NOT gate.
 */
CirGate* CirMgr::createNotGate(CirGate* in0) {
    CirGate* notGate = new CirAigGate(getNumTots(), 0);
    addTotGate(notGate);
    notGate->setIn0(in0, true);
    notGate->setIn1(_const1, false);
    return notGate;
}

/**
 * @brief  Creates an AND gate in the circuit.
 *
 * @param in0 The first input gate for the AND gate.
 * @param in1 The second input gate for the AND gate.
 * @return    Returns a pointer to the created AND gate.
 */
CirGate* CirMgr::createAndGate(CirGate* in0, CirGate* in1) {
    CirGate* andGate = new CirAigGate(getNumTots(), 0);
    addTotGate(andGate);
    andGate->setIn0(in0, false);
    andGate->setIn1(in1, false);
    return andGate;
}

/**
 * @brief Creates an OR gate in the circuit.
 *
 * @param in0 The first input gate for the OR gate.
 * @param in1 The second input gate for the OR gate.
 * @return    Returns a pointer to the created OR gate.
 */
CirGate* CirMgr::createOrGate(CirGate* in0, CirGate* in1) {
    CirGate* tmpGate = new CirAigGate(getNumTots(), 0);
    addTotGate(tmpGate);
    tmpGate->setIn0(in0, true);
    tmpGate->setIn1(in1, true);
    return createNotGate(tmpGate);
}

/**
 * @brief Creates an XOR gate in the circuit.
 *
 * @param in0 The first input gate for the XOR gate.
 * @param in1 The second input gate for the XOR gate.
 * @return    Returns a pointer to the created XOR gate.
 */
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
