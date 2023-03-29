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
#include "bddMgrV.h"  // MODIFICATION FOR SoCV BDD
#include "bddNodeV.h" // MODIFICATION FOR SoCV BDD
#include "gvMsg.h"
#include "gvNtk.h"
#include "stack"
#include "util.h"

extern BddMgrV* bddMgrV; // MODIFICATION FOR SoCV BDD

const bool
GVNtkMgr::setBddOrder(const bool& file) {
    unsigned supportSize = getInputSize() + getInoutSize() + 2 * getFFSize();
    unsigned bddspsize   = bddMgrV->getNumSupports();
    if (supportSize >= bddMgrV->getNumSupports()) {
        gvMsg(GV_MSG_ERR)
            << "BDD Support Size is Smaller Than Current Design Required !!"
            << endl;
        return false;
    }
    // build support
    unsigned supportId = 1;
    for (unsigned i = 0, n = getInputSize(); i < n; ++i) {
        GVNetId nId     = (file) ? getInput(i) : getInput(n - i - 1);
        string  netName = getNetNameFromId(nId.id);
        bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
        bddMgrV->addBddNodeV(netName, bddMgrV->getSupport(supportId)());
        cout << " Name : " << netName  << "-> Id : " << nId.id << endl;
        cout << "Support Id : " << supportId << "\n\n";
        cout << " --- \n";
        ++supportId;
    }
    for (unsigned i = 0, n = getInoutSize(); i < n; ++i) {
        const GVNetId& nId     = (file) ? getInout(i) : getInout(n - i - 1);
        string         netName = getNetNameFromId(nId.id);
        bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
        bddMgrV->addBddNodeV(netName, bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    for (unsigned i = 0, n = getFFSize(); i < n; ++i) {
        const GVNetId& nId     = (file) ? getLatch(i) : getLatch(n - i - 1);
        string         netName = getNetNameFromId(nId.id);
        cout << "support id  -->  " << supportId << endl;
        // bddMgrV->addBddNodeV(nId.id, bddMgrV->getSupport(supportId)());
        bddMgrV->addBddNodeV(netName, bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    // Next State
    /*🥳
    for (unsigned i = 0, n = getFFSize(); i < n; ++i) {
        const GVNetId& nId     = (file) ? getLatch(i) : getLatch(n - i - 1);
        string         netName = getNetNameFromId(nId.id);
        bddMgrV->addBddNodeV(netName + "_ns", bddMgrV->getSupport(supportId)());
        ++supportId;
    }
    */

    // Constants
    /*for (uint32_t i = 0; i < getConstSize(); ++i) {
      assert(getGateType(getConst(i)) == AIG_FALSE);
      bddMgrV->addBddNodeV(getConst(i).id, BddNodeV::_zero());
    }*/

    return true;
}

void
GVNtkMgr::buildNtkBdd() {
    // TODO: build BDD for ntk here
    // Perform DFS traversal from DFF inputs, inout, and output gates.
    // Collect ordered nets to a GVNetVec
    // Construct BDDs in the DFS order

    stack<GVNetId> s;
    for (unsigned i = 0; i < getOutputSize(); ++i) {
        s.push(getOutput(i));
    }
    while (s.size() > 0) {
        buildBdd(s.top());
        s.pop();
    }
    // _isBddBuilt = true;

    // build next state
    cout << "getFFsize  -->  " << getFFSize() << endl;
    for (unsigned i = 0; i < getFFSize(); ++i) {
        GVNetId left = getLatch(i); // get RI
        if (bddMgrV->getBddNodeV(left.id) == (size_t)0) {
            buildBdd(left);
        }
       // BddNodeV ns = ((left.cp) ? ~bddMgrV->getBddNodeV(left.id)
       //                          : bddMgrV->getBddNodeV(left.id));
        bddMgrV->addBddNodeV(getLatch(i).id, bddMgrV->getBddNodeV(left.id)());
        // return; // debug
    }
    // BddNodeV test = bddMgrV->getBddNodeV(420);
    // cout << test << endlgit ;
}

void
GVNtkMgr::buildBdd(const GVNetId& netId) {
    // V3NetVec orderedNets;
    vector<GVNetId> orderedNets;

    orderedNets.clear();
    orderedNets.reserve(getNetSize());

    cout << "id visired " << netId.id << endl;

    newMiscData();
    dfsOrder(netId, orderedNets);
    cout << " Current Id : " << netId.id << "\n";
    cout << " Order Size : " << orderedNets.size() << "\n";
    cout << " Net Size : " << getNetSize() << "\n";
    assert(orderedNets.size() <= getNetSize());
    //cout << "size = " << orderedNets.size() << endl;

    // TODO: build BDD for the specified net here
    GVNetId left, right;
    for (unsigned i = 0; i < orderedNets.size(); ++i) {
        //cout << " Type : " << orderedNets[i].type << "\n";
        //cout << " ID : " << orderedNets[i].id << "\n";
        //cout << " cp : " << orderedNets[i].cp << "\n";
        //cout << " === \n";
        if (getGateType(orderedNets[i]) == GV_NTK_OBJ_AIG) {
            left  = getInputNetId(orderedNets[i], 0);
            right = getInputNetId(orderedNets[i], 1);
            if (bddMgrV->getBddNodeV(left.id) == (size_t)0) {
                //cout << bddMgrV->getBddNodeV(left.id) << endl;
                buildBdd(left);
            }
            if (bddMgrV->getBddNodeV(right.id) == (size_t)0) {
                buildBdd(right);
            }
            //BddNodeV newNode = ((left.cp) ? ~bddMgrV->getBddNodeV(left.id) : bddMgrV->getBddNodeV(left.id)) &
            //                   ((right.cp) ? ~bddMgrV->getBddNodeV(right.id) : bddMgrV->getBddNodeV(right.id));
            BddNodeV newNode = ((orderedNets[i].fanin0Cp) ? ~bddMgrV->getBddNodeV(left.id) : bddMgrV->getBddNodeV(left.id)) &
                               ((orderedNets[i].fanin1Cp) ? ~bddMgrV->getBddNodeV(right.id) : bddMgrV->getBddNodeV(right.id));
            bddMgrV->addBddNodeV(orderedNets[i].id, newNode());
            //cout << "New node : \n\n";
            //cout << newNode << endl;
        } else if(getGateType(orderedNets[i]) == GV_NTK_OBJ_FF 
                  || getGateType(orderedNets[i]) == GV_NTK_OBJ_PO){
            GVNetId fanin = getInputNetId(orderedNets[i], 0);
            BddNodeV newNode = (orderedNets[i].fanin0Cp)? ~bddMgrV->getBddNodeV(fanin.id) : bddMgrV->getBddNodeV(fanin.id);
            bddMgrV->addBddNodeV(orderedNets[i].id, newNode());
        }
    }
}

// put fanins of a net (id) into a vector (nets) in topological order
void
GVNtkMgr::dfsOrder(const GVNetId& id, vector<GVNetId>& nets) {
    if (isLatestMiscData(id)) return;
    //  Set Latest Misc Data
    //setLatestMiscData(id);
    setLatestMiscData(id);
    //  Traverse Fanin Logics
    const GV_Ntk_Type_t type = getGateType(id);
    //cout << "dfsorder type = " << type << endl;
    //cout << "dfsorder id = " << id.id << endl;
    if (type == GV_NTK_OBJ_FF || type == GV_NTK_OBJ_PO) {
        dfsOrder(getInputNetId(id, 0), nets);
    } else if (type == GV_NTK_OBJ_AIG) {
        dfsOrder(getInputNetId(id, 0), nets);
        dfsOrder(getInputNetId(id, 1), nets);
    }
    nets.push_back(id); // Record Order
}

#endif
