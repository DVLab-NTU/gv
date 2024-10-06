/****************************************************************************
  FileName     [ satMgr.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "itpMgr.h"

#include <cassert>
#include <iostream>
#include <queue>
#include <vector>

#include "cirGate.h"
#include "cirMgr.h"
#include "gvMsg.h"
#include "minisat/reader.h"
#include "minisatMgr.h"
#include "satMgr.h"

using gv::cir::CirMgr;
using gv::sat::MinisatMgr;
using gv::sat::SatSolverMgr;
using namespace gv::cir;

namespace gv {
namespace itp {

void ItpMgr::verifyPropertyItp(const string &name, const CirGate *monitor) {
    // Initialize
    // duplicate the network, so you can modified
    // the ntk for the proving property without
    // destroying the original network
    _cirMgr  = new CirMgr();
    *_cirMgr = *cirMgr;
    SatProofRes pRes;
    /*MinisatMgr *gvSatSolver = new gv::sat::MinisatMgr(_cirMgr);*/
    gv::sat::SatSolverMgr *gvSatSolver = new MinisatMgr(_cirMgr);

    // Prove the monitor here!!
    pRes.setMaxDepth(1000);
    pRes.setSatSolver(gvSatSolver);
    itpUbmc(monitor, pRes);

    pRes.reportResult(name);
    if (pRes.isFired()) pRes.reportCex(monitor, _cirMgr);

    // delete gvSatSolver;
    // delete _cirMgr;
    reset();
}

void ItpMgr::verifyPropertyBmc(const string &name, const CirGate *monitor) {
    // Initialize
    // duplicate the network, so you can modified
    // the ntk for the proving property without
    // destroying the original network
    _cirMgr  = new CirMgr();
    *_cirMgr = *cirMgr;
    SatProofRes pRes;
    /*GVSatSolver *gvSatSolver = new GVSatSolver(_cirMgr);*/
    SatSolverMgr *gvSatSolver = new gv::sat::MinisatMgr(_cirMgr);

    // Prove the monitor here!!
    pRes.setMaxDepth(1000);
    pRes.setSatSolver(gvSatSolver);
    indBmc(monitor, pRes);

    pRes.reportResult(name);
    if (pRes.isFired()) pRes.reportCex(monitor, _cirMgr);

    // delete gvSatSolver;
    // delete _cirMgr;
    reset();
}

void ItpMgr::indBmc(const CirGate *monitor, SatProofRes &pRes) {
    SatSolverMgr *gvSatSolver = pRes.getSatSolver();
    bind(gvSatSolver);

    uint32_t i = 0;
    // GVNetId  I = buildInitState();
    CirGate *I = buildInitState();

    gvSatSolver->addBoundedVerifyData(I, i);
    gvSatSolver->assertProperty(I, false, i);
    // Start Bounded Model Checking
    for (uint32_t j = pRes.getMaxDepth(); i < j; ++i) {
        // Add time frame expanded circuit to SAT Solver
        gvSatSolver->addBoundedVerifyData(monitor, i);
        gvSatSolver->assumeRelease();
        gvSatSolver->assumeProperty(monitor, false, i);
        gvSatSolver->simplify();
        // Assumption Solver: If SAT, diproved!
        if (gvSatSolver->assump_solve()) {
            pRes.setFired(i);
            break;
        }
        gvSatSolver->assertProperty(monitor, true, i);
    }
}

