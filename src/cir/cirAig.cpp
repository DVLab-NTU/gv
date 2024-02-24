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
const bool CirMgr::readCirFromAbc(string fileName, FileType fileType) {
    ABCParam param;
    map<unsigned, string> id2Name;
    ifstream cirin(fileName);
    if (!cirin) {
        cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
        return false;
    }
    strcpy(param.pFileName, fileName.c_str());
    // cout << "filename = " << fileName << endl;
    cirMgr->fileName = fileName;
    if (fileType == AIGER) {
        abcMgr->readAig(param);
    } else if (fileType == VERILOG) {
        param.fTechMap = 1;
        param.fVerbose = 0;
        abcMgr->readVerilog(param);
        yosysMgr->createMapping(fileName);
        abcMgr->buildAigName(id2Name);
    }
    // initialize the size of the containers
    abcMgr->initCir(fileType);
    abcMgr->travPreprocess();
    abcMgr->travAllObj(fileType, id2Name);
    genDfsList();

    return true;
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
