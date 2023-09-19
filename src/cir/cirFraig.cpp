// /****************************************************************************
//   FileName     [ cirFraig.cpp ]
//   PackageName  [ cir ]
//   Synopsis     [ Define cir FRAIG functions ]
//   Author       [ Chung-Yang (Ric) Huang ]
//   Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
// ****************************************************************************/

// #include <cassert>
// #include "cirMgr.h"
// #include "cirGate.h"
// #include "cirFraig.h"
// #include "sat.h"
// #include "myHashMap.h"
// #include "util.h"

// using namespace std;

// // TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
// //       Feel free to define your own variables or functions

// /*******************************/
// /*   Global variable and enum  */
// /*******************************/

// /**************************************/
// /*   Static varaibles and functions   */
// /**************************************/

// /*******************************************/
// /*   Public member functions about fraig   */
// /*******************************************/
// // _floatList may be changed.
// // _unusedList and _undefList won't be changed
// void
// CirMgr::strash()
// {
//    size_t numDfs = _dfsList.size();
//    size_t hSize = getHashSize(numDfs);
//    HashMap<StrashKey, CirGateV> sh(hSize);
//    size_t des = 0;
//    for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
//       CirGate *g = _dfsList[i];
//       if (!g->isAig()) { _dfsList[des] = g; ++des; continue; }
//       CirGateV in0 = g->getIn0();
//       CirGateV in1 = g->getIn1();
//       StrashKey k(in0(), in1());
//       CirGateV v;
//       if (sh.check(k, v)) { // Strash!!! remove gate g
//          in0.gate()->removeFanout(g);
//          in1.gate()->removeFanout(g);
//          v.gate()->merge("Strashing: ", g, false);
//          deleteAigGate(g);
//       }
//       else {  // keep this gate; insert to hash
//          v = size_t(g);
//          sh.forceInsert(k, v);
//          _dfsList[des] = g;
//          ++des;
//       }
//    }
//    _dfsList.resize(des);
//    checkFloatList();
// }

// void
// CirMgr::fraig()
// {
//    if (_fecGrps.empty()) return;

//    SatSolver solver;

//    // build proof model
//    initProofModel(solver, _dfsList);

//    unsigned nin = getNumPIs();
//    size_t patterns[nin];
//    size_t nPatterns = 0;
//    size_t nUnsat = 0;
//    bool doSimplify = false;
//    size_t UNSAT_RESTART = _dfsList.size() / 128 + 16;
//    // SAT: collect sim patterns; update FEC groups after 64 patterns
//    // UNSAT: set EQ gate, DO NOT merge
//    for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
//       CirGate *g = _dfsList[i];
//       if (g->getEqGate() != 0 || !g->isFEC()) continue;
//       CirPValue valG = g->getPValue();
//       if (((valG == size_t(0)) || (valG == ~size_t(0))) &&
//            (g->getFECId() / 2 == 0)) {
//          if (!g->isAig()) continue;
//          bool checkValue = (valG == 0);
//          if (satCheckConst(solver, g, checkValue, patterns)) {
//             unsigned litId = g->getFECId();
//             IdList *fecs = getFECGrps(litId/2);
//             removeData(*fecs, g->getGid()*2+litId%2);
//             g->resetFECId();
//             if (++nPatterns == 64) doSimplify = true;
//          }
//          else ++nUnsat;
//       }
//       else {
//          Var varG = g->getSatVar();
//          IdList *fecs = getFECGrps(g->getFECId()/2);
//          size_t j = 0, m = fecs->size();
//          for ( ; j < m; ++j) {
//             CirGate *f = litId2Gate((*fecs)[j]);
//             Var varF   = f->getSatVar();
//             if ((varF <= varG) || f->getEqGate() != 0) continue;
//             if (satCheckFEC(solver, g, f, (*fecs)[j]%2, patterns)) {
//                ++nPatterns;
//                if (nPatterns == 64) doSimplify = true;
//                break; // break when meet diff
//             }
//             else ++nUnsat;
//          }
//          removeData(*fecs, g->getGid()*2+g->getFECId()%2);
//          g->resetFECId();
//       }
//       if (doSimplify) {
//          cout << char(13) << setw(35) << ' ' << char(13);
// //         if (nUnsat >= 128) {
//          if (nUnsat >= UNSAT_RESTART) {
//             simplifyByEQ();
//             nUnsat = 0;
//             i = 0; n = _dfsList.size();
//          } // restart
//          if (nPatterns != 0) {
//             updateFECbySatPattern(patterns); nPatterns = 0; }
//          doSimplify = false;
//       }
//    }
//    cout << char(13) << setw(35) << ' ' << char(13);
//    if (nUnsat != 0) simplifyByEQ();
//    if (nPatterns != 0) updateFECbySatPattern(patterns);
//    checkFloatList();
//    checkUnusedList();
//    clearFECGrps();
//    unsetFlag(NO_FEC); // in order to perform sim and fraig again
//    strash();
// }

