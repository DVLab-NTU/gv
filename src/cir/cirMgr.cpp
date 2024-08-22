/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirMgr.h"

#include <ctype.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "cirDef.h"
#include "cirGate.h"
#include "fmt/core.h"
#include "gvType.h"
#include "util.h"
#include "yosysMgr.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr           = 0;
CirGate* CirMgr::_const0 = new CirConstGate(0);

bool CirMgr::readCircuitNew() {
    map<unsigned, string> id2Name;
    ABCParams params;
    strcpy(params.pFileName, _fileName.c_str());
    if (_fileType == AIGER) {
        _ysyMgr->readAiger(_fileName);
        _abcMgr->readAiger(params);
    } else if (_fileType == BLIF) {
        _ysyMgr->readBlif(_fileName);
    } else if (_fileType == VERILOG) {
        params.fTechMap = 1;
        params.fVerbose = 0;
        // _ysyMgr->setLogging(true);
        _ysyMgr->readVerilog(_fileName);
        _ysyMgr->createMapping(_fileName);
        // _abcMgr->readSeqVerilog(params);
        _abcMgr->readVerilogNew(params);
        _abcMgr->buildAigName(id2Name);
        _abcMgr->initCir(_fileType);
        _abcMgr->travPreprocess();
        _abcMgr->giaToCir(_fileType, id2Name);
        genDfsList();
    }
    return true;
}
bool CirSeq::readCircuit() {
    map<unsigned, string> id2Name;
    ABCParams params;
    strcpy(params.pFileName, _fileName.c_str());
    if (_fileType == AIGER) {
        _ysyMgr->readAiger(_fileName);
        _abcMgr->readAiger(params);
    } else if (_fileType == BLIF) {
        _ysyMgr->readBlif(_fileName);
    } else if (_fileType == VERILOG) {
        params.fTechMap = 1;
        params.fVerbose = 0;
        // _ysyMgr->setLogging(true);
        _ysyMgr->readVerilog(_fileName);
        _ysyMgr->createMapping(_fileName);
        // _abcMgr->readSeqVerilog(params);
        _abcMgr->readVerilogNew(params);
        _abcMgr->buildAigName(id2Name);
        _abcMgr->initCir(_fileType);
        _abcMgr->travPreprocess();
        _abcMgr->giaToCir(_fileType, id2Name);
        genDfsList();
    }
    return true;
}

bool CirComb::readCircuit() {
    // ABCParams params;
    // strcpy(params.pFileName, _fileName.c_str());
    // _abcMgr->readCombVerilog(params);
    _ysyMgr->readVerilog(_fileName);
    fmt::print("REG NUM: {0}", _ysyMgr->getNumREGs());
    return true;
}

void CirMgr::deleteCircuit() {
    _piList.clear();
    _poList.clear();
    _roList.clear();
    _riList.clear();
    _aigList.clear();
    _dfsList.clear();
    _totGateList.clear();
    // clearList<GateVec>(_dfsList);
    // clearList<GateVec>(_totGateList);
    if (_fanoutInfo) {
        delete[] _fanoutInfo;
        _fanoutInfo = 0;
    }
    delete _abcMgr;
    delete _ysyMgr;
    // delete _const1;
    //   delete _const0;
    //   _const0 = new CirConstGate(0);
}

void CirMgr::genDfsList() {
    clearList(_dfsList);
    CirGate::setGlobalRef();
    for (unsigned i = 0, n = getNumPOs(); i < n; ++i)
        getPo(i)->genDfsList(_dfsList);

    for (unsigned i = 0, n = getNumLATCHs(); i < n; ++i)
        getRi(i)->genDfsList(_dfsList);
}

void CirPiGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    gateList.push_back(this);
}

void CirRoGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    gateList.push_back(this);
}

void CirPoGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    CirGate* g = _in0.gate();
    if (!g->isGlobalRef())
        g->genDfsList(gateList);
    gateList.push_back(this);
}

void CirRiGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    CirGate* g = _in0.gate();
    if (!g->isGlobalRef())
        g->genDfsList(gateList);
    gateList.push_back(this);
}

void CirAigGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    CirGate* g = _in0.gate();
    // cout << g->getGid()<<endl;
    if (!g->isGlobalRef())
        g->genDfsList(gateList);
    g = _in1.gate();
    // cout << g->getGid()<<endl;
    if (!g->isGlobalRef())
        g->genDfsList(gateList);
    gateList.push_back(this);
}

