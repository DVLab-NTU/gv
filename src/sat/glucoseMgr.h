#pragma once

#include "cirGate.h"
#include "cirMgr.h"
#include "core/Solver.h"
#include "core/SolverTypes.h"
#include "mtl/Vec.h"
#include "satMgr.h"

namespace gv {
namespace sat {

class GlucoseMgr : SatSolverMgr {
public:
    GlucoseMgr(gv::cir::CirMgr*);
    ~GlucoseMgr();

    void assumeRelease();
    void assumeProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth);
    void assertProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth);
    const bool simplify();
    const bool solve();
    const bool assump_solve();

    // Gate Formula to Solver Functions
    void add_FALSE_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_PI_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_FF_Formula(const gv::cir::CirGate*, const uint32_t&);
    void add_AND_Formula(const gv::cir::CirGate*, const uint32_t&);

    void addBoundedVerifyData(const gv::cir::CirGate*, const uint32_t&);
    const bool existVerifyData(const gv::cir::CirGate*, const uint32_t&);
    /*void resizeNtkData(const uint32_t& num);*/

private:
    const Glucose::Var newVar();
    const Glucose::Var getVerifyData(const gv::cir::CirGate*, const uint32_t&) const;
    void addBoundedVerifyDataRecursively(const gv::cir::CirGate*, const uint32_t&);

    inline const Glucose::Var getOriVar(const size_t& v) const { return (Glucose::Var)(v >> 1ul); }
    inline const size_t getPosVar(const Glucose::Var& v) const { return (((size_t)v) << 1ul); }
    inline const size_t getNegVar(const Glucose::Var& v) const { return ((getPosVar(v)) | 1ul); }

    Glucose::Solver* _solver;
    Glucose::vec<Glucose::Lit> _assump;
    Glucose::Var _curVar;
    gv::cir::CirMgr* const _cirMgr;
    vector<Glucose::Var>* _ntkData;  // Mapping between GVNetId and Solver Data
};

}  // namespace sat
}  // namespace gv
