/****************************************************************************
  FileName     [ satMgr.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "gvSatMgr.h"
#include "gvMsg.h"
#include "reader.h"
#include "cirMgr.h"
#include "cirGate.h"
#include <cassert>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

void
SATMgr::verifyPropertyItp(const string& name, const CirGate* monitor) {
    // Initialize
    // duplicate the network, so you can modified
    // the ntk for the proving property without
    // destroying the original network
    SatProofRes  pRes;
    GVSatSolver* gvSatSolver = new GVSatSolver(gvNtkMgr);

    // Prove the monitor here!!
    pRes.setMaxDepth(1000);
    pRes.setSatSolver(gvSatSolver);
    itpUbmc(monitor, pRes);

    pRes.reportResult(name);
    if (pRes.isFired()) pRes.reportCex(monitor, gvNtkMgr);

    //  delete gvSatSolver;
    //  delete _ntk;
    reset();
}

void
SATMgr::verifyPropertyBmc(const string& name, const CirGate* monitor) {
    // Initialize
    // duplicate the network, so you can modified
    // the ntk for the proving property without
    // destroying the original network
    SatProofRes  pRes;
    GVSatSolver* gvSatSolver = new GVSatSolver(gvNtkMgr);

    // Prove the monitor here!!
    pRes.setMaxDepth(1000);
    pRes.setSatSolver(gvSatSolver);
    indBmc(monitor, pRes);

    pRes.reportResult(name);
    if (pRes.isFired()) pRes.reportCex(monitor, gvNtkMgr);

    // delete gvSatSolver;
    // delete _ntk;
    reset();
}

void
SATMgr::indBmc(const CirGate* monitor, SatProofRes& pRes) {
    GVSatSolver* gvSatSolver = pRes.getSatSolver();
    bind(gvSatSolver);

    uint32_t i = 0;
    // GVNetId  I = buildInitState();
    CirGate* I = buildInitState();

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

void
SATMgr::itpUbmc(const CirGate* monitor, SatProofRes& pRes) {

    GVSatSolver* gvSatSolver = pRes.getSatSolver();
    bind(gvSatSolver);

    size_t num_clauses = getNumClauses();
    bool   proved      = false;
    gvSatSolver->assumeRelease();
    // GVNetId S, R, R_prime, tmp1, tmp2, tmp3, tmp4;
    CirGate* S;
    CirGate* R;
    CirGate* R_prime;
    CirGate* tmp1;
    CirGate* tmp2;
    CirGate* tmp3;
    CirGate* tmp4;
    CirGate* tmp5;

    // TODO : finish your own Interpolation-based property checking

    // PART I:
    // Build Initial State
    // GVNetId  I = buildInitState();
    CirGate* I = buildInitState();
    // PART II:
    // Take care the first timeframe (i.e. Timeframe 0 )
    //    Check if monitor is violated at timeframe 0
    //    Build the whole timeframe 0 and map the var to latch net
    //    Mark the added clauses ( up to now ) to onset
    uint32_t i = 0, k = 0;
    gvSatSolver->addBoundedVerifyData(I, i);
    gvSatSolver->assumeProperty(I, false, i);
    gvSatSolver->addBoundedVerifyData(monitor, i);
    gvSatSolver->assumeProperty(monitor, false, i);
    gvSatSolver->simplify();
    if (gvSatSolver->assump_solve()) {
        pRes.setFired(i);
        return;
    }

    num_clauses = getNumClauses();
    gvSatSolver->assertProperty(monitor, true, i);
    for (size_t j = 0; j < cirMgr->getNumLATCHs(); ++j) {
        gvSatSolver->addBoundedVerifyData(cirMgr->getRo(j), i);
        mapVar2Net(gvSatSolver->getVerifyData(cirMgr->getRo(j), i), cirMgr->getRo(j));
    }
    i++; // i = 1
    // map gvSatSolver vars to latch nets
    for (size_t j = 0; j < gvNtkMgr->getFFSize(); ++j) {
        gvSatSolver->addBoundedVerifyData(cirMgr->getRo(j), i);
        mapVar2Net(gvSatSolver->getVerifyData(cirMgr->getRo(j), i), cirMgr->getRo(j));
    }
    num_clauses = getNumClauses();

    // mark the onset clauses
    for (size_t j = 0; j < getNumClauses(); ++j) {
        markOnsetClause(j);
    }
    num_clauses = getNumClauses();

    for (size_t j = 0; j < gvNtkMgr->getFFSize(); ++j) {
        gvSatSolver->addBoundedVerifyData(cirMgr->getRo(j), i);
        mapVar2Net(gvSatSolver->getVerifyData(cirMgr->getRo(j), i), cirMgr->getRo(j));
    }
    num_clauses = getNumClauses();

    // PART III:
    // Start the ITP verification loop
    // Perform BMC
    //    SAT  -> cex found
    //    UNSAT-> start inner loop to compute the approx. images
    //    Each time the clauses are added to the solver,
    //    mark them to onset/offset carefully
    //    ( ex. addedBoundedVerifyData(), assertProperty() are called )
    for (; i < pRes.getMaxDepth(); ++i) {
        // perfrom BMC
        gvSatSolver->addBoundedVerifyData(monitor, i);
        gvSatSolver->assumeRelease();
        // gvSatSolver->assumeProperty(monitor, false, i);
        gvSatSolver->assumeProperty(monitor, false, i);
        gvSatSolver->assumeProperty(I, false, 0);
        gvSatSolver->simplify();

        if (gvSatSolver->assump_solve()) {
            pRes.setFired(i);
            break;
        }

        // mark the offset
        for (size_t j = num_clauses; j < getNumClauses(); ++j) {
            markOffsetClause(j);
        }
        num_clauses = getNumClauses();

        k = 0;
        R = I;
        S = getItp();
        for (; k < pRes.getMaxDepth(); ++k) {
            gvSatSolver->assumeRelease();
            gvSatSolver->addBoundedVerifyData(S, 0);
            gvSatSolver->assumeProperty(S, false, 0);
            for (size_t j = num_clauses; j < getNumClauses(); ++j) {
                markOnsetClause(j);
            }
            num_clauses = getNumClauses();

            gvSatSolver->assumeProperty(monitor, false, i);
            gvSatSolver->simplify();

            // Assumption Solver: If SAT, disproved!
            if (gvSatSolver->assump_solve()) {

                gvSatSolver->assertProperty(monitor, true, i);
                for (size_t j = num_clauses; j < getNumClauses(); ++j) {
                    markOffsetClause(j);
                }
                num_clauses = getNumClauses();
                break;
            } else num_clauses = getNumClauses();
            S = getItp();
            // see if Si or Ri-1 equals to Ri
            // tmp2    = ~gvNtkMgr->createNet();
            // tmp3    = ~gvNtkMgr->createNet();
            // tmp4    = ~gvNtkMgr->createNet();
            // R_prime = ~gvNtkMgr->createNet();
            tmp1    = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(tmp1);
            tmp2    = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(tmp2);
            tmp3    = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(tmp3);
            tmp4    = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(tmp4);
            tmp5    = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(tmp5);
            R_prime = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(R_prime);
            gvSatSolver->resizeNtkData(6);

            // gvNtkMgr->createGVAndGate(R_prime, ~R, ~S); // or(R, S)
            // Equal to ...
            tmp1->setIn0(R, true); tmp1->setIn1(S, true); 
            R_prime->setIn0(tmp1, true); R_prime->setIn1(cirMgr->_const0, true); 
            // gvNtkMgr->createGVAndGate(tmp2, ~R, R_prime);
            // Equal to ...
            tmp2->setIn0(R, true); tmp2->setIn1(R_prime,false);
            // gvNtkMgr->createGVAndGate(tmp3, R, ~R_prime);
            // Equal to ...
            tmp3->setIn0(R, false); tmp3->setIn1(R_prime, true);
            // gvNtkMgr->createGVAndGate(tmp4, tmp2, tmp3); // tmp4 = R xor R_prime
            // Equal to ...
            tmp4->setIn0(tmp2, true); tmp4->setIn1(tmp3, true);
            // INV
            tmp5->setIn0(tmp4, true); tmp5->setIn1(cirMgr->_const0, true);

            // gvSatSolver->addBoundedVerifyData(tmp4, 0);
            gvSatSolver->addBoundedVerifyData(tmp5, 0);
            gvSatSolver->assumeRelease();
            // gvSatSolver->assumeProperty(tmp4, false, 0); // assume R xor R_prime is true
            gvSatSolver->assumeProperty(tmp5, false, 0); // assume R xor R_prime is true
            gvSatSolver->simplify();
            if (!gvSatSolver->assump_solve()) // USAT
            {
                pRes.setProved(k);
                proved = true;
                break;
            } else {
                for (size_t j = num_clauses; j < getNumClauses(); ++j) {
                    markOnsetClause(j);
                }
                num_clauses = getNumClauses();
                R           = R_prime;
            }
        }
        if (proved) break;
    }
    return;
}

void
SATMgr::bind(GVSatSolver* ptrMinisat) {
    _ptrMinisat = ptrMinisat;
    if (_ptrMinisat->_solver->proof == NULL) {
        gvMsg(GV_MSG_ERR) << "The Solver has no Proof!! Try Declaring the Solver with proofLog be set!!" << endl;
        exit(0);
    }
}

void
SATMgr::reset() {
    _ptrMinisat = NULL;
    _varGroup.clear();
    _var2Net.clear();
    _isClauseOn.clear();
    _isClaOnDup.clear();
}

void
SATMgr::markOnsetClause(const ClauseId& cid) {
    unsigned cSize = getNumClauses();
    assert(cid < (int)cSize);
    if (_isClauseOn.size() < cSize) {
        _isClauseOn.resize(cSize, false);
    }
    _isClauseOn[cid] = true;
}

void
SATMgr::markOffsetClause(const ClauseId& cid) {
    unsigned cSize = getNumClauses();
    assert(cid < (int)cSize);
    if (_isClauseOn.size() < cSize) {
        _isClauseOn.resize(cSize, false);
    }
    _isClauseOn[cid] = false;
}

void
SATMgr::mapVar2Net(const Var& var, CirGate* net) {
    _var2Net[var] = net;
}

CirGate*
SATMgr::getItp() const {
    assert(_ptrMinisat);
    assert(_ptrMinisat->_solver->proof);

    string   proofName = "socv_proof.itp";
    // remove proof log if exist
    ifstream logFile(proofName.c_str());
    if (logFile.good()) {
        string rmCmd = "rm " + proofName + " -f";
        system(rmCmd.c_str());
    }

    // save proof log
    _ptrMinisat->_solver->proof->save(proofName.c_str());

    // bulding ITP
    // GVNetId netId = buildItp(proofName);
    CirGate* netId = buildItp(proofName);

    // delete proof log
    unlink(proofName.c_str());

    return netId;
}

vector<Clause>
SATMgr::getUNSATCore() const {
    assert(_ptrMinisat);
    assert(_ptrMinisat->_solver->proof);

    vector<Clause> unsatCore;
    unsatCore.clear();

    // save proof log
    string proofName = "socv_proof.itp";
    _ptrMinisat->_solver->proof->save(proofName.c_str());

    // generate unsat core
    Reader rdr;
    rdr.open(proofName.c_str());
    retrieveProof(rdr, unsatCore);

    // delete proof log
    unlink(proofName.c_str());

    return unsatCore;
}

void
SATMgr::retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const {
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
        if ((tmp & 1) == 0) { // root clause
            while ((tmp = rdr.get64()) != 0) {
            }
        } else { // learnt clause
            idx = 0;
            while ((tmp = rdr.get64()) != 0) {
                idx = 1;
            }
            if (idx == 0) clausePos.pop_back(); // Clause Deleted
        }
    }

    // Generate unsatCore
    priority_queue<unsigned int> clause_queue;
    vector<bool>                 in_queue;
    in_queue.resize(clausePos.size());
    for (unsigned int i = 0; i < in_queue.size(); ++i) in_queue[i] = false;
    in_queue[in_queue.size() - 1] = true;
    clause_queue.push(clausePos.size() - 1); // Push leaf (empty) clause
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

void
SATMgr::retrieveProof(Reader& rdr, vector<unsigned int>& clausePos, vector<ClauseId>& usedClause) const {
    unsigned int tmp, cid, idx, tmp_cid, root_cid;

    // Clear all
    clausePos.clear();
    usedClause.clear();
    _varGroup.clear();
    _varGroup.resize(_ptrMinisat->_solver->nVars(), NONE);
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
                if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_ON;
                else if (_varGroup[idx >> 1] == LOCAL_OFF) _varGroup[idx >> 1] = COMMON;
            } else {
                if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_OFF;
                else if (_varGroup[idx >> 1] == LOCAL_ON) _varGroup[idx >> 1] = COMMON;
            }
            while (1) {
                tmp = rdr.get64();
                if (tmp == 0) break;
                idx += tmp;
                if (_isClauseOn[root_cid]) {
                    if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_ON;
                    else if (_varGroup[idx >> 1] == LOCAL_OFF) _varGroup[idx >> 1] = COMMON;
                } else {
                    if (_varGroup[idx >> 1] == NONE) _varGroup[idx >> 1] = LOCAL_OFF;
                    else if (_varGroup[idx >> 1] == LOCAL_ON) _varGroup[idx >> 1] = COMMON;
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
                clausePos.pop_back();   // Clause Deleted
                _isClaOnDup.pop_back(); // Clause Deleted
            }
        }
    }

    // Generate usedClause
    priority_queue<unsigned int> clause_queue;
    vector<bool>                 in_queue;
    in_queue.resize(clausePos.size());
    for (unsigned int i = 0; i < in_queue.size(); ++i) in_queue[i] = false;
    in_queue[in_queue.size() - 1] = true;
    clause_queue.push(clausePos.size() - 1); // Push root empty clause
    while (clause_queue.size() != 0) {
        cid = clause_queue.top();
        clause_queue.pop();

        rdr.seek(clausePos[cid]);

        tmp = rdr.get64();
        if ((tmp & 1) == 0) continue; // root clause

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

CirGate*
SATMgr::buildInitState() const {
    // TODO: build initial state
    // GVNetId I, in1, in2;
    CirAigGate* I;
    CirAigGate* in1;
    CirGate* in2;
    // and const 1 with the first latch
    I   = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(I);
    in2 = new CirConstGate(cirMgr->_const0->getGid()); I->setIn0(cirMgr->_const0, true);
    // iteratively and adn all the PI's negation with otthers
    for (size_t i = 0; i < cirMgr->getNumLATCHs(); ++i) {
        I->setIn1(cirMgr->getRo(i), true);
        if(i + 1 != cirMgr->getNumLATCHs()){
            in1 = new CirAigGate(cirMgr->getNumTots(), 0);
            cirMgr->addTotGate(in1);
            in1->setIn0(I, false);
            I = in1;
        }
    }
    // resize solver data to ntk size
    // _ptrMinisat->resizeNtkData(gvNtkMgr->getFFSize());
    _ptrMinisat->resizeNtkData(cirMgr->getNumLATCHs());
    // GateList orderedGates;
    // CirGate::setGlobalRef();
    // I->genDfsList(orderedGates);
    return I;
}

// build the McMillan Interpolant
CirGate*
SATMgr::buildItp(const string& proofName) const {
    Reader                 rdr;
    // records
    // map<ClauseId, GVNetId> claItpLookup;
    map<ClauseId, CirGate*> claItpLookup;
    vector<unsigned int>   clausePos;
    vector<ClauseId>       usedClause;
    // ntk
    // uint32_t               netSize = gvNtkMgr->getNetSize();
    uint32_t               netSize = cirMgr->getNumTots();
    // temperate variables
    // GVNetId                nId, nId1, nId2;
    CirGate*               nId;
    CirGate*               nId1;
    CirGate*               nId2;
    int                    i, cid, tmp, idx, tmp_cid;
    // const 1 & const 0
    // GVNetId                CONST0, CONST1;
    // CONST0 = gvNtkMgr->getConst(0);
    // CONST1 = ~CONST0;
    CirGate* CONST0 = cirMgr->_const0;
    CirGate* CONST1 = new CirAigGate(cirMgr->getNumTots(),0);
    cirMgr->addTotGate(CONST1);
    CONST1->setIn0(cirMgr->_const0, true); CONST1->setIn1(cirMgr->_const0, true);

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
                    // Not Sure ...
                    if ((idx & 1) == 1) {
                        CirGate* tmpId = new CirAigGate(cirMgr->getNumTots(), 0);
                        cirMgr->addTotGate(tmpId);
                        tmpId->setIn0(nId1, true);
                        tmpId->setIn1(cirMgr->_const0, true);
                        nId1 = tmpId;
                        // nId1 = ~nId1;
                    }
                    if ((idx & 1) == 1) {
                        CirGate* tmpId = new CirAigGate(cirMgr->getNumTots(), 0);
                        cirMgr->addTotGate(tmpId);
                        tmpId->setIn0(nId, true);
                        tmpId->setIn1(cirMgr->_const0, true);
                        // nId = ~nId;
                        nId = tmpId;
                    }
                    while (1) {
                        tmp = rdr.get64();
                        if (tmp == 0) break;
                        idx += tmp;
                        if (_varGroup[idx >> 1] == COMMON) {
                            assert(_var2Net.find(idx >> 1) != _var2Net.end());
                            nId2 = (_var2Net.find(idx >> 1))->second;
                            // NOT SURE ...
                            if ((idx & 1) == 1) {
                                CirGate* tmpId = new CirAigGate(cirMgr->getNumTots(), 0);
                                cirMgr->addTotGate(tmpId);
                                tmpId->setIn0(nId2, true);
                                tmpId->setIn1(cirMgr->_const0, true);
                                // nId2 = ~nId2;
                                nId2 = tmpId;
                            }
                            // or
                            // nId = ~gvNtkMgr->createNet();
                            // gvNtkMgr->createGVAndGate(nId, ~nId1, ~nId2);
                            // nId1 = nId;
                            nId = new CirAigGate(cirMgr->getNumTots(), 0);
                            cirMgr->addTotGate(nId);
                            nId->setIn0(nId1, true);
                            nId->setIn1(nId2, true);
                            // not finish or gate !!
                            CirGate* tmpId = new CirAigGate(cirMgr->getNumTots(), 0);
                            cirMgr->addTotGate(tmpId);
                            tmpId->setIn0(nId, true);
                            tmpId->setIn1(cirMgr->_const0, true);
                            nId1 = tmpId;
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
                    if (_varGroup[idx] == LOCAL_ON) { // Local to A. Build OR Gate.
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
                            // nId = ~gvNtkMgr->createNet();
                            // gvNtkMgr->createGVAndGate(nId, ~nId1, ~nId2);
                            // nId1 = nId;
                            nId = new CirAigGate(cirMgr->getNumTots(), 0);
                            cirMgr->addTotGate(nId);
                            nId->setIn0(nId1, true);
                            nId->setIn1(nId2, true);
                            CirGate* tmpId = new CirAigGate(cirMgr->getNumTots(), 0);
                            cirMgr->addTotGate(tmpId);
                            tmpId->setIn0(nId, true);
                            tmpId->setIn1(cirMgr->_const0, true);
                            nId1 = tmpId;
                        }
                    } else { // Build AND Gate.
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
                            // nId = gvNtkMgr->createNet();
                            // gvNtkMgr->createGVAndGate(nId, nId1, nId2);
                            // nId1 = nId;
                            nId = new CirAigGate(cirMgr->getNumTots(), 0);
                            cirMgr->addTotGate(nId);
                            nId->setIn0(nId1, false);
                            nId->setIn1(nId2, false);
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

    // _ptrMinisat->resizeNtkData(gvNtkMgr->getNetSize() - netSize); // resize Solver data to ntk size
    _ptrMinisat->resizeNtkData(cirMgr->getNumTots() - netSize); // resize Solver data to ntk size

    return nId;
}

void
SatProofRes::reportResult(const string& name) const {
    // Report Verification Result
    gvMsg(GV_MSG_IFO) << endl;
    if (isProved()) {
        gvMsg(GV_MSG_IFO) << "Monitor \"" << name << "\" is safe." << endl;
    } else if (isFired()) {
        gvMsg(GV_MSG_IFO) << "Monitor \"" << name << "\" is violated." << endl;
    } else {
        gvMsg(GV_MSG_IFO) << "UNDECIDED at depth = " << _maxDepth << endl;
    }
}

void
SatProofRes::reportCex(const CirGate* monitor, const GVNtkMgr* const ntk) const {
    assert(_satSolver != 0);

    // Output Pattern Value (PI + PIO)
    GVBitVecX dataValue;
    for (uint32_t i = 0; i <= _fired; ++i) {
        gvMsg(GV_MSG_IFO) << i << ": ";
        // for (int j = ntk->getInoutSize() - 1; j >= 0; --j) {
        //     if (_satSolver->existVerifyData(ntk->getInout(j), i)) {
        //         dataValue = _satSolver->getDataValue(ntk->getInout(j), i);
        //         gvMsg(GV_MSG_IFO) << dataValue[0];
        //     } else {
        //         gvMsg(GV_MSG_IFO) << 'x';
        //     }
        // }
        for (int j = cirMgr->getNumPIs() - 1; j >= 0; --j) {
            if (_satSolver->existVerifyData(cirMgr->getPi(j), i)) {
                dataValue = _satSolver->getDataValue(cirMgr->getPi(j), i);
                gvMsg(GV_MSG_IFO) << dataValue[0];
            } else {
                gvMsg(GV_MSG_IFO) << 'x';
            }
        }
        gvMsg(GV_MSG_IFO) << endl;
        assert(_satSolver->existVerifyData(monitor, i));
    }
}