void ItpMgr::itpUbmc(const CirGate *const monitor, SatProofRes &pRes) {
    SatSolverMgr *gvSatSolver = pRes.getSatSolver();
    bind(gvSatSolver);

    size_t num_clauses = getNumClauses();
    bool proved        = false;
    gvSatSolver->assumeRelease();
    // GVNetId S, R, R_prime, tmp1, tmp2, tmp3, tmp4;
    CirGate *S;
    CirGate *R;
    CirGate *R_prime;
    CirGate *tmp1;
    CirGate *tmp2;
    CirGate *tmp3;
    CirGate *tmp4;
    CirGate *tmp5;

    // TODO : finish your own Interpolation-based property checking

    // PART I:
    // Build Initial State

    // PART II:
    // Take care the first timeframe (i.e. Timeframe 0 )
    //    Check if monitor is violated at timeframe 0
    //    Build the whole timeframe 0 and map the var to latch net
    //    Mark the added clauses ( up to now ) to onset

    // PART III:
    // Start the ITP verification loop
    // Perform BMC
    //    SAT  -> cex found
    //    UNSAT-> start inner loop to compute the approx. images
    //    Each time the clauses are added to the solver,
    //    mark them to onset/offset carefully
    //    ( ex. addedBoundedVerifyData(), assertProperty() are called )
}

void ItpMgr::bind(SatSolverMgr *ptrMinisat) {
    _ptrMinisat = ptrMinisat;
    /*if (_ptrMinisat->getProof() == NULL) {*/
    if (_ptrMinisat->getProof() == NULL) {
        gvMsg(GV_MSG_ERR) << "The Solver has no Proof!! Try Declaring the Solver "
                             "with proofLog be set!!"
                          << endl;
        exit(0);
    }
}

void ItpMgr::reset() {
    _ptrMinisat = NULL;
    _varGroup.clear();
    _var2Net.clear();
    _isClauseOn.clear();
    _isClaOnDup.clear();
}

void ItpMgr::markOnsetClause(const ClauseId &cid) {
    unsigned cSize = getNumClauses();
    assert(cid < (int)cSize);
    if (_isClauseOn.size() < cSize) {
        _isClauseOn.resize(cSize, false);
    }
    _isClauseOn[cid] = true;
}

void ItpMgr::markOffsetClause(const ClauseId &cid) {
    unsigned cSize = getNumClauses();
    assert(cid < (int)cSize);
    if (_isClauseOn.size() < cSize) {
        _isClauseOn.resize(cSize, false);
    }
    _isClauseOn[cid] = false;
}

void ItpMgr::mapVar2Net(const Var &var, CirGate *net) { _var2Net[var] = net; }

CirGate *ItpMgr::getItp() const {
    assert(_ptrMinisat);
    // assert(_ptrMinisat->getProof());

    string proofName = "socv_proof.itp";
    // remove proof log if exist
    ifstream logFile(proofName.c_str());
    if (logFile.good()) {
        string rmCmd = "rm " + proofName + " -f";
        system(rmCmd.c_str());
    }

    // save proof log
    /*_ptrMinisat->getProof()->save(proofName.c_str());*/
    _ptrMinisat->getProof()->save(proofName.c_str());

    // bulding ITP
    // GVNetId netId = buildItp(proofName);
    CirGate *gateId = buildItp(proofName);

    // delete proof log
    unlink(proofName.c_str());

    return gateId;
}

vector<Clause> ItpMgr::getUNSATCore() const {
    assert(_ptrMinisat);
    // assert(_ptrMinisat->getProof());

    vector<Clause> unsatCore;
    unsatCore.clear();

    // save proof log
    string proofName = "socv_proof.itp";
    _ptrMinisat->getProof()->save(proofName.c_str());

    // generate unsat core
    Reader rdr;
    rdr.open(proofName.c_str());
    retrieveProof(rdr, unsatCore);

    // delete proof log
    unlink(proofName.c_str());

    return unsatCore;
}

