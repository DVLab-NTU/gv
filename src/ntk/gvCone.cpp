/****************************************************************************
  FileName     [ gvCone.cpp ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ Define RTL logic cone manager ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2022-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_CONE_CPP
#define GV_CONE_CPP

#include "gvCone.h"

//----------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------
GVConeComparitor gvConeComparitor;

Vec_Ptr_t*
GV_NtkDfsNodesReverse(Abc_Ntk_t* pNtk, Abc_Obj_t** ppNodes, int nNodes) {
    Vec_Ptr_t* vNodes;
    Abc_Obj_t *pObj, *pFanout;
    int        i, k;
    //  assert( Abc_NtkIsStrash(pNtk) );
    // set the traversal ID
    Abc_NtkIncrementTravId(pNtk);
    // start the array of nodes
    vNodes = Vec_PtrAlloc(100);
    for (i = 0; i < nNodes; i++) {
        pObj = ppNodes[i];
        //   assert( Abc_ObjIsCi(pObj) );
        Abc_NodeSetTravIdCurrent(pObj);
        //   pObj = Abc_ObjFanout0Ntk(pObj);
        Abc_ObjForEachFanout(pObj, pFanout, k) Abc_NtkDfsReverseNodes_rec(pFanout, vNodes);
    }
    return vNodes;
}

/**Function*************************************************************

  Synopsis    [Assigns random simulation info to the PIs.]

  Description [from abc/src/opt/sim/simUtils.c]

  SideEffects []

  SeeAlso     []

***********************************************************************/
void
Sim_UtilAssignRandom(Sim_Man_t* p) {
    Abc_Obj_t* pNode;
    unsigned*  pSimInfo;
    int        i, k;
    // assign the random/systematic simulation info to the PIs
    Abc_NtkForEachCi(p->pNtk, pNode, i) {
        pSimInfo = (unsigned*)p->vSim0->pArray[pNode->Id];
        for (k = 0; k < p->nSimWords; k++) pSimInfo[k] = SIM_RANDOM_UNSIGNED;
    }
}

void
GVRTLConeMgr::genConelist() {
    vector<RTLIL::Module*> modules = _design->getDesign()->selected_modules();

    for (auto module : modules) {

        // use Sigmap to convert nets to a unique representation
        SigMap sigmap(module);

        // create cone node for every signal and record PI/PO
        pool<SigBit>         input_bits;
        pool<SigBit>         output_bits;
        dict<SigBit, string> bit_names;
        for (auto wire : module->selected_wires()) {
            cout << wire->name.str();
            SigSpec wire_spec = sigmap(wire);
            for (int i = 0; i < wire_spec.size(); ++i) {
                SigBit bit     = wire_spec[i];
                _coneList[bit] = new GVRTLCone(bit);
                if (wire->port_input || wire->port_output) {
                    if (wire->port_input) input_bits.insert(bit), cout << " (pi):" << log_signal(bit);
                    else output_bits.insert(bit), cout << " (po):" << log_signal(bit);
                    bit_names[bit] = rmSpaceBackslash(log_id(wire));
                    if (wire->width > 1) {
                        bit_names[bit] = bit_names[bit] + "[" + to_string(i) + "]";
                    }
                }
            }
            cout << endl;
        }

        // index: from sigbit to driven cells
        dict<SigBit, pool<Cell*>> bit2cells;
        dict<SigBit, pool<Cell*>> bit2cells_rev;

        // index: from cell to driven sigbits
        dict<Cell*, pool<SigBit>> cell2bits;
        dict<Cell*, pool<SigBit>> cell2bits_rev;

        for (auto cell : module->cells()) {
            log("%s\n", log_id(cell));
            for (auto& conn : cell->connections()) {
                log("\t  %s\t->\t", log_id(conn.first));
                log("  %s\n", log_signal(conn.second));
                if (cell->input(conn.first)) {
                    for (auto bit : sigmap(conn.second)) {
                        if (bit.wire == NULL) continue;
                        bit2cells[bit].insert(cell);
                        cell2bits_rev[cell].insert(bit);
                    }
                }
                if (cell->output(conn.first)) {
                    for (auto bit : sigmap(conn.second)) {
                        if (bit.wire == NULL) continue;
                        cell2bits[cell].insert(bit);
                        bit2cells_rev[bit].insert(cell);
                    }
                }
            }
        }

        // PI to Po way
        pool<SigBit> queue;
        for (auto pi : input_bits) {
            queue.insert(pi);
            pool<Cell*> visited_cells;

            log("\n---pi---\n%s\n", bit_names[pi].c_str());
            while (!queue.empty()) {
                pool<Cell*> this_iter_cells;

                for (auto bit : queue) {
                    for (auto cell : bit2cells[bit]) {
                        if (!visited_cells.count(cell)) {
                            // log("  %s\n", log_id(cell));
                            visited_cells.insert(cell);
                            this_iter_cells.insert(cell);
                        }
                    }
                }

                queue.clear();
                for (auto cell : this_iter_cells)
                    for (auto bit : cell2bits[cell]) {
                        queue.insert(bit);
                        _coneList[bit]->setTfiList(bit_names[pi]);
                        log("  %s,", log_signal(bit));
                    }
            }
        }

        for (auto po : output_bits) {
            queue.insert(po);
            pool<Cell*> visited_cells;
            log("\n---po---\n%s\n", bit_names[po].c_str());
            while (!queue.empty()) {
                pool<Cell*> this_iter_cells;

                for (auto bit : queue) {
                    for (auto cell : bit2cells_rev[bit]) {
                        if (!visited_cells.count(cell)) {
                            visited_cells.insert(cell);
                            this_iter_cells.insert(cell);
                        }
                    }
                }

                queue.clear();
                for (auto cell : this_iter_cells)
                    for (auto bit : cell2bits_rev[cell]) {
                        queue.insert(bit);
                        if (_coneList[bit]) _coneList[bit]->setTfoList(bit_names[po]); // link to po
                        log("  %s,", log_signal(bit));
                    }
            }
        }
    }

    genAllHash();
}

