/****************************************************************************
  FileName     [ gvNtk.cpp ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ GV Network ]
  Author       [ ]
  Copyright    [ ]
****************************************************************************/
#include "gvNtk.h"
#include "gvMsg.h"
#include "util.h"
#include "map"
#include "base/abc/abc.h"

// extern functions
extern "C"
{
  Gia_Man_t * Wln_BlastSystemVerilog( char * pFileName, char * pTopModule, char * pDefines, int fSkipStrash, int fInvert, int fTechMap, int fVerbose );
}
GVRTLDesign* gvRTLDesign;
GVNtkMgr*    gvNtkMgr;

// GV Ntk defines
#define isGVNetInverted(netId) (netId.cp)

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
//----------------------------------------------------------------------
// constructor / deconstructor for GVNtkMgr
//----------------------------------------------------------------------
void
GVNtkMgr::reset() {
    // GV
    _InputList.clear();
    _OutputList.clear();
    _InoutList.clear();
    _FFList.clear();
    _ConstList.clear();
    _numNets = 0;
    _netId2Name.clear();
    // yosys
    PI_PO_FF_var.clear();
    cellName2Pointer.clear();
    wire_from.clear();
    wire_to.clear();
    cell_fanin_connect_to_PI.clear();
    cell_fanout_connect_to_PO.clear();
    cell_fanin.clear();
    cell_fanout.clear();
}

