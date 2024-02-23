// /****************************************************************************
//   FileName     [ cirSim.cpp ]
//   PackageName  [ cir ]
//   Synopsis     [ Define cir simulation functions ]
//   Author       [ Chung-Yang (Ric) Huang ]
//   Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
// ****************************************************************************/

#ifndef CIR_SIM_CPP
#define CIR_SIM_CPP

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

static bool debug_g = false;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static inline void checkFecHash(FECHash &fecHash, CirGate *g) {
    IdList *ll   = 0;
    CirPValue pv = g->getPValue();
    //    if (fecHash.check(pv, ll)) {
    //       ll->push_back(g->getGid() * 2);
    //    }
    //    else if (fecHash.check(~pv, ll)) {
    //       ll->push_back(g->getGid() *2 + 1);
    //    }
    //    else {
    //       ll = new IdList(1);
    //       (*ll)[0] = g->getGid() * 2;
    //       fecHash.forceInsert(pv, ll);
    //    }
}

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void CirMgr::randomSim() {
    size_t nSims = 0, nFails = 0;
    size_t MAX_FAILS = 3;  // 3;
    unsigned nin     = getNumPIs();
    if (nin < 6) nin = 0;
    else nin -= 6;  // 2^6 = 64
    double m = MAX_FAILS;
    while (nin >>= 1) m *= 1.5;  // 1.4; // 1.3; // 1.2; // 1.33;
    unsigned ntot = _dfsList.size();
    while (ntot >>= 2) m *= 1.4;  // 1.4; // 1.3; // 1.13;
    size_t nfv = _fecVector.size();
    m          = (m + nfv) / (nfv + 1);
    MAX_FAILS  = unsigned(m);
    if (debug_g)
        cout << "MAX_FAILS = " << MAX_FAILS << endl;
    cout << "Total #FEC Group = " << _fecGrps.size() << flush;
    for (size_t i = 0; i < nfv; ++i) {
        setPPattern(_fecVector[i]);
        if (!simAndCheckFEC()) ++nFails;
        nSims += 64;
        if (_simLog) outputSimLog();
        cout << char(13) << setw(30) << ' ' << char(13)
             << "Total #FEC Group = " << _fecGrps.size() << flush;
    }
    while (nFails < MAX_FAILS) {
        setRandPPattern();
        if (!simAndCheckFEC()) ++nFails;
        nSims += 64;
        if (_simLog) outputSimLog();
        cout << char(13) << setw(30) << ' ' << char(13)
             << "Total #FEC Group = " << _fecGrps.size() << flush;
    }
    finalizeFEC();
    cout << char(13) << setw(30) << ' ' << char(13) << nSims
         << " patterns simulated." << endl;
    //   _fecMgr.print();
}

void CirMgr::fileSim(ifstream &patternFile) {
    unsigned nSims = 0, nin = getNumPIs();
    while (!patternFile.eof()) {
        size_t patterns[nin];
        unsigned nPatterns = gatherPatterns(patternFile, patterns, nin);
        if (!nPatterns) break;
        setPPattern(patterns);
        simAndCheckFEC();
        nSims += nPatterns;
        if (_simLog) outputSimLog(nPatterns);
        cout << char(13) << "Total #FEC Group = " << _fecGrps.size() << flush;
    }
    finalizeFEC();
    cout << char(13) << nSims << " patterns simulated." << endl;
}

// return true if new FEC pairs are generated
bool CirMgr::simAndCheckFEC() {
    pSim1Pattern();
    if (isFlag(NO_FEC)) return false;
    if (_fecGrps.empty()) return initFEC();
    else return checkFEC();
}

// Make sure the circuit has been simulated!!
bool CirMgr::initFEC() {
    FECHash fecHash;
    fecHash.init(getHashSize(_dfsList.size()));
    checkFecHash(fecHash, _const0);  // make sure 0 is in HashMap
    for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
        CirGate *g = _dfsList[i];
        if (g->isAig()) checkFecHash(fecHash, g);
    }

    // Collect valid FEC groups
    for (FECHash::iterator hi = fecHash.begin(); hi != fecHash.end(); ++hi) {
        IdList *ll = (*hi).second;
        if (ll->size() == 1) {
            delete ll;
            continue;
        }
        _fecGrps.push_back(ll);
    }
    if (_fecGrps.empty()) {
        setFlag(NO_FEC);
        return false;
    }
    return true;
}