int
GVAbcCone::genHash() {
    Abc_Obj_t* pNode;
    int        i;
    string     pName;

    Abc_NtkForEachPi(_rootNode->pNtk, pNode, i) {
        pName = Abc_ObjName(pNode);
        if (checkInTFI(pNode)) {
            // do the marking thing
            if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
                gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
            }
            _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[pName]);
        }
    }

    Abc_NtkForEachPo(_rootNode->pNtk, pNode, i) {
        pName = Abc_ObjName(pNode);
        if (checkInTFO(pNode)) {
            // do the marking thing
            if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
                gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
            }
            _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[pName]);
        }
    }
    return _structuralHash;
}

int
GVRTLCone::genHash() {
    // coulde have be done in RTL gen conelist
    for (auto pi : _TFIlist) {
        // cout << "\t(pi) " << pi << endl;
        if (gvConeComparitor._mapOfPiPo.find(pi) == gvConeComparitor._mapOfPiPo.end()) {
            gvConeComparitor._mapOfPiPo[pi] = gvConeComparitor._mapOfPiPo.size();
        }
        _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[pi]);
    }
    for (auto po : _TFOlist) {
        // cout << "\t(po) " << po << endl;
        if (gvConeComparitor._mapOfPiPo.find(po) == gvConeComparitor._mapOfPiPo.end()) {
            gvConeComparitor._mapOfPiPo[po] = gvConeComparitor._mapOfPiPo.size();
        }
        _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[po]);
    }
    return _structuralHash;
}

void
GVAbcConeMgr::runFirstStepSim() {

    Vec_Ptr_t* vResult;
    abctime    clk;
    Abc_Obj_t* pNode;

    srand(0xEDA);

    // start the simulation manager
    _simMgr            = Sim_ManStart(_design, 0);
    _simMgr->nSimWords = 1;

    Sim_UtilAssignRandom(_simMgr);
    clk = Abc_Clock();
    Sim_UtilSimulate(_simMgr, 0);
    _simMgr->timeSim = Abc_Clock() - clk;
}