//----------------------------------------------------------------------
// mapping
//----------------------------------------------------------------------
void
GVNtkMgr::mapConnectionFromYosys() {
    // get yosys_design pointer from GVRTLDesign
    RTLIL::Design*                            design = gvRTLDesign->getDesign();
    RTLIL::Module*                            top_module = design->top_module();
    // map wire branch to the head
    SigMap                                    sigmap(top_module);
    TopoSort<IdString, RTLIL::sort_by_id_str> toposort;
    // sigmap for PI, PO
    // cout << "=============================" << endl;
    for (auto wire : top_module->wires()) {
        // isPublic(): if its name contains "\" (PI, PO, reg)
        if (wire->name.isPublic()) {
            PI_PO_FF_var.push_back(wire);
            // cout << "PI / PO / reg -->  " << wire->name.c_str() << endl;
        }
        // cout << "=============================\n\n" << endl;
        // sigmap for wire-cell LUT
        // connect to PI/PO/reg (var)
        for (auto cell : top_module->cells()) {
            // map name to type
            cellName2Pointer[cell->name] = cell;
            // map connection
            for (auto conn : cell->connections()) {
                if (cell->input(conn.first)) { // portname: ID::A, ID::B
                    for (auto bit : sigmap(conn.second)) { // wire name
                        for (auto var : PI_PO_FF_var) {
                            if (bit == sigmap(SigSpec(var).bits()[0])) {
                                cell_fanin_connect_to_PI[cell->name].insert(
                                    bit.wire->name);
                                // cout << "=============================" <<
                                // endl; cout << "cell name (to PI)  --> "
                                //      << cell->name.c_str() << endl;
                                // cout << "cell type (PI)  -->  "
                                //      << cell->type.c_str() << endl;
                                // cout << "PI name  -->  " << var->name.c_str()
                                //      << endl;
                                // cout << "fanin bit : " <<
                                // bit.wire->name.c_str()
                                //      << endl;
                                // cout << "=============================" <<
                                // endl;
                            }
                        }
                        wire_to[bit].insert(cell->name);
                        cell_fanin[cell->name].insert(bit);
                        // cout << "cell type (PI)  -->  " << cell->type.c_str()
                        //      << endl;
                        // cout << "fanin bit : " << bit.wire->name.c_str()
                        //      << endl;
                        // cout << "\n" << endl;
                    }
                } else if (cell->output(conn.first)) { // portname: ID::Y
                    for (auto bit : sigmap(conn.second)) {
                        for (auto var : PI_PO_FF_var) {
                            if (bit == sigmap(SigSpec(var).bits()[0])) {
                                cell_fanout_connect_to_PO[cell->name].insert(
                                    bit.wire->name);
                                // cout << "=============================" <<
                                // endl; cout << "cell name (to PO)  --> "
                                //      << cell->name.c_str() << endl;
                                // cout << "cell type (PO)  -->  "
                                //      << cell->type.c_str() << endl;
                                // cout << "PO name  -->  " << var->name.c_str()
                                //      << endl;
                                // cout
                                //     << "fanout bit : " <<
                                //     bit.wire->name.c_str()
                                //     << endl;
                                // cout << "=============================" <<
                                // endl;
                            }
                        }
                        wire_from[bit].insert(cell->name);
                        cell_fanout[cell->name].insert(bit);
                        // cout << "cell type (PO)  -->  " << cell->type.c_str()
                        //      << endl;
                        // cout << "fanout bit : " << bit.wire->name.c_str()
                        //      << endl;
                        // cout << "\n" << endl;
                    }
                }
                // sort in topological order
                toposort.node(cell->name);
            }
        }
        // print bit_user / bit_driver
        /*
        for (auto& it : wire_to) {
            if (wire_from.count(it.first)) {
                for (auto driver_cell : wire_from.at(it.first)) {
                    for (auto user_cell : it.second) {
                        // cout << "=============================" << endl;
                        // cout << "wire --> " << it.first.wire->name.c_str()
                        //      << endl; // match not only PI/PO, but also
                        //               // internal sig
                        // cout << "from (cell name) --> " <<
                        // driver_cell.c_str()
                        //      << endl;
                        // cout << "to (cell name) --> " << user_cell.c_str()
                        //      << endl;
                        // cout << "=============================" << endl;
                        toposort.edge(driver_cell, user_cell);
                    }
                }
            }
        }
        toposort.analyze_loops = true;
        toposort.sort();
        */

        // --- Prototype Start --- 
        // key = cell name, value = GVNetID
        map<string,GVNetId> cell2NetID;
        // resize the inputData to the number of wires
        _inputData.resize(wire_to.size() + cell_fanin_connect_to_PI.size() + cell_fanout_connect_to_PO.size());
        cout << "Wire to size : " <<  wire_to.size() << endl;
        cout << "Wire from size : " <<  wire_from.size() << endl;
        cout << "cell_fanin_connect_to_PI size : " <<  cell_fanin_connect_to_PI.size() << endl;
        cout << "cell_fanout_connect_to_PO size : " <<  cell_fanout_connect_to_PO.size() << endl;
        // for new id
        int size = 0;
        for (auto& it : wire_to) {
            if (wire_from.count(it.first)) {
                for (auto driver_cell : wire_from.at(it.first)) {
                    for (auto user_cell : it.second) {
                        string currWireName = it.first.wire->name.c_str();
                        string fanOutNode = user_cell.c_str();
                        string fanInNode = driver_cell.c_str();

                        // fanout node is the new node
                        cout << cellName2Pointer[fanOutNode]->type.c_str() << endl;
                        cout << cellName2Pointer[fanInNode]->type.c_str() << endl;
                        
                        if(!cell2NetID.count(fanOutNode)){
                            GVNetId newId = GVNetId::makeNetId(size++);
                            // Gate type is "NOT"
                            if(cellName2Pointer[fanOutNode]->type == ID($_NOT_))
                                newId.type = GV_NTK_OBJ_NOT;
                            else if(cellName2Pointer[fanOutNode]->type != ID($_AND_)){
                                newId.type = GV_NTK_OBJ_FF;
                                cout << newId.id << " FF : "<< cellName2Pointer[fanOutNode]->type.c_str() << "\n";
                            }

                            cell2NetID[fanOutNode] = newId;
                        }
                        // fanin node is the new node
                        if(!cell2NetID.count(fanInNode)){
                            GVNetId newId = GVNetId::makeNetId(size++);
                            // Gate type is "NOT"
                            if(cellName2Pointer[fanInNode]->type == ID($_NOT_))
                                newId.type = GV_NTK_OBJ_NOT;
                            else if(cellName2Pointer[fanInNode]->type != ID($_AND_)){
                                newId.type = GV_NTK_OBJ_FF;
                                cout << newId.id << " FF : "<< cellName2Pointer[fanOutNode]->type.c_str() << "\n";
                            }
                            cell2NetID[fanInNode] = newId;
                        }

                        // fanin node's net id 
                        GVNetId fanInNodeId  =  cell2NetID[fanInNode];
                        // fanout node's net id 
                        GVNetId fanOutNodeId =  cell2NetID[fanOutNode];
                        // update the input date table: fanoutNodeId -> faninNodeId
                        _inputData[fanOutNodeId.id].push_back(fanInNodeId);

                        // === Debug Area Start ===
                        cout << " Wire   name    : " << currWireName << "\n";
                        cout << " Fanout name    : " << fanOutNode << "\n";
                        cout << " Fanin  name    : " << fanInNode << "\n";
                        cout << " Fanout Node Id : " << fanOutNodeId.id << "\n";
                        cout << " Fanin  Node Id : " << fanInNodeId.id << "\n";
                        // cout << " Input  Data Id : " << _inputData[fanOutNodeId.id][0].id << "\n";
                        // === Debug Area End === 
                        //return;
                    }
                }
            }
        }

        // Start to connect PI to node(s)
        for(auto &gate: cell_fanin_connect_to_PI){
            string currNode = gate.first.c_str();
            // if the current node have not been traversal.
            if(!cell2NetID.count(currNode)){
                GVNetId newId = GVNetId::makeNetId(size++);
                cell2NetID[currNode] = newId;
            }
            for(auto pi: gate.second){
                string piNode = pi.c_str(); 
                if(!cell2NetID.count(piNode)){
                    GVNetId newId = GVNetId::makeNetId(size++);
                    cell2NetID[piNode] = newId;
                }
                // current node net id 
                GVNetId currNodeId = cell2NetID[currNode];
                // PI node net id 
                GVNetId piNodeId = cell2NetID[piNode];
                _inputData[currNodeId.id].push_back(piNodeId);
                cout << "Update input data -> currNode Id : " << currNodeId.id << " piNodeId : " << piNodeId.id << "\n";
            }
        }

        // Start to connect PO to node(s)
        for(auto &gate: cell_fanout_connect_to_PO){
            string currNode = gate.first.c_str();
            // if the current node have not been traversal.
            if(!cell2NetID.count(currNode)){
                GVNetId newId = GVNetId::makeNetId(size++);
                cell2NetID[currNode] = newId;
            }
            for(auto po: gate.second){
                string poNode = po.c_str(); 
                if(!cell2NetID.count(poNode)){
                    GVNetId newId = GVNetId::makeNetId(size++);
                    cell2NetID[poNode] = newId;
                }
                // current node net id 
                GVNetId currNodeId = cell2NetID[currNode];
                // PI node net id 
                GVNetId poNodeId = cell2NetID[poNode];
                _inputData[poNodeId.id].push_back(currNodeId);
                cout << "Update input data -> currNode Name : " << currNode << " poNode Name : " << poNode<< "\n";
                cout << "Update input data -> currNode Id : " << currNodeId.id << " poNodeId : " << poNodeId.id << "\n";
            }
        }

        // Start to collapse "NOT" gate
        for(int i = 0; i < _inputData.size(); ++i){
            cout << " Fanout Node : " << i << "\n";
            for(int j = 0;j < _inputData[i].size(); ++j){
                // if type is "NOT", remove it and reconnect the wire
                if(_inputData[i][j].type == GV_NTK_OBJ_NOT){
                    int notId = _inputData[i][j].id;
                    assert(_inputData[notId].size() > 0);
                    GVNetId notFanin = _inputData[notId][0];
                    // Invert the "cp" of "NOT" gate's fanin node
                    // and Reconnect the wire
                    _inputData[i][j] = ~notFanin;
                    cout << "NOT gate fanin : " << _inputData[notId][0].id << endl; 
                }
                cout << " Fanin Node : " << _inputData[i][j].id << " Type : " << _inputData[i][j].type 
                << " cp : " << _inputData[i][j].cp << "\n";
            }
            cout << " ----- \n";
        }
        
        cout << "\n Input Data Info : \n";
        for(int i = 0; i < _inputData.size(); ++i){
            cout << " Fanout Node : " << i << "\n";
            for(int j = 0;j < _inputData[i].size(); ++j){
                cout << " Fanin Node : " << _inputData[i][j].id << " Type : " << _inputData[i][j].type 
                << " cp : " << _inputData[i][j].cp << "\n";            }
            cout << " ----- \n";
        }
        
        // --- Prototype End ---
    }
    // cout << "=============================\n\n" << endl;
    // sigmap for wire-cell LUT
    // connect to PI/PO/reg (var)
    for (auto cell : top_module->cells()) {
        // map name to type
        cellName2Pointer[cell->name] = cell;
        // map connection
        for (auto conn : cell->connections()) {
            if (cell->input(conn.first)) {             // portname: ID::A, ID::B
                for (auto bit : sigmap(conn.second)) { // wire name
                    for (auto var : PI_PO_FF_var) {
                        // cout << "HHHHHHHH    -->   "
                        //      << sigmap(SigSpec(var).bits()).size() << endl;
                        if (bit == sigmap(SigSpec(var).bits()[0])) {
                            cell_fanin_connect_to_PI[cell->name].insert(
                                bit.wire->name);
                            // cout << "=============================" <<
                            // endl; cout << "cell name (to PI)  --> "
                            //      << cell->name.c_str() << endl;
                            // cout << "cell type (PI)  -->  "
                            //      << cell->type.c_str() << endl;
                            // cout << "PI name  -->  " << var->name.c_str()
                            //      << endl;
                            // cout << "fanin bit : " <<
                            // bit.wire->name.c_str()
                            //      << endl;
                            // cout << "=============================" <<
                            // endl;
                        }
                    }
                    wire_to[bit].insert(cell->name);
                    if (cell->type.in(ID($_AND_), ID($_NOT_))) {
                        cell_fanin[cell->name].insert(bit);
                    } else {                       // FF
                        if (conn.first == ID::D) { // don't care "\clk", "\rst"
                            cell_fanin[cell->name].insert(bit);
                        }
                    }
                }
            } else if (cell->output(conn.first)) { // portname: ID::Y
                bool PIO = false;
                for (auto bit : sigmap(conn.second)) {
                    for (auto var : PI_PO_FF_var) {
                        if (bit == sigmap(SigSpec(var).bits()[0])) {
                            for (int i = 0; i < SigSpec(var).bits().size();
                                 ++i) {
                                cout
                                    << "LLLL  -->  "
                                    << SigSpec(var).bits()[i].wire->name.c_str()
                                    << endl;
                            }
                            cell_fanout_connect_to_PO[cell->name].insert(
                                bit.wire->name);
                            PIO = true;
                            // cout << "=============================" <<
                            // endl; cout << "cell name (to PO)  --> "
                            //      << cell->name.c_str() << endl;
                            // cout << "cell type (PO)  -->  "
                            //      << cell->type.c_str() << endl;
                            // cout << "PO name  -->  " << var->name.c_str()
                            //      << endl;
                            // cout
                            //     << "fanout bit : " <<
                            //     bit.wire->name.c_str()
                            //     << endl;
                            // cout << "=============================" <<
                            // endl;
                        }
                    }
                    wire_from[bit].insert(cell->name);
                    cell_fanout[cell->name].insert(bit);
                    if (!cell->type.in(ID($_AND_), ID($_NOT_))) {
                        if ((conn.first == ID::Q)) {
                            PIO = false;
                            // cout << "==============" << endl;
                            // cout << "cell name : " << cell->name.c_str()
                            //      << endl;
                            // cout << "cell type : " << cell->type.c_str()
                            //      << endl;
                            // cout
                            //     << "sigmap PO name : " <<
                            //     bit.wire->name.c_str()
                            //     << endl;
                            // if (conn.second.is_wire()) {
                            //     cout << "slice PO name : "
                            //          << conn.second.as_wire()->name.c_str()
                            //          << endl;
                            // }
                            // cout << "==============" << endl;
                        }
                    }
                }
            }
            // sort in topological order
            toposort.node(cell->name);
        }
    }
    // print bit_user / bit_driver
    for (auto& it : wire_to) {
        if (wire_from.count(it.first)) {
            for (auto driver_cell : wire_from.at(it.first)) {
                for (auto user_cell : it.second) {
                    // cout << "=============================" << endl;
                    // cout << "wire --> " << it.first.wire->name.c_str()
                    //      << endl; // match not only PI/PO, but also
                    //               // internal sig
                    // cout << "from (cell name) --> " <<
                    // driver_cell.c_str()
                    //      << endl;
                    // cout << "to (cell name) --> " << user_cell.c_str()
                    //      << endl;
                    // cout << "=============================" << endl;
                    toposort.edge(driver_cell, user_cell);
                }
            }
        }
    }
    toposort.analyze_loops = true;
    toposort.sort();
    return;
}

