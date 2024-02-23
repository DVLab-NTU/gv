/****************************************************************************
  FileName     [ SATMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/

#ifndef SAT_MGR_H_
#define SAT_MGR_H_

#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <map>
#include <vector>

// #include "gvNtk.h"
#include "gvSat.h"
// #include "SolverTypesV.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "reader.h"

class GVNetId;
typedef int ClauseId;

enum VAR_GROUP {
    LOCAL_ON,
    LOCAL_OFF,
    COMMON,
    NONE
};

class SatProofRes {
public:
    SatProofRes(GVSatSolver* s = 0) : _proved(UINT_MAX), _fired(UINT_MAX), _maxDepth(UINT_MAX), _satSolver(s) {}

    void setProved(size_t i) { _proved = i; }
    void setFired(size_t i) { _fired = i; }

    bool isProved() const { return (_proved != UINT_MAX); }
    bool isFired() const { return (_fired != UINT_MAX); }

    void setMaxDepth(size_t d) { _maxDepth = d; }
    size_t getMaxDepth() const { return _maxDepth; }

    void setSatSolver(GVSatSolver* s) { _satSolver = s; }
    GVSatSolver* getSatSolver() const { return _satSolver; }

    void reportResult(const string&) const;
    void reportCex(const CirGate*, const CirMgr* const) const;

private:
    size_t _proved;
    size_t _fired;
    size_t _maxDepth;  // maximum proof depth
    GVSatSolver* _satSolver;
};

class SATMgr {
public:
    SATMgr() : _ptrMinisat(NULL), _cirMgr(NULL) { reset(); }
    ~SATMgr() { reset(); }

    // entry point for SoCV SAT property checking
    void verifyPropertyItp(const string& name, const CirGate* monitor);
    void verifyPropertyBmc(const string& name, const CirGate* monitor);
    // Various proof engines
    void indBmc(const CirGate*, SatProofRes&);
    void itpUbmc(const CirGate*, SatProofRes&);

    // bind with a solver to get proof info.
    void bind(GVSatSolver* ptrMinisat);
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
    bool startSatSolver(GVSatSolver* GVSatSolver);
    void buildMiter(GVSatSolver* GVSatSolver, GVNetId& R_, GVNetId& R, int& orgNtkSize);

private:
    // helper functions to get proof info.
    // GVNetId buildInitState() const;
    CirGate* buildInitState() const;
    // GVNetId buildItp(const string& proofName) const;
    CirGate* buildItp(const string& proofName) const;
    void retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<ClauseId>& usedClause) const;
    void retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;

    // GV minisat interface for model checking
    GVSatSolver* _ptrMinisat;
    // The duplicated Cir
    CirMgr* _cirMgr;

    // to handle interpolation
    map<Var, CirGate*> _var2Net;  // mapping common variables to net
    vector<bool> _isClauseOn;     // record onset clauses
    // will be determined in retrieveProof, you don't need to take care about this!
    mutable vector<bool> _isClaOnDup;     // duplication & extension of _isClauseOn
    mutable vector<VAR_GROUP> _varGroup;  // mapping var to different groups
};

#endif /* SAT_MGR_H_ */
