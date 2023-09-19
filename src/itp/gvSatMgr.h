/****************************************************************************
  FileName     [ SATMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define sat prove package interface ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#ifndef SAT_MGR_H_
#define SAT_MGR_H_

#include "gvNtk.h"
#include "gvSat.h"
#include <cassert>
#include <map>
#include <unistd.h>
#include <vector>
// #include "SolverTypesV.h"
#include "reader.h"

class GVNetId;
typedef int ClauseId;

enum VAR_GROUP
{
    LOCAL_ON,
    LOCAL_OFF,
    COMMON,
    NONE
};

class SatProofRes
{
    public:
        SatProofRes(GVSatSolver* s = 0) : _proved(GVNtkUD), _fired(GVNtkUD), _maxDepth(GVNtkUD), _satSolver(s) {}

        void setProved(uint32_t i) { _proved = i; }
        void setFired(uint32_t i) { _fired = i; }

        bool isProved() const { return (_proved != GVNtkUD); }
        bool isFired() const { return (_fired != GVNtkUD); }

        void     setMaxDepth(uint32_t d) { _maxDepth = d; }
        uint32_t getMaxDepth() const { return _maxDepth; }

        void         setSatSolver(GVSatSolver* s) { _satSolver = s; }
        GVSatSolver* getSatSolver() const { return _satSolver; }

        void reportResult(const string&) const;
        void reportCex(const GVNetId&, const GVNtkMgr* const) const;

    private:
        uint32_t     _proved;
        uint32_t     _fired;
        uint32_t     _maxDepth; // maximum proof depth
        GVSatSolver* _satSolver;
};

class SATMgr
{
    public:
        SATMgr() : _ptrMinisat(NULL), _ntk(NULL) { reset(); }
        ~SATMgr() { reset(); }

        // entry point for SoCV SAT property checking
        void verifyPropertyItp(const string& name, const GVNetId& monitor);
        void verifyPropertyBmc(const string& name, const GVNetId& monitor);
        // Various proof engines
        void indBmc(const GVNetId&, SatProofRes&);
        void itpUbmc(const GVNetId&, SatProofRes&);

        // bind with a solver to get proof info.
        void           bind(GVSatSolver* ptrMinisat);
        // clear data members
        void           reset();
        // mark onset/offset clause
        void           markOnsetClause(const ClauseId& cid);
        void           markOffsetClause(const ClauseId& cid);
        // map var to V3Net (PPI)
        void           mapVar2Net(const Var& var, const GVNetId& net);
        // please be sure that you call these function right after a UNSAT solving
        GVNetId        getItp() const;
        vector<Clause> getUNSATCore() const;
        // get number of clauses (the latest clause id + 1)
        int            getNumClauses() const { return _ptrMinisat->getNumClauses(); }

        // self define helper function
        void markSet(bool onORoff, ClauseId& currClause);
        bool startSatSolver(GVSatSolver* GVSatSolver);
        void buildMiter(GVSatSolver* GVSatSolver, GVNetId& R_, GVNetId& R, int& orgNtkSize);

    private:
        // helper functions to get proof info.
        GVNetId buildInitState() const;
        GVNetId buildItp(const string& proofName) const;
        void    retrieveProof(Reader& rdr, vector<unsigned>& clausePos, vector<ClauseId>& usedClause) const;
        void    retrieveProof(Reader& rdr, vector<Clause>& unsatCore) const;

        // V3 minisat interface for model checking
        GVSatSolver* _ptrMinisat;
        // The duplicated V3Ntk
        GVNtkMgr*    _ntk;

        // to handle interpolation
        map<Var, GVNetId>         _var2Net;    // mapping common variables to net
        vector<bool>              _isClauseOn; // record onset clauses
        // will be determined in retrieveProof, you don't need to take care about this!
        mutable vector<bool>      _isClaOnDup; // duplication & extension of _isClauseOn
        mutable vector<VAR_GROUP> _varGroup;   // mapping var to different groups
};

#endif /* SAT_MGR_H_ */
