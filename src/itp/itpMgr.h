/****************************************************************************
  FileName     [ itpMgr.h ]
  PackageName  [ itp ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/
#pragma once

#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <map>
#include <vector>

#include "cirGate.h"
#include "cirMgr.h"
#include "minisat/reader.h"
#include "minisatMgr.h"
#include "satMgr.h"

class GVNetId;
typedef int ClauseId;

enum VAR_GROUP {
    LOCAL_ON,
    LOCAL_OFF,
    COMMON,
    NONE
};

namespace gv {
namespace itp {

class SatProofRes {
public:
    SatProofRes(gv::sat::SatSolverMgr* s = 0) : _proved(UINT_MAX), _fired(UINT_MAX), _maxDepth(UINT_MAX), _satSolver(s) {}

    void setProved(size_t i) { _proved = i; }
    void setFired(size_t i) { _fired = i; }

    bool isProved() const { return (_proved != UINT_MAX); }
    bool isFired() const { return (_fired != UINT_MAX); }

    void setMaxDepth(size_t d) { _maxDepth = d; }
    size_t getMaxDepth() const { return _maxDepth; }

    /*void setSatSolver(gv::sat::MinisatMgr* s) { _satSolver = s; }*/
    void setSatSolver(gv::sat::SatSolverMgr* s) { _satSolver = s; }
    /*gv::sat::MinisatMgr* getSatSolver() const { return _satSolver; }*/
    gv::sat::SatSolverMgr* getSatSolver() const { return _satSolver; }

    void reportResult(const string&) const;
    void reportCex(const CirGate*, const CirMgr* const) const;

private:
    size_t _proved;
    size_t _fired;
    size_t _maxDepth;  // maximum proof depth
    /*gv::sat::MinisatMgr* _satSolver;*/
    gv::sat::SatSolverMgr* _satSolver;
};
}  // namespace itp
}  // namespace gv

namespace gv {
namespace itp {

class ItpMgr {
public:
    ItpMgr() : _ptrMinisat(NULL), _cirMgr(NULL) { reset(); }
    ~ItpMgr() { reset(); }

    // entry point for SoCV SAT property checking
    void verifyPropertyItp(const string& name, const CirGate* monitor);
    void verifyPropertyBmc(const string& name, const CirGate* monitor);
    // Various proof engines
    void indBmc(const CirGate*, SatProofRes&);
    void itpUbmc(const CirGate*, SatProofRes&);

    // bind with a solver to get proof info.
    void bind(gv::sat::SatSolverMgr* ptrMinisat);
    // clear data members
    void reset();
    // mark onset/offset clause
    void markOnsetClause(const ClauseId& cid);
    void markOffsetClause(const ClauseId& cid);
    // map var to V3Net (PPI)
    void mapVar2Net(const Var& var, CirGate* net);
    // please be sure that you call these function right after a UNSAT solving
    // GVNetId        getItp() const;
    CirGate* getItp() const;
    vector<Clause> getUNSATCore() const;
    // get number of clauses (the latest clause id + 1)
    int getNumClauses() const { return _ptrMinisat->getNumClauses(); }

    // self define helper function
    void markSet(bool onORoff, ClauseId& currClause);
    bool startSatSolver(gv::sat::SatSolverMgr* GVSatSolver);
    void buildMiter(gv::sat::SatSolverMgr* GVSatSolver, GVNetId& R_, GVNetId& R, int& orgNtkSize);

private:
    // helper functions to get proof info.
    // GVNetId buildInitState() const;
    CirGate* buildInitState() const;
    // GVNetId buildItp(const string& proofName) const;
    CirGate* buildItp(const string& proofName) const;
    void retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<ClauseId>& usedClause) const;
    void retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;

    // GV minisat interface for model checking
    gv::sat::SatSolverMgr* _ptrMinisat;
    // The duplicated Cir
    CirMgr* _cirMgr;

    // to handle interpolation
    map<Var, CirGate*> _var2Net;  // mapping common variables to net
    vector<bool> _isClauseOn;     // record onset clauses
    // will be determined in retrieveProof, you don't need to take care about this!
    mutable vector<bool> _isClaOnDup;     // duplication & extension of _isClauseOn
    mutable vector<VAR_GROUP> _varGroup;  // mapping var to different groups
};

}  // namespace itp
}  // namespace gv
