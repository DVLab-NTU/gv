/****************************************************************************
  FileName     [ satMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       []
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#pragma once

#include <cassert>

#include "cirMgr.h"
#include "core/Solver.h"
#include "minisat/Proof.h"
#include "minisat/gvBitVec.h"

namespace gv {
namespace sat {

enum class SATSolverType {
    MINISAT = 0,
    GLUCOSE
};

// Top-level SAT solver manager
class SatSolverMgr {
public:
    SatSolverMgr(gv::cir::CirMgr*);
    virtual ~SatSolverMgr();

    // clang-format off
    virtual void reset() = 0;
    virtual void assumeRelease() = 0;
    virtual void assumeProperty(const size_t&, const bool&) = 0;
    virtual void assertProperty(const size_t&, const bool&) = 0;
    virtual void assumeProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth) = 0;
    virtual void assertProperty(const gv::cir::CirGate* gate, const bool& invert, const uint32_t& depth) = 0;
    virtual const bool simplify() = 0;
    virtual const bool solve() = 0;
    virtual const bool assump_solve() = 0;
    virtual int getNumClauses() const = 0;
    virtual void add_FALSE_Formula(const gv::cir::CirGate*, const uint32_t&) = 0;
    virtual void add_PI_Formula(const gv::cir::CirGate*, const uint32_t&) = 0;
    virtual void add_FF_Formula(const gv::cir::CirGate*, const uint32_t&) = 0;
    virtual void add_AND_Formula(const gv::cir::CirGate*, const uint32_t&) = 0;
    virtual void addBoundedVerifyData(const gv::cir::CirGate*, const uint32_t&)  = 0;
    virtual const bool existVerifyData(const gv::cir::CirGate*, const uint32_t&) = 0;
    // clang-format on

    // minisat interface
    virtual void resizeNtkData(const uint32_t& num) {};
    virtual Proof* getProof() { return nullptr; };
    virtual int nVars() { return 0; };
    virtual const GVBitVecX getDataValue(const gv::cir::CirGate* gate, const uint32_t& depth) const { return GVBitVecX(); };
    virtual const Var getVerifyData(const gv::cir::CirGate*, const uint32_t&) const {};

private:
    gv::cir::CirMgr* _cirMgr;
};

}  // namespace sat
}  // namespace gv