// return true if _fecGrps changes!!
bool CirMgr::checkFEC() {
    vector<IdList *> newGrps;
    bool grpChange = false;
    for (size_t i = 0, n = _fecGrps.size(); i < n; ++i) {
        IdList *ll = _fecGrps[i];
        grpChange |= checkFECRecur(*ll, newGrps);
    }
    _fecGrps.swap(newGrps);
    for (size_t i = 0, n = newGrps.size(); i < n; ++i)
        delete newGrps[i];
    if (_fecGrps.empty()) setFlag(NO_FEC);
    return grpChange;
}

bool CirMgr::checkFECRecur(const IdList &ll, vector<IdList *> &newGrps) {
    FECHash fecHash;
    size_t n = ll.size();  // Note: n can be 0
    fecHash.init(getHashSize(n));
    for (size_t i = 0; i < n; ++i) {
        CirGate *g = litId2Gate(ll[i]);
        checkFecHash(fecHash, g);
    }

    // Collect valid FEC groups
    size_t nNewGrps = 0;
    for (FECHash::iterator hi = fecHash.begin(); hi != fecHash.end(); ++hi) {
        IdList *nn = (*hi).second;
        if (nn->size() == 1) {
            delete nn;
            continue;
        }
        newGrps.push_back(nn);
        ++nNewGrps;
    }
    return (nNewGrps != 1);
}

void CirMgr::finalizeFEC() {
    CirGate::setGlobalRef();
    for (size_t i = 0, n = _fecGrps.size(); i < n; ++i) {
        IdList *ll = _fecGrps[i];
        sort(ll->begin(), ll->end());
        if ((*ll)[0] % 2 != 0)  // isInv
            for (size_t j = 0, m = ll->size(); j < m; ++j) (*ll)[j] ^= 1;
        for (size_t j = 0, m = ll->size(); j < m; ++j) {
            CirGate *g = litId2Gate((*ll)[j]);
            g->setToGlobalRef();
            g->setFECId(i * 2 + (*ll)[j] % 2);
        }
    }
    for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
        CirGate *g = _dfsList[i];
        if (!g->isGlobalRef())
            g->resetFECId();
    }
    // Check _const0 as it is NOT in the DFS list
    if (!_const0->isGlobalRef())
        _const0->resetFECId();
}

void CirMgr::simplifyFECGrps() {
    vector<IdList *> newGrps;
    for (size_t i = 0, n = _fecGrps.size(); i < n; ++i) {
        IdList *ll        = _fecGrps[i];
        size_t validGates = 0;
        for (size_t j = 0, m = ll->size(); j < m; ++j)
            if (litId2Gate((*ll)[j]) != 0) ++validGates;
        if (validGates > 1) {
            IdList *nn     = new IdList(validGates);
            unsigned fecId = newGrps.size() * 2;
            for (size_t j = 0, k = 0, m = ll->size(); j < m; ++j) {
                CirGate *g = litId2Gate((*ll)[j]);
                if (g != 0) {
                    (*nn)[k++] = (*ll)[j];
                    g->setFECId(fecId + ((*ll)[j] & 0x1));
                }
            }
            newGrps.push_back(nn);
        } else if (validGates == 1)
            for (size_t j = 0, m = ll->size(); j < m; ++j) {
                CirGate *g = litId2Gate((*ll)[j]);
                if (g != 0) g->resetFECId();
            }
    }
    newGrps.swap(_fecGrps);
    for (size_t i = 0, n = newGrps.size(); i < n; ++i) delete newGrps[i];
}

void CirMgr::clearFECGrps() {
    for (size_t i = 0, n = _fecGrps.size(); i < n; ++i) delete _fecGrps[i];
    for (size_t i = 0, n = getNumTots(); i < n; ++i)
        if (_totGateList[i]) _totGateList[i]->resetFECId();
    clearList(_fecGrps);
}

