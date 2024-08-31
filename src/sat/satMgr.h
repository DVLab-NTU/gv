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
    SatSolverMgr(CirMgr*);
    virtual ~SatSolverMgr();

    // clang-format off
    virtual void reset() = 0;
    virtual void assumeRelease() = 0;
    virtual void assumeProperty(const size_t&, const bool&) = 0;
    virtual void assertProperty(const size_t&, const bool&) = 0;
    virtual void assumeProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) = 0;
    virtual void assertProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) = 0;
    virtual const bool simplify() = 0;
    virtual const bool solve() = 0;
    virtual const bool assump_solve() = 0;
    virtual int getNumClauses() const = 0;
    virtual void add_FALSE_Formula(const CirGate*, const uint32_t&) = 0;
    virtual void add_PI_Formula(const CirGate*, const uint32_t&) = 0;
    virtual void add_FF_Formula(const CirGate*, const uint32_t&) = 0;
    virtual void add_AND_Formula(const CirGate*, const uint32_t&) = 0;
    virtual void addBoundedVerifyData(const CirGate*, const uint32_t&)  = 0;
    virtual const bool existVerifyData(const CirGate*, const uint32_t&) = 0;
    virtual void resizeNtkData(const uint32_t& num) = 0;
    // clang-format on

    // minisat interface
    virtual Proof* getProof() { return nullptr; };
    virtual int nVars() { return 0; };
    virtual const GVBitVecX getDataValue(const CirGate* gate, const uint32_t& depth) const { return GVBitVecX(); };
    virtual const Var getVerifyData(const CirGate*, const uint32_t&) const {};

private:
    CirMgr* _cirMgr;
    /*Solver _solver;*/
};

}  // namespace sat
}  // namespace gv
