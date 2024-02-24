
#include <cstring>

#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "fileType.h"

/**
 * @brief Decide whether the input is PI or RO
 *
 */
bool inputIsPi(const int& gateId) {
    return gateId <= cirMgr->getNumPIs();
}

/**
 * @brief Parse the primary input of Gia from the ABC
 *
 */
void parseInput(const int& idx, const int& gateId) {
    cirMgr->createInput(idx, gateId);
}

void parseOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, string poName) {
    cirMgr->createOutput(idx, gateId, in0Id, inv, poName);
}

int parseRo(const int& idx, const int& gateId, const FileType& fileType) {
    cirMgr->createRo(idx, gateId, fileType);
}

void parseRi(const int& idx, const int& gateId, const int& in0Id, const int& inv) {
    cirMgr->createRi(idx, gateId, in0Id, inv);
}

void parseRiRo(const int& riGid, const int& roGid) {
    cirMgr->createRiRo(riGid, roGid);
}

void parseAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv) {
    cirMgr->createAig(gateId, in0Id, in0Inv, in1Id, in1Inv);
}

void parseConst0() {
    cirMgr->createConst0();
}

void parseConst1() {
    cirMgr->createConst1();
}

void initCirMgr(const int& piNum, const int& poNum, const int& regNum, const int& totNum) {
    cirMgr->initCir(piNum, poNum, regNum, totNum);
}

unsigned getNumPIs() { return cirMgr->getNumPIs(); }
unsigned getNumPOs() { return cirMgr->getNumPOs(); }
unsigned getNumLATCHs() { return cirMgr->getNumLATCHs(); }
unsigned getNumAIGs() { return cirMgr->getNumAIGs(); }
unsigned getAigIn0Gid(const unsigned& idx) { return cirMgr->getAig(idx)->getIn0Gate()->getGid(); }
unsigned getAigIn1Gid(const unsigned& idx) { return cirMgr->getAig(idx)->getIn1Gate()->getGid(); }
unsigned getPoIn0Gid(const unsigned& idx) { return cirMgr->getPo(idx)->getIn0Gate()->getGid(); }
unsigned getRiIn0Gid(const unsigned& idx) { return cirMgr->getRi(idx)->getIn0Gate()->getGid(); }
int getAigIn0Cp(const unsigned& idx) { return cirMgr->getAig(idx)->getIn0().isInv(); }
int getAigIn1Cp(const unsigned& idx) { return cirMgr->getAig(idx)->getIn1().isInv(); }
int getPoIn0Cp(const unsigned& idx) { return cirMgr->getPo(idx)->getIn0().isInv(); }
int getRiIn0Cp(const unsigned& idx) { return cirMgr->getRi(idx)->getIn0().isInv(); }

void CirMgr::createInput(const int& idx, const int& gateId) {
    CirPiGate* gate      = new CirPiGate(gateId, 0);
    _piList[idx]         = gate;
    _totGateList[gateId] = gate;
}

void CirMgr::createOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, string poName) {
    CirPoGate* gate = new CirPoGate(gateId, 0, in0Id);
    char* n         = new char[poName.size() + 1];
    strcpy(n, poName.c_str());
    gate->setName(n);
    gate->setIn0(getGate(in0Id), inv);
    _poList[idx]         = gate;
    _totGateList[gateId] = gate;
}

int CirMgr::createRo(const int& idx, const int& gateId, const FileType& fileType) {
    if (fileType == VERILOG) {
        return _roList[idx]->getGid();
    } else if (fileType == AIGER) {
        CirRoGate* gate      = new CirRoGate(gateId, 0);
        _roList[idx]         = gate;
        _totGateList[gateId] = gate;
    }
    return 0;
}

void CirMgr::createRi(const int& idx, const int& gateId, const int& in0Id, const int& inv) {
    CirRiGate* gate = new CirRiGate(gateId, 0, in0Id);
    string str      = to_string(gateId) + "_ns";
    char* n         = new char[str.size() + 1];
    strcpy(n, str.c_str());
    gate->setName(n);
    gate->setIn0(getGate(in0Id), inv);
    _riList[idx]         = gate;
    _totGateList[gateId] = gate;
}

void CirMgr::createAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv) {
    CirAigGate* gate = new CirAigGate(gateId, 0);
    gate->setIn0(getGate(in0Id), in0Inv);
    gate->setIn1(getGate(in1Id), in1Inv);
    _totGateList[gateId] = gate;
    _aigList.push_back(gate);
}

void CirMgr::createRiRo(const int& riGid, const int& roGid) {
    CirGate* riGate   = getGate(riGid);
    CirRoGate* roGate = static_cast<CirRoGate*>(getGate(roGid));
    roGate->setIn0(riGate, false);
}

void CirMgr::createConst0() {
    // CONST0 Gate
    _totGateList[0] = CirMgr::_const0;
}

void CirMgr::createConst1() {
    // CONST1 Gate
    _const1 = new CirAigGate(getNumTots(), 0);
    addTotGate(_const1);
    _const1->setIn0(_const0, true);
    _const1->setIn1(_const0, true);
}

void CirMgr::initCir(const int& piNum, const int& poNum, const int& regNum, const int& totNum) {
    _piList.resize(piNum);
    _riList.resize(regNum);
    _roList.resize(regNum);
    _poList.resize(poNum);
    _totGateList.resize(totNum);
}