//----------------------------------------------------------------------
// construct GV network
//----------------------------------------------------------------------
void
GVNtkMgr::createNetFromYosys() {
    // get yosys_design pointer from GVRTLDesign
    RTLIL::Design* design     = gvRTLDesign->getDesign();
    RTLIL::Module* top_module = design->top_module();

    // PI
    for (auto wire : top_module->wires()) {
        if (wire->port_input) {
            // cout << "width      -->  " << GetSize(wire) << endl;
            for (size_t i = 0; i < GetSize(wire); i++) {
                GVNetId id = GVNetId::makeNetId(
                    _numNets++); // create the net for the PI and increase the
                                 // number of total nets by 1
                createNet(id, GV_NTK_OBJ_PI); // create the input for GVNtk
                // map GVNetId to wire name
                string multi_bit_wire =
                    wire->name.c_str() + '[' + to_string(i) + ']';
                _netId2Name[id.id] = wire->name.c_str();
            }
        }
    }

    // PO
    for (auto wire : top_module->wires()) {
        if (wire->port_output) {
            for (size_t i = 0; i < GetSize(wire); i++) {
                GVNetId id = GVNetId::makeNetId(
                    _numNets++); // create the net for the PO and increase the
                                 // number of total nets by 1
                createNet(id, GV_NTK_OBJ_PO); // create the input for GVNtk
                // map GVNetId to wire name
                _netId2Name[id.id] = wire->name.c_str();
            }
        }
    }

    // FF
    for (auto cell : top_module->cells()) {
        if (!cell->type.in(ID($_AND_), ID($_NOT_))) {
            for (auto fanin : cell_fanin[cell->name]) {
                for (size_t i = 0; i < GetSize(fanin.wire); i++) {
                    // Q: current state
                    GVNetId id_Q = GVNetId::makeNetId(
                        _numNets++); // create the net for the FF and increase
                                     // the number of total nets by 1
                    createNet(id_Q,
                              GV_NTK_OBJ_FF); // create the input for GVNtk
                                              // map GVNetId to wire name
                    string cur_name      = fanin.wire->name.c_str();
                    _netId2Name[id_Q.id] = cur_name;
                    // D: next state
                    GVNetId id_D = GVNetId::makeNetId(
                        _numNets++); // create the net for the FF and increase
                                     // the number of total nets by 1
                    createNet(id_D,
                              GV_NTK_OBJ_FF); // create the input for GVNtk
                                              // map GVNetId to wire name
                    string ns_name       = cur_name.append("_ns");
                    _netId2Name[id_D.id] = ns_name;
                }
            }
        }
    }

    // for (auto& it : _netId2Name) {
    //     cout << "id / name --> " << it.first << " / " <<
    //     _netId2Name[it.first]
    //          << endl;
    // }

    cout << "GV NTK stats" << endl;
    cout << "# PI's : " << getInputSize() << endl;
    cout << "# PO's : " << getOutputSize() << endl;
    cout << "# FF's : " << getFFSize() << endl;
}

