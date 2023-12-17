/****************************************************************************
  FileName     [ cirBdd.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define BDD manager functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "bddMgrV.h"   // MODIFICATION FOR SoCV BDD
#include "bddNodeV.h"  // MODIFICATION FOR SoCV BDD
#include "cirGate.h"
#include "cirMgr.h"
#include "gvMsg.h"
#include "util.h"

extern BddMgrV* bddMgrV;  // MODIFICATION FOR SoCV BDD

const bool
CirMgr::setBddOrder(const bool& file) {
    unsigned supportSize = getNumPIs() + 2 * getNumLATCHs();
    unsigned bddspsize = bddMgrV->getNumSupports();
    if (supportSize >= bddMgrV->getNumSupports()) {
        gvMsg(GV_MSG_ERR) << "BDD Support Size is Smaller Than Current Design Required !!" << endl;
        return false;
    }
    // build support
    unsigned supportId = 1;
    // build PI (primary input)
    for (unsigned i = 0, n = getNumPIs(); i < n; ++i) {
        CirPiGate* gate = (file) ? getPi(i) : getPi(n - i - 1);
        bddMgrV->addBddNodeV(gate->getGid(), bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    // build FF_CS (X: current state)
    for (unsigned i = 0, n = getNumLATCHs(); i < n; ++i) {
        CirRoGate* gate = (file) ? getRo(i) : getRo(n - i - 1);
        bddMgrV->addBddNodeV(gate->getGid(), bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    // build FF_NS (Y: next state)
    // here we only create "CS_name + _ns" for y_i
    for (unsigned i = 0, n = getNumLATCHs(); i < n; ++i) {
        CirRiGate* gate = (file) ? getRi(i) : getRi(n - i - 1);
        bddMgrV->addBddNodeV(gate->getName(), bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    // Constants (const0 node, id=0)
    bddMgrV->addBddNodeV(_const0->getGid(), BddNodeV::_zero());
    ++supportId;

    return true;
}

void CirMgr::buildNtkBdd() {
    // TODO: build BDD for ntk here
    // Perform DFS traversal from DFF inputs, inout, and output gates.
    // Collect ordered nets to a GVNetVec
    // Construct BDDs in the DFS order

    // build PO
    for (unsigned i = 0; i < getNumPOs(); ++i) {
        buildBdd(getPo(i));
    }

    // build next state (RI)
    for (unsigned i = 0; i < getNumLATCHs(); ++i) {
        CirGate* left = getRi(i);  // get RI
        if (bddMgrV->getBddNodeV(left->getGid()) == (size_t)0) {
            buildBdd(left);
        }
        BddNodeV ns = ((left->getIn0().isInv()) ? ~bddMgrV->getBddNodeV(left->getGid()) : bddMgrV->getBddNodeV(left->getGid()));
    }
}

void CirMgr::buildBdd(CirGate* gate) {
    GateList orderedGates;
    clearList(orderedGates);
    CirGate::setGlobalRef();
    gate->genDfsList(orderedGates);
    assert(orderedGates.size() <= getNumTots());

    // TODO: build BDD for the specified net here
    CirGateV left;
    CirGateV right;
    for (unsigned i = 0; i < orderedGates.size(); ++i) {
        if (orderedGates[i]->getType() == AIG_GATE) {
            // build fanin
            left = orderedGates[i]->getIn0();
            right = orderedGates[i]->getIn1();
            BddNodeV newNode = ((left.isInv()) ? ~bddMgrV->getBddNodeV(left.gateId())
                                               : bddMgrV->getBddNodeV(left.gateId())) &
                               ((right.isInv()) ? ~bddMgrV->getBddNodeV(right.gateId())
                                                : bddMgrV->getBddNodeV(right.gateId()));
            bddMgrV->addBddNodeV(orderedGates[i]->getGid(), newNode());
        }
        // PO, RI
        else if ((orderedGates[i]->getType() == RI_GATE) || (orderedGates[i]->getType() == PO_GATE)) {
            CirGateV in0 = orderedGates[i]->getIn0();
            BddNodeV newNode = (in0.isInv()) ? ~bddMgrV->getBddNodeV(in0.gateId()) : bddMgrV->getBddNodeV(in0.gateId());
            bddMgrV->addBddNodeV(orderedGates[i]->getGid(), newNode());
        }
    }
}

