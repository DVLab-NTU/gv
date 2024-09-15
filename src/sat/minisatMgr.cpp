/****************************************************************************
  FileName     [ sat.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "minisatMgr.h"

#include <cmath>

#include "cirGate.h"
#include "cirMgr.h"
#include "satMgr.h"

using namespace gv::cir;
using gv::sat::MinisatMgr;

MinisatMgr::MinisatMgr(CirMgr* cirMgr) : SatSolverMgr(cirMgr), _cirMgr(cirMgr) {
    _solver        = new SolverV();
    _solver->proof = new Proof();
    _assump.clear();
    _curVar = 0;
    _solver->newVar();
    ++_curVar;
    _ntkData = new vector<Var>[_cirMgr->getNumTots()];
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) _ntkData[i].clear();
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) {
        if ((*_cirMgr)[i] != nullptr)
            (*_cirMgr)[i]->clearSatVar();
    }
}

MinisatMgr::~MinisatMgr() {
    delete _solver;
    assumeRelease();
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) _ntkData[i].clear();
    delete[] _ntkData;
}

void MinisatMgr::reset() {
    delete _solver;
    _solver        = new SolverV();
    _solver->proof = new Proof();
    _assump.clear();
    _curVar = 0;
    _solver->newVar();
    ++_curVar;
    _ntkData = new vector<Var>[_cirMgr->getNumTots()];
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) _ntkData[i].clear();
    for (uint32_t i = 0; i < _cirMgr->getNumTots(); ++i) _cirMgr->getGate(i)->clearSatVar();
}

void MinisatMgr::assumeRelease() {
    _assump.clear();
}

void MinisatMgr::assumeProperty(const size_t& var, const bool& invert) {
    _assump.push(mkLit(getOriVar(var), invert ^ isNegFormula(var)));
}

void MinisatMgr::assertProperty(const size_t& var, const bool& invert) {
    _solver->addUnit(mkLit(getOriVar(var), invert ^ isNegFormula(var)));
}

void MinisatMgr::assumeProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) {
    const Var var = getVerifyData(gate, depth);
    _assump.push(mkLit(var, invert));
}

void MinisatMgr::assertProperty(const CirGate* gate, const bool& invert, const uint32_t& depth) {
    const Var var = getVerifyData(gate, depth);
    _solver->addUnit(mkLit(var, invert));
}

const bool MinisatMgr::simplify() {
    return _solver->simplifyDB();
}

const bool MinisatMgr::solve() {
    _solver->solve();
    return _solver->okay();
}

const bool MinisatMgr::assump_solve() {
    bool result = _solver->solve(_assump);
    return result;
}

const GVBitVecX MinisatMgr::getDataValue(const CirGate* gate, const uint32_t& depth) const {
    Var var = getVerifyData(gate, depth);
    uint32_t i, width = 1;
    GVBitVecX value(width);
    // Modification for cir structure
    if (false) {
        for (i = 0; i < width; ++i)
            if (gv_l_True == _solver->model[var + i])
                value.set0(i);
            else
                value.set1(i);
    } else {
        for (i = 0; i < width; ++i)
            if (gv_l_True == _solver->model[var + i])
                value.set1(i);
            else
                value.set0(i);
    }
    return value;
}

const bool MinisatMgr::getDataValue(const size_t& var) const {
    return (isNegFormula(var)) ^ (gv_l_True == _solver->model[getOriVar(var)]);
}

// const size_t
// MinisatMgr::getFormula(const GVNetId& id, const uint32_t& depth) {
// Var var = getVerifyData(id, depth);
// return (id.fanin0Cp ? getNegVar(var) : getPosVar(var));
// }

void MinisatMgr::resizeNtkData(const uint32_t& num) {
    vector<Var>* tmp = new vector<Var>[_cirMgr->getNumTots()];
    for (uint32_t i = 0, j = _cirMgr->getNumTots() - num; i < j; ++i) tmp[i] = _ntkData[i];
    delete[] _ntkData;
    _ntkData = tmp;
}

const Var MinisatMgr::newVar() {
    Var cur_var = _curVar;
    _solver->newVar();
    _curVar++;
    return cur_var;
}

const Var MinisatMgr::getVerifyData(const CirGate* gate, const uint32_t& depth) const {
    /*if (depth >= _ntkData[gate->getGid()].size())*/
    /*    return 0;*/
    /*else*/
    /*    return _ntkData[gate->getGid()][depth];*/
    return gate->getSatVar(depth);
}

void MinisatMgr::add_FALSE_Formula(const CirGate* gate, const uint32_t& depth) {
    /*const uint32_t index = gate->getGid();*/
    /*_ntkData[index].push_back(newVar());*/
    /*_solver->addUnit(mkLit(_ntkData[index].back(), true));*/
    gate->addSatVar(newVar());
    _solver->addUnit(mkLit(gate->getSatVar(depth), true));
}

void MinisatMgr::add_PI_Formula(const CirGate* gate, const uint32_t& depth) {
    /*const uint32_t index = gate->getGid();*/
    /*_ntkData[index].push_back(newVar());*/
    gate->addSatVar(newVar());
}

void MinisatMgr::add_FF_Formula(const CirGate* gate, const uint32_t& depth) {
    const uint32_t index = gate->getGid();
    if (depth) {
        // Build FF I/O Relation
        CirGateV in0   = gate->getIn0();
        const Var var1 = getVerifyData(in0.gate(), depth - 1);

        if (in0.isInv()) {
            // a <-> b
            /*_ntkData[index].push_back(newVar());*/
            gate->addSatVar(newVar());
            /*Lit a = mkLit(_ntkData[index].back());*/
            Lit a = mkLit(gate->getLastSatVar());
            Lit b = mkLit(var1, true);
            vec<Lit> lits;
            lits.clear();
            lits.push(~a);
            lits.push(b);
            _solver->addClause(lits);
            lits.clear();
            lits.push(a);
            lits.push(~b);
            _solver->addClause(lits);
            lits.clear();
        } else {
            /*_ntkData[index].push_back(var1);*/
            gate->addSatVar(var1);
        }
    } else {  // Timeframe 0
        /*_ntkData[index].push_back(newVar());*/
        gate->addSatVar(newVar());
    }
}

void MinisatMgr::add_AND_Formula(const CirGate* gate, const uint32_t& depth) {
    // const uint32_t index = getGVNetIndex(out);
    const uint32_t index = gate->getGid();
    /*_ntkData[index].push_back(newVar());*/
    gate->addSatVar(newVar());

    /*const Var& var = _ntkData[index].back();*/
    const Var& var = gate->getLastSatVar();
    // Build AND I/O Relation
    const CirGateV in0 = gate->getIn0();
    const CirGateV in1 = gate->getIn1();
    const Var var0     = getVerifyData(in0.gate(), depth);
    const Var var1     = getVerifyData(in1.gate(), depth);

    Lit y = mkLit(var);
    Lit a = mkLit(var0, in0.isInv());
    Lit b = mkLit(var1, in1.isInv());

    vec<Lit> lits;
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
}

void MinisatMgr::addBoundedVerifyData(const CirGate* gate, const uint32_t& depth) {
    if (existVerifyData(gate, depth)) return;
    addBoundedVerifyDataRecursively(gate, depth);
}

void MinisatMgr::addBoundedVerifyDataRecursively(const CirGate* gate, const uint32_t& depth) {
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

const bool MinisatMgr::existVerifyData(const CirGate* gate, const uint32_t& depth) {
    return getVerifyData(gate, depth);
}
