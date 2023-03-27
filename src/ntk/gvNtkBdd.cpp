/****************************************************************************
  FileName     [ v3NtkBdd.cpp ]
  PackageName  [ v3/src/ntk ]
  Synopsis     [ V3 Network to BDDs. ]
  Author       [ Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef V3_NTK_C
#define V3_NTK_C

// #include "v3NtkHandler.h" // MODIFICATION FOR SoCV BDD
#include "util.h"
#include "gvNtk.h"
#include "gvMsg.h"
#include "bddNodeV.h" // MODIFICATION FOR SoCV BDD
#include "bddMgrV.h" // MODIFICATION FOR SoCV BDD

extern BddMgrV* bddMgrV; // MODIFICATION FOR SoCV BDD

const bool GVNtkMgr::setBddOrder(const bool& file) const {
  unsigned supportSize = getInputSize() + getInoutSize() + 2*getFFSize();
  if(supportSize >= bddMgrV->getNumSupports()) {
    gvMsg(GV_MSG_ERR) << "BDD Support Size is Smaller Than Current Design Required !!" << endl;
    return false;
  }
  // build support
  unsigned supportId = 1;
  for(unsigned i = 0, n = getInputSize(); i < n; ++i) {
    const GVNetId& nId = (file)? getInput(i) : getInput(n-i-1);
    bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
    //bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(nId),
    //    bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  for(unsigned i = 0, n = getInoutSize(); i < n; ++i) {
    const GVNetId& nId = (file)? getInout(i) : getInout(n-i-1);
    bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
    //bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(nId),
    //    bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  for(unsigned i = 0, n = getFFSize(); i < n; ++i) {
    const GVNetId& nId = (file)? getLatch(i) : getLatch(n-i-1);
    bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
    //bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(nId),
    //    bddMgrV->getSupport(supportId)());
    ++supportId;
  }
  // Next State
  for(unsigned i = 0, n = getFFSize(); i < n; ++i) {
    const GVNetId& nId = (file)? getLatch(i) : getLatch(n-i-1);
    //bddMgrV->addBddNodeV(handler->getNetNameOrFormedWithId(nId)+"_ns",
    //    bddMgrV->getSupport(supportId)());
    ++supportId;
  }

  // Constants
  /*for (uint32_t i = 0; i < getConstSize(); ++i) {
    assert(getGateType(getConst(i)) == AIG_FALSE);
    bddMgrV->addBddNodeV(getConst(i).id, BddNodeV::_zero());
  }*/

  return true;
}

/*
void GVNtkMgr::buildNtkBdd() {
  // TODO: build BDD for ntk here
  // Perform DFS traversal from DFF inputs, inout, and output gates.
  // Collect ordered nets to a V3NetVec
  // Construct BDDs in the DFS order

  V3Stack<GVNetId>::Stack s;
  for (unsigned i = 0; i < getOutputSize(); ++i) {
    s.push(getOutput(i));
  }
  while (s.size() > 0) {
    buildBdd(s.top());
    s.pop();
  }
  _isBddBuilt = true;
  
  // build next state
  for (unsigned i = 0; i < getFFSize(); ++i) {
    GVNetId left = getInputNetId(getLatch(i), 0);
    if (bddMgrV->getBddNodeV(left.id) == (size_t)0) {
      buildBdd(left);
    }
    BddNodeV ns = ((left.cp)? ~bddMgrV->getBddNodeV(left.id): bddMgrV->getBddNodeV(left.id));
    bddMgrV->addBddNodeV(getLatch(i).id, ns());
  }
  // BddNodeV test = bddMgrV->getBddNodeV(420);
  // cout << test << endl;
}
*/

/*
void GVNtkMgr::buildBdd(const GVNetId& netId) {
  V3NetVec orderedNets;

  orderedNets.clear();
  orderedNets.reserve(getNetSize());

  newMiscData();
  dfsOrder(netId, orderedNets);
  assert (orderedNets.size() <= getNetSize());

  // TODO: build BDD for the specified net here

  GVNetId left, right; 
  for (unsigned i = 0; i < orderedNets.size(); ++i) {
    if (getGateType(orderedNets[i]) == AIG_NODE) {
      left = getInputNetId(orderedNets[i], 0);
      right = getInputNetId(orderedNets[i], 1);
      if (bddMgrV->getBddNodeV(left.id) == (size_t)0) {
        buildBdd(left);
      }
      if (bddMgrV->getBddNodeV(right.id) == (size_t)0) {
        buildBdd(right);
      }
      BddNodeV newNode = ((left.cp)? ~bddMgrV->getBddNodeV(left.id): bddMgrV->getBddNodeV(left.id)) &
                        ((right.cp)? ~bddMgrV->getBddNodeV(right.id): bddMgrV->getBddNodeV(right.id));
      bddMgrV->addBddNodeV(orderedNets[i].id, newNode());
    }
  }
}
*/

/*
// put fanins of a net (id) into a vector (nets) in topological order
void GVNtkMgr::dfsOrder(const GVNetId& id, V3NetVec& nets) {
  if (isLatestMiscData(id)) return;
  // Set Latest Misc Data
  setLatestMiscData(id);
  // Traverse Fanin Logics
  const V3GateType type = getGateType(id);
  if(type == AIG_NODE) {
    dfsOrder(getInputNetId(id, 0), nets);
    dfsOrder(getInputNetId(id, 1), nets);
  }
  nets.push_back(id);  // Record Order
}
*/
#endif
