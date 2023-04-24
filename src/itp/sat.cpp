/****************************************************************************
  FileName     [ sat.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu, Andrew Lee ]
  Copyright    [ Copyleft(c) 2010-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_C
#define SAT_C

#include "sat.h"
#include <cmath>

extern "C" {

}

SatSolver::SatSolver(const GVNtkMgr* const ntk ): _ntk(ntk)
{
   _solver = new sat_solver();
//    _solver->proof = new Proof(); 
   _assump.clear();
   _curVar = 0; sat_solver_addvar(_solver); ++_curVar;
   _ntkData = new vector<Var> [ntk->getNetSize()];
   for(uint32_t i = 0; i < ntk->getNetSize(); ++i ) _ntkData[i].clear();
}

SatSolver::~SatSolver() 
{
   delete _solver;  
   assumeRelease();
   for(uint32_t i = 0; i < _ntk->getNetSize(); ++i ) _ntkData[i].clear();
   delete [] _ntkData;
}

void SatSolver::reset()
{
   delete _solver; 
   _solver = new sat_solver();
//    _solver->proof = new Proof(); 
   _assump.clear();
   _curVar = 0; sat_solver_addvar(_solver); ++_curVar;
   _ntkData = new vector<Var> [_ntk->getNetSize()];
   for(uint32_t i = 0; i < _ntk->getNetSize(); ++i ) _ntkData[i].clear();
}

void SatSolver::assumeRelease() { _assump.clear(); }

void SatSolver::assumeProperty(const size_t& var, const bool& invert)
{
   _assump.push_back(Abc_Var2Lit(getOriVar(var), invert ^ isNegFormula(var)));
}

void SatSolver::assertProperty(const size_t& var, const bool& invert)
{
    lit Clause[1];
    Clause[0] = Abc_Var2Lit(getOriVar(var), invert ^ isNegFormula(var)); // create the property as single lit clause
    sat_solver_addclause(_solver, Clause, Clause+1);
}

void SatSolver::assumeProperty(const GVNetId& id, const bool& invert, const uint32_t& depth)
{
//    assert( 1 == _ntk->getNetWidth(id) );
   const Var var = getVerifyData(id, depth); assert(var);
   _assump.push_back(Abc_Var2Lit(var, invert ^ isGVNetInverted(id)));
}

// void SatSolver::assertProperty(const GVNetId& id, const bool& invert, const uint32_t& depth)
// {
//    assert(_ntk->validNetId(id)); assert( 1 == _ntk->getNetWidth(id) );
//    const Var var = getVerifyData(id, depth); assert(var);
//    _solver->addUnit(mkLit(var, invert ^ isGVNetInverted(id)));
// }

const int SatSolver::simplify() { return sat_solver_simplify(_solver); }

// solve without taking any assumptions
// return value 0 : undetermined, 1 : true, -1 : false
const int SatSolver::solve()
{
   return sat_solver_solve(_solver, 0, 0, 0, 0, 0, 0);
}

// sovle wuth the given assumptions
// return value 0 : undetermined, 1 : true, -1 : false
const int SatSolver::assump_solve()
{
   return sat_solver_solve(_solver, &_assump[0],&_assump[_assump.size()], 0, 0, 0, 0);
}

// const GVBitVecX SatSolver::getDataValue(const GVNetId& id, const uint32_t& depth) const 
// {
//    Var var = getVerifyData(id, depth); assert(var);
//    uint32_t i, width = _ntk->getNetWidth(id);
//    GVBitVecX value(width);
//    if(isGVNetInverted(id)) {
//       for(i = 0; i < width; ++i)
//          if(l_True == _solver->model[var+i]) value.set0(i);
//          else value.set1(i);
//    }
//    else {
//       for(i = 0; i < width; ++i)
//          if(l_True == _solver->model[var+i]) value.set1(i);
//          else value.set0(i);
//    }
//    return value;
// }

const bool SatSolver::getDataValue(const size_t& var) const 
{
   return (isNegFormula(var)) ^ (l_True == _solver->model[getOriVar(var)] );
}

const size_t SatSolver::getFormula(const GVNetId& id, const uint32_t& depth)
{
   Var var = getVerifyData(id, depth); assert(var);
   assert(!isNegFormula(getPosVar(var)));
   return (isGVNetInverted(id) ? getNegVar(var) : getPosVar(var));
}

void SatSolver::resizeNtkData(const uint32_t& num)
{
   vector<Var>* tmp = new vector<Var>[_ntk->getNetSize()];
   for(uint32_t i = 0, j = (_ntk->getNetSize()-num); i < j; ++i)
      tmp[i] = _ntkData[i];
   delete [] _ntkData;
   _ntkData = tmp;
}

const Var SatSolver::newVar() {
   Var cur_var = _curVar;
   sat_solver_addvar(_solver);
   _curVar++; 
   return cur_var;
}

const Var SatSolver::getVerifyData(const GVNetId& id, const uint32_t& depth) const
{
//    assert(_ntk->validNetId(id));
   if ( depth >= _ntkData[id.id].size() ) return 0;
   else return _ntkData[id.id][depth];
}

void SatSolver::add_FALSE_Formula(const GVNetId& out, const uint32_t& depth)
{
   const uint32_t index = out.id; 
    lit Clause[1];
    assert( depth == _ntkData[index].size());
    _ntkData[index].push_back(newVar()); 
    Clause[0] = Abc_Var2Lit(_ntkData[index].back(), true); // create the property as single lit clause
    sat_solver_addclause(_solver, Clause, Clause+1);
}

void SatSolver::add_PI_Formula(const GVNetId& out, const uint32_t& depth)
{
   const uint32_t index = out.id; 
   assert( depth == _ntkData[index].size());
   _ntkData[index].push_back(newVar()); 
}

void SatSolver::add_FF_Formula(const GVNetId& out, const uint32_t& depth) 
{
   const uint32_t index = out.id;
   assert( depth == _ntkData[index].size());

   if(depth) {
      // Build FF I/O Relation
      const GVNetId in1 = _ntk->getInputNetId(out,0);
      const Var var1 = getVerifyData(in1, depth-1); assert(var1);

      if(isGVNetInverted(in1)) {
         // a <-> b
         _ntkData[index].push_back(newVar());
         Lit a = Abc_Var2Lit(_ntkData[index].back(),false);
         Lit b = Abc_Var2Lit(var1,true);
         vector<Lit> lits; lits.clear();
         lits.push_back(~a); lits.push_back( b); sat_solver_addclause(_solver, &lits.front(), &lits.back()); lits.clear();
         lits.push_back( a); lits.push_back(~b); sat_solver_addclause(_solver, &lits.front(), &lits.back()); lits.clear();
      }
      else _ntkData[index].push_back(var1);
   }
   else {  // Timeframe 0
      _ntkData[index].push_back(newVar());
   }
}

void SatSolver::add_AND_Formula(const GVNetId& out, const uint32_t& depth)
{
   const uint32_t index = out.id; 
   assert( depth == _ntkData[index].size());
   _ntkData[index].push_back(newVar()); 

   const Var& var = _ntkData[index].back();
   // Build AND I/O Relation
   const GVNetId in1 = _ntk->getInputNetId(out,0);
   const GVNetId in2 = _ntk->getInputNetId(out,1);
   const Var var1 = getVerifyData(in1,depth); assert(var1);
   const Var var2 = getVerifyData(in2,depth); assert(var2);

   Lit y = Abc_Var2Lit(var, false);
   Lit a = Abc_Var2Lit(var1, isGVNetInverted(in1));
   Lit b = Abc_Var2Lit(var2, isGVNetInverted(in2));

   vector<Lit> lits; lits.clear();
   lits.push_back(a); lits.push_back(~y); sat_solver_addclause(_solver, &lits.front(), &lits.back()); lits.clear();
   lits.push_back(b); lits.push_back(~y); sat_solver_addclause(_solver, &lits.front(), &lits.back()); lits.clear();
   lits.push_back(~a); lits.push_back(~b); lits.push_back(y); sat_solver_addclause(_solver, &lits.front(), &lits.back()); lits.clear();
}

void SatSolver::addBoundedVerifyData(const GVNetId& id, const uint32_t& depth)
{
   if( existVerifyData( id, depth) ) return;
   addBoundedVerifyDataRecursively(id,depth);
}

void SatSolver::addBoundedVerifyDataRecursively(const GVNetId& id, const uint32_t& depth)
{
   if( existVerifyData(id,depth) ) return;
   const GV_Ntk_Type_t type = id.type;
   if( GV_NTK_OBJ_PI == type ) add_PI_Formula(id,depth);
   else if( GV_NTK_OBJ_FF_NS == type ) {  // build from ppo (next state)
      if(depth) { addBoundedVerifyDataRecursively(_ntk->getInputNetId(id,0), depth-1); }
      add_FF_Formula(id, depth);
   }
   else if(type == GV_NTK_OBJ_CONST0 || type == GV_NTK_OBJ_AIG) {
      if(GV_NTK_OBJ_AIG  == type) {
         addBoundedVerifyDataRecursively(_ntk->getInputNetId(id,0), depth);
         addBoundedVerifyDataRecursively(_ntk->getInputNetId(id,1), depth);
         add_AND_Formula(id,depth);
      }
      else {
         assert(GV_NTK_OBJ_CONST0 == type);
         add_FALSE_Formula(id,depth);
      }
   }
   else {
      assert(0);
   }
}

const bool SatSolver::existVerifyData(const GVNetId& id, const uint32_t& depth)
{
   return getVerifyData(id, depth);
}

#endif
