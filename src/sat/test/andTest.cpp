#include <iostream>
#include <vector>
#include "sat.h"

using namespace std;

class Gate
{
public:
   Gate(unsigned i = 0): _gid(i) {}
   ~Gate() {}

   Var getVar() const { return _var; }
   void setVar(const Var& v) { _var = v; }

private:
   unsigned   _gid;  // for debugging purpose...
   Var        _var;
};

// 
//
vector<Gate *> gates;

void
initCircuit()
{
   // Init gates
   for (size_t i = 0; i < 301; ++i)
      gates.push_back(new Gate(i));  // gates[i]

   // POs are not needed in this demo example
}

void
genProofModel(SatSolver& s)
{
   // Allocate and record variables; No Var ID for POs
   for (size_t i = 0, n = gates.size(); i < n; ++i) {
      Var v = s.newVar();
      gates[i]->setVar(v);
   }

   s.addAigCNF(gates[101]->getVar(), gates[0]->getVar(), false,
               gates[1]->getVar(), false);
   for (size_t i = 2; i < 100; ++i) {
      // AIG(100+i, i, 100+i-1)
      s.addAigCNF(gates[100+i]->getVar(), gates[100+i-1]->getVar(), false,
                  gates[i]->getVar(), false);
   } // final output = 199

   s.addAigCNF(gates[200]->getVar(), gates[99]->getVar(), false,
               gates[100]->getVar(), false);
   for (int i = 98; i >= 0; --i) {
      s.addAigCNF(gates[103+i]->getVar(), gates[103+i-1]->getVar(), false,
                  gates[i]->getVar(), false);
   } // final output = 299

   s.addXorCNF(gates[300]->getVar(), gates[199]->getVar(), false,
               gates[299]->getVar(), false);
}

void reportResult(const SatSolver& solver, bool result)
{
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
   if (result) {
//      for (size_t i = 0, n = gates.size(); i < n; ++i)
      for (size_t i = 0, n = 101; i < n; ++i)
         cout << solver.getValue(gates[i]->getVar()) << endl;
   }
   cout << "output = " << solver.getValue(gates[300]->getVar()) << endl;
}

int main()
{
   initCircuit();

   SatSolver solver;
   solver.initialize();

   //
   genProofModel(solver);

   bool result;
   solver.assumeRelease();  // Clear assumptions
   solver.assumeProperty(gates[300]->getVar(), true);  // Gate(300) = 1
   result = solver.assumpSolve();
   reportResult(solver, result);
}
