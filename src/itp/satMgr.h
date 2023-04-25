/****************************************************************************
  FileName     [ SATMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#ifndef SAT_MGR_H_
#define SAT_MGR_H_

#include <unistd.h>
#include <cassert>
#include <vector>
#include <map>
#include "v3Ntk.h"
#include "sat.h"
#include "reader.h"

class V3NetId;

enum VAR_GROUP {
  LOCAL_ON,
  LOCAL_OFF,
  COMMON,
  NONE
};

class SatProofRes {
  public:
    SatProofRes(SatSolver* s = 0):
      _proved(V3NtkUD), _fired(V3NtkUD), _maxDepth(V3NtkUD), _satSolver(s) {}

    void setProved(uint32_t i) { _proved = i; }
    void setFired(uint32_t i) { _fired = i; }

    bool isProved() const { return (_proved != V3NtkUD); }
    bool isFired() const { return (_fired != V3NtkUD); }

    void setMaxDepth(uint32_t d) { _maxDepth = d; }
    uint32_t getMaxDepth() const { return _maxDepth; }

    void setSatSolver(SatSolver* s) { _satSolver = s; }
    SatSolver* getSatSolver() const { return _satSolver; }

    void reportResult(const string&) const;
    void reportCex(const V3NetId&, const V3Ntk* const) const;

  private:
    uint32_t      _proved;
    uint32_t      _fired;
    uint32_t      _maxDepth;  // maximum proof depth
    SatSolver*    _satSolver;
};

class SATMgr {
  public:
    SATMgr():_ptrMinisat(NULL), _ntk(NULL) { reset(); }
    ~SATMgr() { reset(); }

    // entry point for SoCV SAT property checking
    void verifyPropertyItp(const string& name, const V3NetId& monitor);
    void verifyPropertyBmc(const string& name, const V3NetId& monitor);
    // Various proof engines
    void indBmc(const V3NetId&, SatProofRes&);
    void itpUbmc(const V3NetId&, SatProofRes&);

    // bind with a solver to get proof info.
    void bind(SatSolver* ptrMinisat);
    // clear data members
    void reset();
    // mark onset/offset clause
    void markOnsetClause(const ClauseId& cid);
    void markOffsetClause(const ClauseId& cid);
    // map var to V3Net (PPI)
    void mapVar2Net(const Var& var, const V3NetId& net);
    // please be sure that you call these function right after a UNSAT solving
    V3NetId getItp() const;
    vector<Clause> getUNSATCore() const;
    // get number of clauses (the latest clause id + 1)
    int getNumClauses() const{ return _ptrMinisat->getNumClauses(); }

    // self define helper function
    void markSet(bool onORoff,ClauseId& currClause);
    bool startSatSolver(SatSolver* satSolver);
    void buildMiter(SatSolver* satSolver,V3NetId& R_,V3NetId& R,int& orgNtkSize);
  private:
    // helper functions to get proof info.
    V3NetId buildInitState() const;
    V3NetId buildItp(const string& proofName) const;
    void retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<ClauseId>& usedClause) const;
    void retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;

    // V3 minisat interface for model checking
    SatSolver* _ptrMinisat;
    // The duplicated V3Ntk
    V3Ntk* _ntk;

    // to handle interpolation
    map<Var, V3NetId> _var2Net;    // mapping common variables to net
    vector<bool>      _isClauseOn; // record onset clauses
    // will be determined in retrieveProof, you don't need to take care about this!
    mutable vector<bool>      _isClaOnDup; // duplication & extension of _isClauseOn
    mutable vector<VAR_GROUP> _varGroup;   // mapping var to different groups
};

#endif /* SAT_MGR_H_ */