void CirConstGate::genDfsList(GateVec& gateList) {
    setToGlobalRef();
    gateList.push_back(this);
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
    cout << endl;
    cout << "Circuit Statistics" << endl
         << "==================" << endl;
    unsigned tot = _totGateList.size();
    unsigned sum = 0;
    sum += _piList.size();
    cout << "  " << setw(7) << left << "PI"
         << setw(7) << right << _piList.size() << endl;
    sum += _poList.size();
    cout << "  " << setw(7) << left << "PO"
         << setw(7) << right << _poList.size() << endl;
    sum += _roList.size();
    cout << "  " << setw(7) << left << "LATCH"
         << setw(7) << right << _roList.size() << endl;
    // tot += _numDecl[AIG];
    cout << "  " << setw(7) << left << "AIG"
         << setw(7) << right << tot - sum << endl;
    cout << "------------------" << endl;
    cout << "  Total  " << setw(7) << right << tot << endl;
}

void CirMgr::printNetlist() const {
    cout << endl;
    for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
        // GateType type = _dfsList[i]->getType();
        // if (!(type == RO_GATE) && !(type == RI_GATE))
        //     continue;
        cout << "[" << i << "] ";
        _dfsList[i]->printGate();
    }
}

void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    for (unsigned i = 0, n = getNumPIs(); i < n; ++i)
        cout << " " << getPi(i)->getGid();
    cout << endl;
}

void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    for (unsigned i = 0, n = getNumPOs(); i < n; ++i)
        cout << " " << getPo(i)->getGid();
    cout << endl;
}

void CirMgr::writeAag(ostream& outfile) const {
    size_t nAig = 0;
    for (size_t i = 0, n = _dfsList.size(); i < n; ++i)
        if (_dfsList[i]->isAig()) ++nAig;
    outfile << "aag " << _numDecl[VARS] + nAig << " " << _numDecl[PI] << " "
            << _numDecl[LATCH] << " " << _numDecl[PO] << " "
            << nAig << endl;
    for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
        outfile << (getPi(i)->getGid() * 2) << endl;

    // Update Latch for sequential circuit.
    for (size_t i = 0, n = _numDecl[LATCH]; i < n; ++i)
        outfile << getRo(i)->getGid() * 2 << " " << getRi(i)->getIn0().litId() << endl;

    for (size_t i = 0, n = _numDecl[PO]; i < n; ++i)
        outfile << getPo(i)->getIn0().litId() << endl;
    for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
        CirGate* g = _dfsList[i];
        if (!g->isAig()) continue;
        outfile << g->getGid() * 2 << " " << g->getIn0().litId() << " "
                << g->getIn1().litId() << endl;
    }
    for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
        if (getPi(i)->getName())
            outfile << "i" << i << " " << getPi(i)->getName() << endl;
    for (size_t i = 0, n = _numDecl[PO]; i < n; ++i)
        if (getPo(i)->getName())
            outfile << "o" << i << " " << getPo(i)->getName() << endl;
    outfile << "c" << endl;
    outfile << "AAG output by Chung-Yang (Ric) Huang" << endl;
}

void CirMgr::writeBlif(const string& fileName) const {
    _ysyMgr->writeBlif(fileName);
}

void CirMgr::writeGate(ostream& outfile, CirGate* g) const {
    GateVec faninCone;
    CirGate::setGlobalRef();
    g->genDfsList(faninCone);

    size_t nAig = 0, npi = 0;
    unsigned maxId = 0;
    for (size_t i = 0, n = faninCone.size(); i < n; ++i) {
        if (faninCone[i]->isAig()) ++nAig;
        else if (faninCone[i]->isPi()) ++npi;
        if (faninCone[i]->getGid() > maxId) maxId = faninCone[i]->getGid();
    }
    outfile << "aag " << maxId << " " << npi << " 0 1 " << nAig << endl;
    // for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
    for (size_t i = 0, n = cirMgr->getNumPIs(); i < n; ++i)
        if (getPi(i)->isGlobalRef())
            outfile << (getPi(i)->getGid() * 2) << endl;
    outfile << g->getGid() * 2 << endl;
    for (size_t i = 0, n = faninCone.size(); i < n; ++i) {
        CirGate* f = faninCone[i];
        if (!f->isAig()) continue;
        outfile << f->getGid() * 2 << " " << f->getIn0().litId() << " "
                << f->getIn1().litId() << endl;
    }
    for (size_t i = 0, n = cirMgr->getNumPIs(), j = 0; i < n; ++i)
        if (getPi(i)->isGlobalRef() && getPi(i)->getName())
            outfile << "i" << j++ << " " << getPi(i)->getName() << endl;
    outfile << "o0 " << g->getGid() << endl;
    outfile << "c" << endl;
    outfile << "Write gate (" << g->getGid()
            << ") by Chung-Yang (Ric) Huang" << endl;
}

const bool CirMgr::readBlif(const string& fileName) const {
    _ysyMgr->init();
    _ysyMgr->readBlif(fileName);
}
