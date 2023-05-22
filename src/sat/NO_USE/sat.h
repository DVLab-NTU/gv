/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Encapsulation of solver engines for inheritance. ]
  Author       [ Cheng-Yin Wu, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_H
#define SAT_H

#include "verifyDefine.h"
#include "verifyStream.h"
#include "cktCell.h"

// Base solver engine class, cannot be directly used!
class satSolver
{
   public : 
      satSolver(const satType);
      virtual ~satSolver();

      // Solver Interface Functions
*      virtual bool setOutputFile(const string);
      virtual void initialize();
*      virtual void assumeInit();
*      virtual void assertInit();
*      virtual void assumeRelease();
*      virtual void assumeProperty(CktOutPin*, bool);
      virtual void assertProperty(CktOutPin*, bool);
*      virtual void allProperty(CktOutPin*, bool);
*      virtual void increaseBound();
      virtual bool solve();
*      virtual bool assump_solve();
*      virtual void printInfo() const;
*      virtual void printSolver() const;
*      virtual bool admitListSize(const unsigned&) const;
      virtual bool existVerifyData(CktOutPin*, const unsigned& = 0) const;
      // Cell Formula to Solver Functions
      virtual void add_PI_Formula(CktCell*, const unsigned&);        // PI, PO, PIO
      virtual void add_AND_Formula(CktCell*, const unsigned&);
      virtual void add_CONST_Formula(CktCell*, const unsigned&);     // RTL Model
      // Solver Basic Function
      void addVerifyData(CktCell*, const unsigned&);
*      // Static Functions
*      static VerifySolver* newVerifySolver(const SolverType);

   private : 
      vector<CktOutPin*>   _DFFList;
};

// User-Introduced Solver Engine
/********** MiniSAT_Solver **********/
class MiniSAT_Solver : public VerifySolver
{
   public : 
      MiniSAT_Solver();
      ~MiniSAT_Solver();
      // Solver Interface Functions Realization
      void initialize();
*      void assumeInit();
*      void assertInit();
*      void assumeRelease();
*      void assumeProperty(CktOutPin*, bool);
      void assertProperty(CktOutPin*, bool);
*      void allProperty(CktOutPin*, bool);
*      void increaseBound();
      bool solve();
*      bool assump_solve();
*      void printInfo() const;
*      void printSolver() const;
*      bool admitListSize(const unsigned&) const;
      bool existVerifyData(CktOutPin*, const unsigned& = 0) const;
      // Cell Formula to Solver Functions Realization
      void add_PI_Formula(CktCell*, const unsigned&);       // PI, PO, PIO
      void add_AND_Formula(CktCell*, const unsigned&);
      void add_CONST_Formula(CktCell*, const unsigned&);    // RTL Model
      // Inline Functions
      inline void resetVerifyData() { resetDFFList(); _init.clear(); _vars.clear(); _assump.clear(); _curVar = 0; }
      // MiniSat Functions
      Var  newVar(const unsigned&);
      Var  getVerifyData(CktOutPin*, const unsigned&) const;
   private : 
      Solver*        _Solver;    // Pointer to a Minisat solver
      Var            _curVar;    // Variable currently
      vec<Lit>       _assump;    // Assumption List for assumption solve
      SATVarVec      _init;      // Initial state Var storage
      SATVarMap      _vars;      // Mapping from CktOutPin* to Var
};

#endif  // SAT_H

