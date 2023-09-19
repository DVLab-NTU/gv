/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   CirGate::setGlobalRef(2); // gate's ref +1 is TO BE DELETED
   for (size_t i = 0, n = _dfsList.size(); i < n; ++i)
      _dfsList[i]->setToGlobalRef();

   // some PIs may become unused
   // PI must be setToGlobalRef() so that they won't get removed
   // [e.g] kk.aag (PI 1 <== not in the DFS list; fanin of an UNUSED gate 5)
   for (unsigned i = 0, n = getNumPIs(); i < n; ++i)
      getPi(i)->setToGlobalRef();
   _const0->setToGlobalRef();

   // Some UNDEF gates are reachable from POs
   // [e.g.] ha_bug.aag (gate 4)
   for (size_t i = 0, n = _undefList.size(); i < n; ++i) {
      const GateList &fanouts = getFanouts(_undefList[i]);
      for (size_t j = 0, m = fanouts.size(); j < m; ++j)
         if (fanouts[j]->isGlobalRef()) {
            getGate(_undefList[i])->setToGlobalRef(); break;
         }
   }

   // The newly added unused gate will be kept (Note: only PI is possible)
   for (size_t i = 1, n = getNumTots(); i < n; ++i) {
      CirGate *g = getGate(i);
      if (!g || g->isGlobalRef()) continue;
      cout << "Sweeping: " << g->getTypeStr() << "(" << i
           << ") removed...\n";
      if (g->isAig()) {
         CirGate *fanin = g->getIn0Gate();
         if (fanin->isGlobalRef()) fanin->removeFanout(g);
         fanin = g->getIn1Gate();
         if (fanin->isGlobalRef()) fanin->removeFanout(g);
         // deleteAigGate(g);
         g->setToGlobalRef(1);
      }
      else if (g->isUndef()) { deleteUndefGate(g); }
      else { assert(0); }  // Shouldn't happen!!
   }

   for (size_t i = 1, n = getNumTots(); i < n; ++i) {
      CirGate *g = getGate(i);
      if (!g || !g->isGlobalRef(1)) continue;
      assert(g->isAig());
      deleteAigGate(g);
   }

   updateUndefList();
   checkFloatList();
   checkUnusedList();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
// ==> need to update _undefList (e.g. flt_po.aag)
void
CirMgr::optimize()
{
   GateList deleteList;
   CirGate::setGlobalRef();
   for (unsigned i = 0, n = getNumPOs(); i < n; ++i)
      getPo(i)->optimize(false, deleteList);

   for (size_t i = 0, n = deleteList.size(); i < n; ++i)
      deleteAigGate(deleteList[i]);

   genDfsList();
   updateUndefList();
   checkFloatList();
   checkUnusedList();
}

CirGateV
CirPoGate::optimize(bool phase, GateList& deleteList)
{
   setToGlobalRef();

   CirGate *g0 = _in0.gate();
   if (!g0->isGlobalRef())
      return g0->optimize(_in0.isInv(), deleteList);
   else return _in0;
}

CirGateV
CirRiGate::optimize(bool phase, GateList& deleteList)
{
   setToGlobalRef();

   CirGate *g0 = _in0.gate();
   if (!g0->isGlobalRef())
      return g0->optimize(_in0.isInv(), deleteList);
   else return _in0;
}

CirGateV
CirAigGate::optimize(bool phase, GateList& deleteList)
{
   setToGlobalRef();

   CirGateV newIn0, newIn1;
   CirGate *g0 = _in0.gate();
   CirGate *g1 = _in1.gate();
   if (!g0->isGlobalRef())
      newIn0 = g0->optimize(_in0.isInv(), deleteList);
   else newIn0 = _in0;
   if (!g1->isGlobalRef())
      newIn1 = g1->optimize(_in1.isInv(), deleteList);
   else newIn1 = _in1;
   
   CirGateV newV;
   if (cirMgr->checkAigOptimize(this, newIn0, newIn1, newV)) {
      // newV.gate()->replace(this, newV.isInv());
      // Apply merge before removeFanout so that newV won't be UNUSED
      newV.gate()->merge("Simplifying: ", this, newV.isInv());
      newV.gate()->removeFanout(this);
      deleteList.push_back(this);
      newV.setInv(phase);
      return newV;
   }

   // No simplification
   return size_t(this) + (phase?1: 0);
}

// return true if g can be simplified by fanins(in0, in1)
bool
CirMgr::checkAigOptimize(CirGate* g, const CirGateV& in0,
                         const CirGateV& in1, CirGateV& gn) const
{
   CirGate *g0 = in0.gate(), *g1 = in1.gate();
   if (g0 == g1) {
      if (in0.isInv() ^ in1.isInv())  // a & !a = 0
         gn = constSimplify(g, size_t(_const0), in0);
      else                            // a & a = a
         gn = in0;
   }
   else if (g0->isConst()) 
      gn = constSimplify(g, in0, in1);
   else if (g1->isConst())
      gn = constSimplify(g, in1, in0);
   else return false;
   return true;
}

// DOES NOT update _dfsList!!
// Need to take care separately...
void
CirMgr::deleteAigGate(CirGate *g)
{
   assert(g->isAig());
   _totGateList[g->getGid()] = 0;
   --(_numDecl[AIG]);
   clearList<GateList>(_fanoutInfo[g->getGid()]);
   delete g;
}

// DOES NOT update _dfsList and _undefList!!
// Need to take care separately...
void
CirMgr::deleteUndefGate(CirGate *g)
{
   assert(g->isUndef());
   _totGateList[g->getGid()] = 0;
   clearList<GateList>(_fanoutInfo[g->getGid()]);
   delete g;
}

// This will merge mg.
// All the fanouts of mg will be merged to this' fanouts.
// Whoever calls this funciton needs to handle if an UNDEF gate replaces 
// some other gates ==> call cirMgr->checkFloatList() later
// [e.g] flt_po*.aag
void
CirGate::merge(const string& header, CirGate *mg, bool isInv)
{
   if (header.size()) {
      cout << header << _gid << " merging " << (isInv?"!":"")
           << mg->getGid() << "..." << endl;
   }
   GateList& fanouts = cirMgr->getFanouts(_gid);
   GateList& mFanouts = cirMgr->getFanouts(mg->getGid());
   for (size_t i = 0, n = mFanouts.size(); i < n; ++i)
      mFanouts[i]->replaceFanin(mg, this, isInv);
   fanouts.insert(fanouts.end(), mFanouts.begin(), mFanouts.end());
   clearList(mFanouts);
}

// This will replace 'g'
// We don't care if g has fanin to this
/*
void CirGate::replace(CirGate* g, bool inv)
{
    removeFanout(g); // in case this has fanout to g
    merge("Simplifying: ", g, inv);
}
*/

// 1. Remove f from this' fanouts.
// 2. Remove this from f's fanins.
//    However, since f is going to be deleted,
//    we don't need to actually remove this from f's fanins.
// 3. We assume this must = f._in0 or f._in1 <== DO NOT CHECK!!
// [Note] The original fanin will become invalid!!
void
CirGate::removeFanout(CirGate* f) const
{
   GateList& fanouts = cirMgr->getFanouts(_gid);
   removeData(fanouts, f);
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
CirGateV
CirMgr::constSimplify(CirGate *g, const CirGateV& constV,
                      const CirGateV& otherV) const
{
   if (constV.isInv()) {  // const 1
      constV.gate()->removeFanout(g);
      return otherV;
   }
   else {                 // const 0
      otherV.gate()->removeFanout(g);
      return size_t(_const0);
   }
}