// /********************************************/
// /*   Private member functions about fraig   */
// /********************************************/
// void
// CirMgr::initProofModel(SatSolver& solver, const GateList& gates)
// {
//    solver.initialize();
//    _const0->setSatVar(solver.newVar()); // CANNOT assume const gate's SAT Var=0
//    for (size_t i = 0, n = gates.size(); i < n; ++i) {
//       CirGate *g = gates[i];
//       g->setSatVar(solver.newVar());
//       if (!g->isAig()) continue;
//       CirGateV in0 = g->getIn0();
//       CirGateV in1 = g->getIn1();
//       solver.addAigCNF(g->getSatVar(), in0.gate()->getSatVar(), in0.isInv(),
//                                        in1.gate()->getSatVar(), in1.isInv());
//    }
// }

// // return true if SAT
// bool
// CirMgr::satCheckConst(SatSolver& solver, CirGate *g, bool value,
//                       SimPattern patterns)
// {
// //   g->setToGlobalRef();
//    cout << char(13) << setw(35) << ' ' << char(13) << "Proving "
//         << g->getGid() << " = " << (value?"1":"0") << "..." << flush;
//    Var var = g->getSatVar();
//    solver.assumeRelease();
//    solver.assumeProperty(_const0->getSatVar(), false);
//    solver.assumeProperty(var, value);
//    switch (solver.assumpSolve()) {
//       case SAT:
//          cout << "SAT!!" << flush;
//          getSatAssignment(solver, patterns);
//          return true;
//       case UNSAT:
//          cout << "UNSAT!!" << flush;
//          g->setEqGate(_const0, !value);
//          break;
//       case UNDECIDED:
//          cout << "Aborting..." << flush;
//          break;
//    }
//    return false;
// }

// bool
// CirMgr::satCheckFEC(SatSolver& solver, CirGate *g, CirGate *f, bool isInvF,
//                     SimPattern patterns)
// {
//    bool isInvG = g->getFECId() & 1;
//    cout << char(13) << setw(35) << ' ' << char(13) << "Proving ("
//         << (isInvG?"!":"") << g->getGid() << ", " << (isInvF?"!":"")
//         << f->getGid() << ")..." << flush;
//    Var varG = g->getSatVar();
//    Var varF = f->getSatVar();
//    Var newV = solver.newVar();
//    solver.addXorCNF(newV, varG, isInvG, varF, isInvF);
//    solver.assumeRelease();
//    solver.assumeProperty(_const0->getSatVar(), false);
//    solver.assumeProperty(newV, true);
//    switch (solver.assumpSolve()) {
//       case SAT:
//          cout << "SAT!!" << flush;
//          getSatAssignment(solver, patterns);
//          return true;
//       case UNSAT:
//          cout << "UNSAT!!" << flush;
//          f->setEqGate(g, isInvG ^ isInvF);
//          break;
//       case UNDECIDED:
//          cout << "Aborting..." << flush;
//          break;
//    }
//    return false;
// }

// void
// CirMgr::getSatAssignment(SatSolver& solver, SimPattern patterns) const
// {
//    for (unsigned i = 0, nin = getNumPIs(); i < nin; ++i) {
//       unsigned v = solver.getValue(_piList[i]->getSatVar());
//       patterns[i] <<= 1;
//       if (v == 1) patterns[i] += 1;
//    }
// }

// void
// CirMgr::simplifyByEQ()
// {
//    for (size_t j = 0, n = _dfsList.size(); j < n; ++j) {
//       CirGate *gate = _dfsList[j];
//       CirGateV eq = gate->getEqGate();
//       if (eq == 0) continue;
//       assert(gate->isAig());
//       CirGate *in0 = gate->getIn0Gate();
//       CirGate *in1 = gate->getIn1Gate();
//       in0->removeFanout(gate);
//       in1->removeFanout(gate);
//       eq.gate()->merge("Fraig: ", gate, eq.isInv()); 
//       deleteAigGate(gate);
//    }
// //   simplify();
//    genDfsList();
//    simplifyFECGrps();
//    cout << "Updating by UNSAT... Total #FEC Group = "
//         << _fecGrps.size() << endl;
// }

// void
// CirMgr::updateFECbySatPattern(SimPattern patterns)
// {
//    setPPattern(patterns);
//    pSim1Pattern();
//    checkFEC();
//    finalizeFEC();
//    cout << "Updating by SAT... Total #FEC Group = "
//         << _fecGrps.size() << endl;
//    size_t nin = getNumPIs();
//    SimPattern pp = new size_t[nin];
//    for (size_t i = 0; i < nin; ++i) { pp[i] = patterns[i]; patterns[i] = 0; }
//    _fecVector.push_back(pp);
// }