void
GVAbcConeMgr::runSecondStepSim() {
    Abc_Obj_t* pNode;
    int        i, fType0, fType1;
    unsigned * pSimInfo1, *pSimInfo2;
    unsigned   sum;

    for (auto cone : _coneList) {
        cout << "========\n" << Abc_ObjName(cone.first) << endl;

        sum                 = 0;
        Vec_Ptr_t* vSim1Old = _simMgr->vSim1;
        _simMgr->vSim1      = Sim_UtilInfoAlloc(Abc_NtkObjNumMax(_design), _simMgr->nSimWords, 0);
        Sim_UtilInfoFree(vSim1Old);

        Sim_UtilInfoFlip(_simMgr, cone.first);

        Abc_NtkForEachNode(_design, pNode, i) {
            if (!cone.second->checkInTFO(pNode) || pNode == cone.first) continue;
            fType0 = cone.second->checkInTFO(Abc_ObjFanin0(pNode));
            fType1 = cone.second->checkInTFO(Abc_ObjFanin1(pNode));
            Sim_UtilSimulateNode(_simMgr, pNode, 1, fType0, fType1);
            pSimInfo1 = (unsigned*)_simMgr->vSim0->pArray[pNode->Id];
            pSimInfo2 = (unsigned*)_simMgr->vSim1->pArray[pNode->Id];
            // if (pSimInfo1[0] != pSimInfo2[0]) {
            cout << " " << Abc_ObjName(pNode) << ": " << pSimInfo1[0] << "->" << pSimInfo2[0] << endl; //}
            cout << " f0:" << fType0 << ", f1:" << fType1 << endl;
            cout << " b0:" << cone.second->checkInTFO(Abc_ObjFanin0(pNode)) << ", b1:" << cone.second->checkInTFO(Abc_ObjFanin1(pNode)) << endl;
        }
        Abc_NtkForEachPo(_design, pNode, i) {
            if (!cone.second->checkInTFO(Abc_ObjFanin0(pNode))) continue;
            cout << Abc_ObjName(pNode) << " : " << Abc_ObjName(Abc_ObjFanin0(pNode)) << endl;
            Sim_UtilSimulateNode(_simMgr, pNode, 1, 1, 1);
            int k;
            pSimInfo1 = (unsigned*)_simMgr->vSim0->pArray[pNode->Id];
            pSimInfo2 = (unsigned*)_simMgr->vSim1->pArray[pNode->Id];
            for (k = 0; k < _simMgr->nSimWords; k++) {
                if (pSimInfo2[k] != pSimInfo1[k]) {
                    sum += pSimInfo2[k];
                    cout << " " << Abc_ObjName(pNode) << ": " << pSimInfo1[k] << "->" << pSimInfo2[k] << endl;
                }
            }
        }
    }
}

void
GVAbcConeMgr::runSecondStepSim(GVAbcCone* pCone) {
    const vector<Abc_Obj_t*> tfoList = pCone->getTfo();
    Abc_Obj_t*               pNode;
    int                      fType0, fType1, i, sum;

    Sim_UtilInfoFlip(_simMgr, (Abc_Obj_t*)pCone->getRootNode());

    for ( i = 0; i < tfoList.size(); ++i) {
        pNode  = tfoList[i];
        if (!pNode) continue;
        fType0 = Abc_NodeIsTravIdCurrent(Abc_ObjFanin0(pNode));
        fType1 = Abc_NodeIsTravIdCurrent(Abc_ObjFanin1(pNode));
        Sim_UtilSimulateNode(_simMgr, pNode, 1, fType0, fType1);
    }
    Abc_NtkForEachPo(_design, pNode, i) {
        if (!Abc_NodeIsTravIdCurrent(Abc_ObjFanin0(pNode))) continue;
        cout << Abc_ObjName(pNode) << " : " << Abc_ObjName(Abc_ObjFanin0(pNode)) << endl;
        Sim_UtilSimulateNode(_simMgr, pNode, 1, 1, 1);
        int k;
        unsigned* pSimInfo1 = (unsigned*)_simMgr->vSim0->pArray[pNode->Id];
        unsigned* pSimInfo2 = (unsigned*)_simMgr->vSim1->pArray[pNode->Id];
        for (k = 0; k < _simMgr->nSimWords; k++) {
            if (pSimInfo2[k] != pSimInfo1[k]) {
                sum += pSimInfo2[k];
                cout << " " << Abc_ObjName(pNode) << ": " << pSimInfo1[k] << "->" << pSimInfo2[k] << endl;
            }
        }
    }
}

void
structCandidate(GVRTLConeMgr* pRTLConeMgr, GVAbcConeMgr* pAbcConeMgr){
    map<Abc_Obj_t*, GVAbcCone*> abcConeList = pAbcConeMgr->getConeList();
    map<SigBit, GVRTLCone*> rtlConeList = pRTLConeMgr->getConeList();

    for (auto abcCone : abcConeList) {
        unsigned abcHash = abcCone.second->getHash();

        for (auto rtlCone : rtlConeList) {
            bitset<64> result = bitset<64>(rtlCone.second->getHash() ^ abcHash);

            if (result.count() <= gvConeComparitor.getPiPoNum() * 0.125) {
                cout << Abc_ObjName(abcCone.first) << "<->" << log_signal(rtlCone.first) << endl;
            }
        }
    }
}

#endif // GV_CONE_CPP