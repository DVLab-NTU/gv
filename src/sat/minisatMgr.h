/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <cassert>
#include <vector>

#include "cirGate.h"
#include "cirMgr.h"
#include "minisat/Solver.h"
#include "minisat/gvBitVec.h"
#include "satMgr.h"

using namespace std;

namespace gv {
namespace itp {
class ItpMgr;
}  // namespace itp
}  // namespace gv

namespace gv {
namespace sat {

/********** MiniSAT_Solver **********/
class MinisatMgr : public SatSolverMgr {
    friend class gv::itp::ItpMgr;

public:
    MinisatMgr(gv::cir::CirMgr*);
    ~MinisatMgr();

    void reset();
    void assumeRelease();
    void assumeProperty(const size_t&, const bool&);
    void assertProperty(const size_t&, const bool&);
    void assumeProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth);
    void assertProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth);
    const bool simplify();
    const bool solve();
    const bool assump_solve();
    const bool setSolver();
    int getNumClauses() const { return _solver->nRootCla(); }

    // Network to Solver Functions
    // const size_t getFormula(const GVNetId&, const uint32_t&);
    const GVBitVecX getDataValue(const gv::cir::CirGate*, const uint32_t&) const;
    const bool getDataValue(const size_t&) const;
    // Variable Interface Functions
    inline const size_t reserveFormula() { return getPosVar(newVar()); }
    inline const bool isNegFormula(const size_t& v) const { return (v & 1ul); }
    inline const size_t getNegFormula(const size_t& v) const { return (v ^ 1ul); }

    // Gate Formula to Solver Functions
    void add_FALSE_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_PI_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_FF_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_AND_Formula(const gv::cir::CirGate*, const uint32_t&);

    void addBoundedVerifyData(const gv::cir::CirGate*, const uint32_t&);
    const bool existVerifyData(const gv::cir::CirGate*, const uint32_t&);
    void resizeNtkData(const uint32_t& num);

    Proof* getProof() { return _solver->proof; };
    int nVars() { return _solver->nVars(); };
    const Var getVerifyData(const gv::cir::CirGate*, const uint32_t&) const;

private:
    const Var newVar();
    /*const Var getVerifyData(const gv::cir::CirGate*, const uint32_t&) const;*/
    void addBoundedVerifyDataRecursively(const gv::cir::CirGate*, const uint32_t&);

    inline const Var getOriVar(const size_t& v) const { return (Var)(v >> 1ul); }
    inline const size_t getPosVar(const Var& v) const { return (((size_t)v) << 1ul); }
    inline const size_t getNegVar(const Var& v) const { return ((getPosVar(v)) | 1ul); }

    SolverV* _solver;                // Pointer to a Minisat solver
    Var _curVar;                     // Variable currently
    vec<Lit> _assump;                // Assumption List for assumption solve
    vector<Var>* _ntkData;           // Mapping between GVNetId and Solver Data
    const gv::cir::CirMgr* _cirMgr;  // Network Under Verification
};

}  // namespace sat
}  // namespace gv