// create net
void
GVNtkMgr::createNet(const GVNetId& id, const int net_type) {
    assert(!isGVNetInverted(id));
    if (net_type == GV_NTK_OBJ_PI) {
        _InputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_PO) {
        _OutputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_FF) {
        _FFList.push_back(id);
    }
    return;
}

// recursively print the gia network
void print_rec(Gia_Man_t * pGia, Gia_Obj_t * pObj)
{
  if(Gia_ObjIsTravIdCurrent(pGia, pObj)) return; // if the TravId of the node is equal to the global TravId, return
  if(Gia_ObjIsPi(pGia, pObj)) cout << "Pi reached, id = " << Gia_ObjId(pGia, pObj) << " " << endl;
  if(Gia_ObjIsRo(pGia, pObj)) cout << "Ro reached, id = " << Gia_ObjId(pGia, pObj) << endl;
  Gia_ObjSetTravIdCurrent(pGia, pObj); // set the TravId of the node to be the same as the global TravId, that is mark it as traversed
  
  if(Gia_ObjIsCi(pObj)) return; // If we reach the combinational input(PI + Ro(register output, or pseudo PI)), return.
  cout << "node id = " << Gia_ObjId(pGia, pObj) << " num fanin = " << Gia_ObjFaninNum(pGia, pObj ) << endl; // print the gia Id of the current node
  cout << "\tfanin-0 id = " << Gia_ObjId(pGia, Gia_ObjFanin0(pObj)) << endl; // print the id of its left child
  cout << "\tfanin-1 id = " << Gia_ObjId(pGia, Gia_ObjFanin1(pObj)) << endl; // print the id of its right child

  if(Gia_ObjFanin0(pObj)) print_rec(pGia, Gia_ObjFanin0(pObj)); // recursive traverse the left child
  if(Gia_ObjFanin1(pObj)) print_rec(pGia, Gia_ObjFanin1(pObj)); // recursive traverse the right child
}

