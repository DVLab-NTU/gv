/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirGate.h"

#include <stdarg.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "base/abc/abc.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr* cirMgr;

unsigned CirGate::_globalRef_s = 0;

#if 1  // 1 for opt mode
#define NDEBUG
#endif

/***************************************/
/*   class CirGateV member functions   */
/***************************************/
unsigned CirGateV::gateId() const { return gate()->getGid(); }
unsigned CirGateV::litId() const {
    return (gate()->getGid() * 2 + (isInv() ? AIG_NEG_FANIN : 0));
}

ostream& operator<<(ostream& os, const CirGateV& gv) {
    if (gv.isFloat()) os << "*";
    if (gv.isInv()) os << "!";
    os << gv.gateId();
    return os;
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirPiGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid;
    if (_name) cout << " (" << _name << ")";
    cout << endl;
}

void CirRoGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid << " " << _in0;
    if (_name) cout << " (" << _name << ")";
    cout << endl;
}

void CirPoGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid << " " << _in0;
    if (_name) cout << " (" << _name << ")";
    cout << endl;
}

void CirRiGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid << " " << _in0;
    if (_name) cout << " (" << _name << ")";
    cout << endl;
}

void CirAigGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid << " " << _in0 << " "
         << _in1 << endl;
}

void CirConstGate::printGate() const {
    cout << setw(4) << left << getTypeStr() << _gid << endl;
}

void CirGate::reportGate() const {
    unsigned w = 80;
    for (unsigned i = 0; i < w; ++i) cout << '=';
    cout << endl;

    // Printing gate info
    cout << "= " << getTypeStr() << "(" << _gid << ")";
    if (getName()) cout << "\"" << getName() << "\"";
    cout << ", line " << _lineNo << endl;

    // Printing FEC Pairs
    // cout << "= FECs:";
    // if (isFEC()) {
    //    IdList *fecs = cirMgr->getFECGrps(_fecId/2);
    //    bool isInv = _fecId & 1;
    //    for (size_t i = 0, n = fecs->size(); i < n; ++i) {
    //       CirGate *g = cirMgr->litId2Gate((*fecs)[i]);
    //       if (g == this) continue;
    //       cout << ((isInv ^ ((*fecs)[i]%2 != 0))?" !":" ") << g->getGid();
    //    }
    // }
    // cout << endl;

    // Printing sim value
    // cout << "= Value: " << _pValue << endl;

    for (unsigned i = 0; i < w; ++i) cout << '=';
    cout << endl;
}

void CirGate::reportFanin(int level) const {
    assert(level >= 0);
    setGlobalRef();
    reportFaninRecur(level, 0, false);
}

void CirGate::reportFaninRecur(int level, int repLevel, bool isInv) const {
    if (repLevel > 0) cout << setw(repLevel * 2) << ' ';
    cout << (isInv ? "!" : "") << getTypeStr() << " " << _gid;
#ifndef NDEBUG
    cout << " " << (_pValue() & size_t(0x1));
#endif
    if (level == repLevel) {
        cout << endl;
        return;
    }
    assert(level > repLevel);
    if (isGlobalRef() && getNumFanins() != 0) {
        cout << " (*)" << endl;
        return;
    }
    cout << endl;
    setToGlobalRef();
    CirGateV in0 = getIn0();
    if (in0.gate())
        in0.gate()->reportFaninRecur(level, repLevel + 1, in0.isInv());
    CirGateV in1 = getIn1();
    if (in1.gate())
        in1.gate()->reportFaninRecur(level, repLevel + 1, in1.isInv());
}

void CirGate::reportFanout(int level) const {
    assert(level >= 0);
    setGlobalRef();
    reportFanoutRecur(level, 0, false);
}

void CirGate::reportFanoutRecur(int level, int repLevel, bool isInv) const {
    if (repLevel > 0) cout << setw(repLevel * 2) << ' ';
    cout << (isInv ? "!" : "") << getTypeStr() << " " << _gid;
    if (level == repLevel) {
        cout << endl;
        return;
    }
    assert(level > repLevel);
    GateList& fanouts = cirMgr->getFanouts(_gid);
    size_t nFanouts   = fanouts.size();
    if (isGlobalRef() && nFanouts != 0) {
        cout << " (*)" << endl;
        return;
    }
    cout << endl;
    setToGlobalRef();
    for (size_t i = 0; i < nFanouts; ++i) {
        CirGate* fanout = fanouts[i];
        CirGateV in0    = fanout->getIn0();
        assert(in0.gate() != 0);
        bool fanoutInv = false;
        if (in0.gate() == this)
            fanoutInv = in0.isInv();
        else {
            CirGateV in1 = fanout->getIn1();
            assert(in1.gate() != 0);
            assert(in1.gate() == this);
            fanoutInv = in1.isInv();
        }
        fanouts[i]->reportFanoutRecur(level, repLevel + 1, fanoutInv);
    }
}

// set in0 by the cirgate
void CirPoGate::setIn0(CirGate* faninGate, bool inv) {
    CirGateV gateV = size_t(faninGate);
    gateV.setInv(inv);
    setIn0(gateV());
}

void CirRoGate::setIn0(CirGate* faninGate, bool inv) {
    CirGateV gateV = size_t(faninGate);
    gateV.setInv(inv);
    setIn0(gateV());
}

void CirRiGate::setIn0(CirGate* faninGate, bool inv) {
    CirGateV gateV = size_t(faninGate);
    gateV.setInv(inv);
    setIn0(gateV());
}

void CirAigGate::setIn0(CirGate* faninGate, bool inv) {
    CirGateV gateV = size_t(faninGate);
    gateV.setInv(inv);
    setIn0(gateV());
}

void CirAigGate::setIn1(CirGate* faninGate, bool inv) {
    CirGateV gateV = size_t(faninGate);
    gateV.setInv(inv);
    setIn1(gateV());
}
