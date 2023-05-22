/****************************************************************************
  FileName     [ satMiniSat.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Implementation of interface functions for MiniSat. ]
  Author       [ Cheng-Yin Wu, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "satMiniSat.h"
#include "satCommon.h"

Var
MiniSAT_Solver::newVar(const unsigned& width) {
   Var cur_var = _curVar;
   for (unsigned i = 0; i < width; ++i) _Solver->newVar();
   _curVar += width;
   return cur_var;
}

void
MiniSAT_Solver::add_PI_Formula(CktCell* cell, const unsigned& depth) {
   check_PI_IO(cell);
   CktOutPin* OutPin = cell->getOutPin();
   unsigned width = VLDesign.getBus(OutPin->getBusId())->getWidth();
   _vars[depth].insert(make_pair(OutPin, newVar(width)));  // set SATVar
}

void
MiniSAT_Solver::add_AND_Formula(CktCell* cell, const unsigned& depth) {
   check_AND_IO(cell);
   CktOutPin* OutPin = cell->getOutPin();
   unsigned width = VLDesign.getBus(OutPin->getBusId())->getWidth();
   Var var = _curVar; _vars[depth].insert(make_pair(OutPin, newVar(width)));  // set SATVar
   CktOutPin* aOutPin = cell->getInPin(0)->getOutPin();
   unsigned aWidth = VLDesign.getBus(aOutPin->getBusId())->getWidth();
   Var aVar = getVerifyData(aOutPin, depth); assert (aVar);
   if (static_cast<CktAndCell*>(cell)->getType() == CKT_RED) and_red(_Solver, aVar, var, aWidth);  // Reduced
   else {
      CktOutPin* bOutPin = cell->getInPin(1)->getOutPin();
      Var bVar = getVerifyData(bOutPin, depth); assert (bVar);
      if (static_cast<CktAndCell*>(cell)->getType() == CKT_LOG) and_2(_Solver, aVar, bVar, var);  // Logic
      else for (unsigned i = 0; i < width; ++i) and_2(_Solver, aVar + i, bVar + i, var + i);  // Bit-wise
   }
}