void ItpMgr::retrieveProof(Reader &rdr, vector<Clause> &unsatCore) const {
    unsigned int tmp, cid, idx, tmp_cid;

    // Clear all
    vector<unsigned int> clausePos;
    clausePos.clear();
    unsatCore.clear();

    // Generate clausePos
    assert(!rdr.null());
    rdr.seek(0);
    for (unsigned int pos = 0; (tmp = rdr.get64()) != RDR_EOF; pos = rdr.Current_Pos()) {
        cid = clausePos.size();
        clausePos.push_back(pos);
        if ((tmp & 1) == 0) {  // root clause
            while ((tmp = rdr.get64()) != 0) {
            }
        } else {  // learnt clause
            idx = 0;
            while ((tmp = rdr.get64()) != 0) {
                idx = 1;
            }
            if (idx == 0) clausePos.pop_back();  // Clause Deleted
        }
    }

    // Generate unsatCore
    priority_queue<unsigned int> clause_queue;
    vector<bool> in_queue;
    in_queue.resize(clausePos.size());
    for (unsigned int i = 0; i < in_queue.size(); ++i) in_queue[i] = false;
    in_queue[in_queue.size() - 1] = true;
    clause_queue.push(clausePos.size() - 1);  // Push leaf (empty) clause
    while (clause_queue.size() != 0) {
        cid = clause_queue.top();
        clause_queue.pop();

        rdr.seek(clausePos[cid]);

        tmp = rdr.get64();
        if ((tmp & 1) == 0) {
            // root clause
            vec<Lit> lits;
            idx = tmp >> 1;
            lits.push(gvToLit(idx));
            while (_varGroup[idx >> 1] != COMMON) {
                tmp = rdr.get64();
                if (tmp == 0) break;
                idx += tmp;
                lits.push(gvToLit(idx));
            }
            unsatCore.push_back(Clause(false, lits));
        } else {
            // derived clause
            tmp_cid = cid - (tmp >> 1);
            if (!in_queue[tmp_cid]) {
                in_queue[tmp_cid] = true;
                clause_queue.push(tmp_cid);
            }
            while (1) {
                tmp = rdr.get64();
                if (tmp == 0) break;
                tmp_cid = cid - rdr.get64();
                if (!in_queue[tmp_cid]) {
                    in_queue[tmp_cid] = true;
                    clause_queue.push(tmp_cid);
                }
            }
        }
    }
}