void CirMgr::printFEC() const {
    size_t n = _fecGrps.size();
    cout << "Total #FEC Group = " << n << endl;
    for (size_t i = 0; i < n; ++i) {
        IdList *ll = _fecGrps[i];
        cout << "[" << i << "] " << litId2Gate((*ll)[0])->getPValue() << endl;
        for (size_t j = 0, m = ll->size(); j < m; ++j)
            cout << litId2GateV((*ll)[j]) << endl;
    }
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::setRandPPattern() const {
    for (unsigned i = 0, n = getNumPIs(); i < n; ++i)
        _piList[i]->setRandPPattern();
}

void CirMgr::setPPattern(SimPattern const patterns) const {
    for (size_t i = 0, n = getNumPIs(); i < n; ++i)
        _piList[i]->setPValue(patterns[i]);
}

// return false if:
// (1) Pattern error ==> nPatterns will reset to 0
// (2) EOF is encountered
unsigned CirMgr::gatherPatterns(ifstream &patternFile, SimPattern const patterns, size_t nin) {
    unsigned nPatterns = 0;
    for (size_t i = 0; i < nin; ++i) patterns[i] = size_t(0);
    string str;
    while (nPatterns < 64) {
        patternFile >> str;
        if (patternFile.eof()) return nPatterns;
        if (nin != str.size()) {
            cerr << "\nError: Pattern(" << str << ") length(" << str.size()
                 << ") does not match the number of inputs(" << nin
                 << ") in a circuit!!" << endl;
            return 0;
        }
        for (size_t i = 0; i < nin; ++i) {
            if (str[i] == '1')
                patterns[i] += (size_t(1) << nPatterns);
            else if (str[i] != '0') {
                cerr << "\nError: Pattern(" << str << ") contains a non-0/1 "
                     << "character(\'" << str[i] << "\')." << endl;
                return 0;
            }
        }
        ++nPatterns;
    }
    assert(nPatterns == 64);
    return nPatterns;
}

void CirMgr::pSim1Pattern() const {
    for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
        cout << _dfsList[i]->getTypeStr() << "\n";
        if (_dfsList[i]->getTypeStr() == "RI") {
            cout << endl;
        }
        _dfsList[i]->pSim();
    }
}

void CirMgr::outputSimLog(size_t nPatterns) {
    assert(_simLog != 0);
    unsigned nin  = getNumPIs();
    unsigned nout = getNumPOs();
    size_t patterns[nin], responses[nout];
    for (unsigned i = 0; i < nin; ++i)
        patterns[i] = _piList[i]->getPValue()();
    for (unsigned i = 0; i < nout; ++i)
        responses[i] = _poList[i]->getPValue()();

    assert(nPatterns <= 64);
    for (size_t i = 0; i < nPatterns; ++i) {
        for (unsigned j = 0; j < nin; ++j) {
            (*_simLog) << (patterns[j] & 0x1);
            patterns[j] >>= 1;
        }
        (*_simLog) << " ";
        for (unsigned j = 0; j < nout; ++j) {
            (*_simLog) << (responses[j] & 0x1);
            responses[j] >>= 1;
        }
        (*_simLog) << endl;
    }
}

void CirPValue::rand() {
    _pValue = size_t(0);
    for (int i = 0; i < 4; ++i)
        _pValue += size_t(rnGen(1 << 16) << i * 16);
}

static const char *pvBin[16] = {
    "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};

ostream &operator<<(ostream &os, const CirPValue &v) {
    const size_t nBinMinus1 = 15, binShift = nBinMinus1 * 4;
    os << pvBin[v._pValue >> binShift] << pvBin[v._pValue << 4 >> binShift];
    for (size_t i = 2; i <= nBinMinus1; i += 2)
        os << '_' << pvBin[v._pValue << (i * 4) >> binShift]
           << pvBin[v._pValue << ((i + 1) * 4) >> binShift];
    return os;
}

#endif
