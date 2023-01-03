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

Vec_Ptr_t * GV_NtkDfsNodesReverse( Abc_Ntk_t * pNtk, Abc_Obj_t ** ppNodes, int nNodes )
{
    Vec_Ptr_t * vNodes;
    Abc_Obj_t * pObj, * pFanout;
    int i, k;
   //  assert( Abc_NtkIsStrash(pNtk) );
    // set the traversal ID
    Abc_NtkIncrementTravId( pNtk );
    // start the array of nodes
    vNodes = Vec_PtrAlloc( 100 );
    for ( i = 0; i < nNodes; i++ )
    {
        pObj = ppNodes[i];
      //   assert( Abc_ObjIsCi(pObj) );
        Abc_NodeSetTravIdCurrent( pObj );
      //   pObj = Abc_ObjFanout0Ntk(pObj);
        Abc_ObjForEachFanout( pObj, pFanout, k )
            Abc_NtkDfsReverseNodes_rec( pFanout, vNodes );
    }
    return vNodes;
}

void
GVRTLConeMgr::genConelist() {
    vector<RTLIL::Module *> modules = _design->getDesign()->selected_modules();

    for(auto module : modules) {

        SigMap sigmap(module);

        // record PI
        pool<SigBit> input_bits;
        for (auto wire : module->wires())
          if (wire->port_input)
              for (auto bit : sigmap(wire))
                  input_bits.insert(bit);
        
        // index: from sigbit to driven cells
        dict<SigBit, pool<Cell*> > bit2cells;

        // index: from cell to driven sigbits
        dict<Cell*, pool<SigBit> > cell2bits;

        for (auto cell : module->cells()){
          log("%s\n", log_id(cell));
          for (auto &conn : cell->connections()) {
              log("\t  %s\t->\t", log_id(conn.first));
              log("  %s\n", log_id(conn.second.as_wire()));
              if (cell->input(conn.first)) {
                  for (auto bit : sigmap(conn.second))
                      bit2cells[bit].insert(cell);
              }
              if (cell->output(conn.first)) {
                  for (auto bit : sigmap(conn.second))
                      cell2bits[cell].insert(bit);
              }
          }
        }

        pool<SigBit> queue = input_bits;
        pool<Cell*> visited_cells;

        while (!queue.empty())
        {
            log("------\n");

            pool<Cell*> this_iter_cells;

            for (auto bit : queue)
            for (auto cell : bit2cells[bit])
                if (!visited_cells.count(cell)) {
                    log("  %s\n", log_id(cell));
                    visited_cells.insert(cell);
                    this_iter_cells.insert(cell);
                }

            queue.clear();
            for (auto cell : this_iter_cells)
            for (auto bit : cell2bits[cell])
                queue.insert(bit);
        }
      }
}

void
GVRTLCone::runTfiDfs() {
  // SigMap sigmap(module);
}

int
GVAbcCone::genHash() {
  Abc_Obj_t* pNode;
  int i;
  Abc_NtkIncrementTravId(_rootNode->pNtk);
  for(i=0; i<_TFIlist.size(); ++i) {
    Abc_NodeSetTravIdCurrent(_TFIlist[i]);
  }
  for(i=0; i< _TFOlist.size(); ++i) {
    Abc_NodeSetTravIdCurrent(_TFOlist[i]);
  }

  char* pName;

  Abc_NtkForEachCi(_rootNode->pNtk, pNode, i) {
    pName = Abc_ObjName(pNode);
    if (Abc_NodeIsTravIdCurrent(pNode)){
      // do the marking thing
      if(gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
        gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size() + 1;
      }
      _structuralHash += 2 ^ gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)];
    }
  }

  Abc_NtkForEachPo(_rootNode->pNtk, pNode, i) {
    pName = Abc_ObjName(pNode);
    if (Abc_NodeIsTravIdCurrent(pNode)){
      // do the marking thing
      if(gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
        gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size() + 1;
      }
      _structuralHash += 2 ^ gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)];
    }
  }
  return _structuralHash;
}

#endif // GV_CONE_CPP