void ItpMgr::retrieveProof(Reader &rdr, vector<unsigned int> &clausePos, vector<ClauseId> &usedClause) const {
    unsigned int tmp, cid, idx, tmp_cid, root_cid;

    // Clear all
    clausePos.clear();
    usedClause.clear();
    _varGroup.clear();
    /*_varGroup.resize(_ptrMinisat->_solver->nVars(), NONE);*/
    _varGroup.resize(_ptrMinisat->nVars(), NONE);
    _isClaOnDup.clear();
    assert((int)_isClauseOn.size() == getNumClauses());

    // Generate clausePos && varGroup
    assert(!rdr.null());
    rdr.seek(0);
    root_cid = 0;
    for (unsigned int pos = 0; (tmp = rdr.get64()) != RDR_EOF; pos = rdr.Current_Pos()) {
        cid = clausePos.size();
        clausePos.push_back(pos);
        if ((tmp & 1) == 0) {
            // Root Clause
            _isClaOnDup.push_back(_isClauseOn[root_cid]);
            idx = tmp >> 1;
            if (_isClauseOn[root_cid]) {
                if (_varGroup[idx >> 1] == NONE)
                    _varGroup[idx >> 1] = LOCAL_ON;
                else if (_varGroup[idx >> 1] == LOCAL_OFF)
                    _varGroup[idx >> 1] = COMMON;
            } else {
                if (_varGroup[idx >> 1] == NONE)
                    _varGroup[idx >> 1] = LOCAL_OFF;
                else if (_varGroup[idx >> 1] == LOCAL_ON)
                    _varGroup[idx >> 1] = COMMON;
            }
            while (1) {
                tmp = rdr.get64();
                if (tmp == 0) break;
                idx += tmp;
                if (_isClauseOn[root_cid]) {
                    if (_varGroup[idx >> 1] == NONE)
                        _varGroup[idx >> 1] = LOCAL_ON;
                    else if (_varGroup[idx >> 1] == LOCAL_OFF)
                        _varGroup[idx >> 1] = COMMON;
                } else {
                    if (_varGroup[idx >> 1] == NONE)
                        _varGroup[idx >> 1] = LOCAL_OFF;
                    else if (_varGroup[idx >> 1] == LOCAL_ON)
                        _varGroup[idx >> 1] = COMMON;
                }
            }
            ++root_cid;
        } else {
            _isClaOnDup.push_back(false);
            idx = 0;
            while (1) {
                tmp = rdr.get64();
                if (tmp == 0) break;
                idx = 1;
                tmp = rdr.get64();
            }
            if (idx == 0) {
                clausePos.pop_back();    // Clause Deleted
                _isClaOnDup.pop_back();  // Clause Deleted
            }
        }
    }

    // Generate usedClause
    priority_queue<unsigned int> clause_queue;
    vector<bool> in_queue;
    in_queue.resize(clausePos.size());
    for (unsigned int i = 0; i < in_queue.size(); ++i) in_queue[i] = false;
    in_queue[in_queue.size() - 1] = true;
    clause_queue.push(clausePos.size() - 1);  // Push root empty clause
    while (clause_queue.size() != 0) {
        cid = clause_queue.top();
        clause_queue.pop();

        rdr.seek(clausePos[cid]);

        tmp = rdr.get64();
        if ((tmp & 1) == 0) continue;  // root clause

        // else, derived clause
        tmp_cid = cid - (tmp >> 1);
        if (!in_queue[tmp_cid]) {
            in_queue[tmp_cid] = true;
            clause_queue.push(tmp_cid);
        }
        while (1) {
            tmp = rdr.get64();
            if (tmp == 0) break;
            tmp_cid = cid - rdr.get64();
            if (!in_queue[tmp_cid]) {
                in_queue[tmp_cid] = true;
                clause_queue.push(tmp_cid);
            }
        }
    }
    for (unsigned int i = 0; i < in_queue.size(); ++i) {
        if (in_queue[i]) {
            usedClause.push_back(i);
        }
    }
}

CirGate *ItpMgr::buildInitState() const {
    // TODO: build initial state
    CirAigGate *I;
    return I;
}