void
GVNtkMgr::createNetFromAbc(char * pFileName) {
  Gia_Man_t * pGia = NULL; // the gia pointer of abc
  Gia_Obj_t * pObj;        // the obj element of gia

  // abc function parameters
  char * pTopModule= NULL; // the top module can be auto detected by yosys, no need to set
  char * pDefines  = NULL;
  int fBlast       =    1; // blast the ntk to gia (abc's aig data structure)
  int fInvert      =    0;
  int fTechMap     =    1;
  int fSkipStrash  =    0;
  int fCollapse    =    0;
  int c, fVerbose  =    1; // set verbose to 1 to see which yosys command is used
  int i, * pWire;

  // read and blast the RTL verilog file into gia
  pGia = Wln_BlastSystemVerilog( pFileName, pTopModule, pDefines, fSkipStrash, fInvert, fTechMap, fVerbose );
  
  // increment the global travel id for circuit traversing usage 
  Gia_ManIncrementTravId(pGia);
  // since we don't want to traverse the constant node, set the TravId of the constant node to be as the global one
  Gia_ObjSetTravIdCurrent( pGia, Gia_ManConst0(pGia) );

  // dfs traverse from each combinational output (Po(primary output) and Ri(register input, which can be understand as pseudo Po))
  Gia_ManForEachCo(pGia, pObj, i)
  {
    print_rec(pGia, Gia_ObjFanin0(pObj)); // we get the fanin of Co. you can imagine that the po net is simply an one bit buf
  }
}