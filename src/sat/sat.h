/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_H
#define SAT_H

#include <cassert>
#include <iostream>
// #include "Solver.h"
#include <vector>
#include "sat/bsat/satSolver.h"

using namespace std;

/********** MiniSAT_Solver **********/
enum SatResult
{
   UNSAT = 0,
   SAT,
   UNDECIDED
};

typedef int Var;

class SatSolver
{
   public : 
      SatSolver():_solver(0) { }
      ~SatSolver() { if (_solver) delete _solver; }

      // Solver initialization and reset
      void initialize() {
         reset();
         if (_curVar == 0) { sat_solver_addvar(_solver); ++_curVar; }
      }
      void reset() {
         if (_solver) delete _solver;
         _solver = new sat_solver();
         _assump.clear(); //_curVar = 0;
      }

   //    // Constructing proof model
      // Return the Var ID of the new Var
      inline Var newVar() { sat_solver_addvar(_solver); return ++_curVar;; }
      // fa/fb = true if it is inverted
      void addAigCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         sat_solver_add_and( _solver, vf, va, vb, va, vb, 0 );
         // vec<lit> lits;
   //       Lit lf = Lit(vf);
   //       Lit la = fa? ~Lit(va): Lit(va);
   //       Lit lb = fb? ~Lit(vb): Lit(vb);
   //       lits.push(la); lits.push(~lf);
   //       _solver->addClause(lits); lits.clear();
   //       lits.push(lb); lits.push(~lf);
   //       _solver->addClause(lits); lits.clear();
   //       lits.push(~la); lits.push(~lb); lits.push(lf);
   //       _solver->addClause(lits); lits.clear();
      }
   //    // fa/fb = true if it is inverted
      void addXorCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         sat_solver_add_xor( _solver, vf, va, vb, (fa ^ fb) ? 1 : 0 );
   //       vec<Lit> lits;
   //       Lit lf = Lit(vf);
   //       Lit la = fa? ~Lit(va): Lit(va);
   //       Lit lb = fb? ~Lit(vb): Lit(vb);
   //       lits.push(~la); lits.push( lb); lits.push( lf);
   //       _solver->addClause(lits); lits.clear();
   //       lits.push( la); lits.push(~lb); lits.push( lf);
   //       _solver->addClause(lits); lits.clear();
   //       lits.push( la); lits.push( lb); lits.push(~lf);
   //       _solver->addClause(lits); lits.clear();
   //       lits.push(~la); lits.push(~lb); lits.push(~lf);
   //       _solver->addClause(lits); lits.clear();
      }

      // For incremental proof, use "assumeSolve()"
      void assumeRelease() { _assump.clear(); }
      void assumeProperty(Var prop, bool val) {
         _assump.push_back(toLitCond(prop, val));
      }
      SatResult assumpSolve() {
         return lbool2SatResult(sat_solver_solve(_solver, &_assump.front(), &_assump.back(), 0, 0, 0, 0));
      }

      // For one time proof, use "solve"
      void assertProperty(Var prop, bool val) {
         lit Lits[1];
         Lits[0] = Abc_Var2Lit(prop, val);
         sat_solver_addclause(_solver, Lits, Lits+1);
         // _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }
      bool solve() { return lbool2bool(sat_solver_solve(_solver, 0, 0, 0, 0, 0, 0)); }

      // Functions about Reporting
      // Return 1/0/-1; -1 means unknown value
      int getValue(Var v) const {
         return sat_solver_get_var_value(_solver, v); }
      void printStats() const { Sat_SolverPrintStats( stdout, _solver ); }

   private : 
      sat_solver           *_solver;    // Pointer to a Minisat solver
      Var               _curVar;    // Variable currently
      vector<lit>          _assump;    // Assumption List for assumption solve

      static SatResult lbool2SatResult(lbool b) {
         if (b == l_False) return UNSAT;
         if (b == l_True)  return SAT;
         return UNDECIDED;
      }
      static bool lbool2bool(lbool b) {
         if (b == l_False) return false;
         if (b == l_True)  return true;
      }
};

#endif  // SAT_H