// build the McMillan Interpolant
CirGate *ItpMgr::buildItp(const string &proofName) const {
    Reader rdr;
    // records
    map<ClauseId, CirGate *> claItpLookup;
    vector<unsigned int> clausePos;
    vector<ClauseId> usedClause;
    // ntk
    uint32_t netSize = _cirMgr->getNumTots();
    // temperate variables
    CirGate *nId;
    CirGate *nId1;
    CirGate *nId2;
    int i, cid, tmp, idx, tmp_cid;
    // const 1 & const 0
    CirGate *CONST0 = _cirMgr->_const0;
    CirGate *CONST1 = _cirMgr->_const1;

    rdr.open(proofName.c_str());
    retrieveProof(rdr, clausePos, usedClause);

    for (i = 0; i < (int)usedClause.size(); i++) {
        cid = usedClause[i];
        rdr.seek(clausePos[cid]);
        tmp = rdr.get64();
        if ((tmp & 1) == 0) {
            // Root Clause
            if (_isClaOnDup[cid]) {
                idx = tmp >> 1;
                while (_varGroup[idx >> 1] != COMMON) {
                    tmp = rdr.get64();
                    if (tmp == 0) break;
                    idx += tmp;
                }
                if (_varGroup[idx >> 1] == COMMON) {
                    assert(_var2Net.find(idx >> 1) != _var2Net.end());
                    nId  = (_var2Net.find(idx >> 1))->second;
                    nId1 = (_var2Net.find(idx >> 1))->second;
                    if ((idx & 1) == 1) nId1 = _cirMgr->createNotGate(nId1);
                    if ((idx & 1) == 1) nId = _cirMgr->createNotGate(nId);
                    while (1) {
                        tmp = rdr.get64();
                        if (tmp == 0) break;
                        idx += tmp;
                        if (_varGroup[idx >> 1] == COMMON) {
                            assert(_var2Net.find(idx >> 1) != _var2Net.end());
                            nId2 = (_var2Net.find(idx >> 1))->second;
                            if ((idx & 1) == 1) nId2 = _cirMgr->createNotGate(nId2);
                            nId  = _cirMgr->createOrGate(nId1, nId2);
                            nId1 = nId;
                        }
                    }
                } else {
                    nId = CONST0;
                }
                claItpLookup[cid] = nId;
            } else {
                claItpLookup[cid] = CONST1;
            }
        } else {
            // Derived Clause
            tmp_cid = cid - (tmp >> 1);
            assert(claItpLookup.find(tmp_cid) != claItpLookup.end());
            nId  = (claItpLookup.find(tmp_cid))->second;
            nId1 = (claItpLookup.find(tmp_cid))->second;
            while (1) {
                idx = rdr.get64();
                if (idx == 0) break;
                idx--;
                // Var is idx
                tmp_cid = cid - rdr.get64();
                assert(claItpLookup.find(tmp_cid) != claItpLookup.end());
                nId2 = (claItpLookup.find(tmp_cid))->second;
                if (nId1 != nId2) {
                    if (_varGroup[idx] == LOCAL_ON) {  // Local to A. Build OR Gate.
                        if (nId1 == CONST1 || nId2 == CONST1) {
                            nId  = CONST1;
                            nId1 = nId;
                        } else if (nId1 == CONST0) {
                            nId  = nId2;
                            nId1 = nId;
                        } else if (nId2 == CONST0) {
                            nId  = nId1;
                            nId1 = nId;
                        } else {
                            // or
                            nId  = _cirMgr->createOrGate(nId1, nId2);
                            nId1 = nId;
                        }
                    } else {  // Build AND Gate.
                        if (nId1 == CONST0 || nId2 == CONST0) {
                            nId  = CONST0;
                            nId1 = nId;
                        } else if (nId1 == CONST1) {
                            nId  = nId2;
                            nId1 = nId;
                        } else if (nId2 == CONST1) {
                            nId  = nId1;
                            nId1 = nId;
                        } else {
                            // and
                            nId  = _cirMgr->createAndGate(nId1, nId2);
                            nId1 = nId;
                        }
                    }
                }
            }
            claItpLookup[cid] = nId;
        }
    }

    cid = usedClause[usedClause.size() - 1];
    nId = claItpLookup[cid];

    /*_ptrMinisat->resizeNtkData(_cirMgr->getNumTots() - netSize);  // resize Solver data to ntk size*/

    return nId;
}

void SatProofRes::reportResult(const string &name) const {
    // Report Verification Result
    cout << endl;
    if (isProved()) {
        cout << "Monitor \"" << name << "\" is safe." << endl;
    } else if (isFired()) {
        cout << "Monitor \"" << name << "\" is violated." << endl;
    } else {
        cout << "UNDECIDED at depth = " << _maxDepth << endl;
    }
}

void SatProofRes::reportCex(const CirGate *monitor, const CirMgr *const _cirMgr) const {
    assert(_satSolver != 0);

    // Output Pattern Value (PI + PIO)
    GVBitVecX dataValue;
    for (uint32_t i = 0; i <= _fired; ++i) {
        cout << i << ": ";
        for (int j = _cirMgr->getNumPIs() - 1; j >= 0; --j) {
            if (_satSolver->existVerifyData(_cirMgr->getPi(j), i)) {
                dataValue = _satSolver->getDataValue(_cirMgr->getPi(j), i);
                cout << dataValue[0];
            } else {
                cout << 'x';
            }
        }
        cout << endl;
        assert(_satSolver->existVerifyData(monitor, i));
    }
}

}  // namespace itp
}  // namespace gv
