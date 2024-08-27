#include "glucoseMgr.h"

#include "core/Solver.h"
#include "core/SolverTypes.h"
#include "fmt/core.h"

namespace gv_sat {

GlucoseMgr::GlucoseMgr(CirMgr* c) : _solver(new Glucose::Solver()), _cirMgr(c) {
    _assump.clear();
    _solver->newVar();
    _solver->verbosity = 1;
    _curVar            = 0;
    ++_curVar;
    _ntkData = new vector<Glucose::Var>[_cirMgr->getNumTots()];
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) _ntkData[i].clear();
}

GlucoseMgr::~GlucoseMgr() {
    delete _solver;
    assumeRelease();
    // Delete the Circuit Data
}

const bool GlucoseMgr::simplify() {
    return _solver->simplify();
}

const bool GlucoseMgr::solve() {
    _solver->solve();
    return _solver->okay();
}

const bool GlucoseMgr::assump_solve() {
    bool result = _solver->solve(_assump);
    return result;
}

void GlucoseMgr::assumeRelease() {
    _assump.clear();
}
void GlucoseMgr::assumeProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) {
    const Glucose::Var var = getVerifyData(gate, depth);
    _assump.push(Glucose::mkLit(var, invert));
}

void GlucoseMgr::assertProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) {
    const Glucose::Var var = getVerifyData(gate, depth);
    _solver->addClause(Glucose::mkLit(var, invert));
}

const Glucose::Var GlucoseMgr::newVar() {
    Glucose::Var cur_var = _curVar;
    _solver->newVar();
    _curVar++;
    return cur_var;
}

const Glucose::Var GlucoseMgr::getVerifyData(const CirGate* gate, const uint32_t& depth) const {
    if (depth >= _ntkData[gate->getGid()].size())
        return 0;
    else
        return _ntkData[gate->getGid()][depth];
}

void GlucoseMgr::add_FALSE_Formula(const CirGate* gate, const uint32_t& depth) {
    const uint32_t index = gate->getGid();
    _ntkData[index].push_back(newVar());
    _solver->addClause(Glucose::mkLit(_ntkData[index].back(), true));
    fmt::println("False Clause: {0}", _ntkData[index].back());
}

void GlucoseMgr::add_PI_Formula(const CirGate* gate, const uint32_t& depth) {
    const uint32_t index = gate->getGid();
    _ntkData[index].push_back(newVar());
    fmt::println("PI Clause: {0}", _ntkData[index].back());
}

void GlucoseMgr::add_FF_Formula(const CirGate* gate, const uint32_t& depth) {
    const uint32_t index = gate->getGid();
    if (depth) {
        // Build FF I/O Relation
        CirGateV in0            = gate->getIn0();
        const Glucose::Var var1 = getVerifyData(in0.gate(), depth - 1);

        if (in0.isInv()) {
            // a <-> b
            _ntkData[index].push_back(newVar());
            Glucose::Lit a = Glucose::mkLit(_ntkData[index].back());
            Glucose::Lit b = Glucose::mkLit(var1, true);
            Glucose::vec<Glucose::Lit> lits;
            lits.clear();
            lits.push(~a);
            lits.push(b);
            _solver->addClause(lits);
            lits.clear();
            lits.push(a);
            lits.push(~b);
            _solver->addClause(lits);
            lits.clear();
            fmt::println("FF Clause: {0}", _ntkData[index].back());
        } else
            _ntkData[index].push_back(var1);
    } else {  // Timeframe 0
        _ntkData[index].push_back(newVar());
    }
}

void GlucoseMgr::add_AND_Formula(const CirGate* gate, const uint32_t& depth) {
    // const uint32_t index = getGVNetIndex(out);

    const uint32_t index = gate->getGid();
    _ntkData[index].push_back(newVar());

    const Glucose::Var& var = _ntkData[index].back();
    // Build AND I/O Relation
    const CirGateV in0      = gate->getIn0();
    const CirGateV in1      = gate->getIn1();
    const Glucose::Var var0 = getVerifyData(in0.gate(), depth);
    const Glucose::Var var1 = getVerifyData(in1.gate(), depth);

    Glucose::Lit y = Glucose::mkLit(var);
    Glucose::Lit a = Glucose::mkLit(var0, in0.isInv());
    Glucose::Lit b = Glucose::mkLit(var1, in1.isInv());

    Glucose::vec<Glucose::Lit> lits;
    lits.clear();
    lits.push(a);
    lits.push(~y);
    _solver->addClause(lits);
    lits.clear();
    lits.push(b);
    lits.push(~y);
    _solver->addClause(lits);
    lits.clear();
    lits.push(~a);
    lits.push(~b);
    lits.push(y);
    _solver->addClause(lits);
    lits.clear();

    // print the inserted clause for debugging
    fmt::println("AND Clause: {0} {1} {2}", var0, var1, var);
}

void GlucoseMgr::addBoundedVerifyData(const CirGate* gate, const uint32_t& depth) {
    if (existVerifyData(gate, depth)) return;
    addBoundedVerifyDataRecursively(gate, depth);
}

void GlucoseMgr::addBoundedVerifyDataRecursively(const CirGate* gate, const uint32_t& depth) {
    GateType type = gate->getType();
    if (existVerifyData(gate, depth)) return;
    if (type == PI_GATE)
        add_PI_Formula(gate, depth);
    else if (RO_GATE == type || RI_GATE == type) {
        uint32_t newDepth = depth;
        if (depth) {
            if (type == RI_GATE) newDepth -= 1;
            addBoundedVerifyDataRecursively(gate->getIn0Gate(), newDepth);
        }
        add_FF_Formula(gate, depth);
    } else {
        if (type == PO_GATE) {
            addBoundedVerifyDataRecursively(gate->getIn0Gate(), depth);
            add_FF_Formula(gate, depth);
        } else if (type == AIG_GATE) {
            addBoundedVerifyDataRecursively(gate->getIn0Gate(), depth);
            addBoundedVerifyDataRecursively(gate->getIn1Gate(), depth);
            add_AND_Formula(gate, depth);
        } else {
            // CONST_GATE
            add_FALSE_Formula(gate, depth);
        }
    }
}

const bool GlucoseMgr::existVerifyData(const CirGate* gate, const uint32_t& depth) {
    return getVerifyData(gate, depth);
}

}  // namespace gv_sat
