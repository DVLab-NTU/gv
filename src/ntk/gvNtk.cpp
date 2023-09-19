/****************************************************************************
  FileName     [ gvNtk.cpp ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ GV Network ]
  Author       [ ]
  Copyright    [ ]
****************************************************************************/
#include "gvNtk.h"
#include "base/abc/abc.h"
#include "gvMsg.h"
#include "map"
#include "util.h"
#include <fstream>

// extern functions
extern "C"
{
    Gia_Man_t* Wln_BlastSystemVerilog(char* pFileName, char* pTopModule,
                                      char* pDefines, int fSkipStrash,
                                      int fInvert, int fTechMap, int fVerbose);
}
// declaration
GVRTLDesign* gvRTLDesign;
GVNtkMgr*    gvNtkMgr;

// ----------------------------------------------------------------------
//  constructor / deconstructor for GVNtkMgr
// ----------------------------------------------------------------------
void
GVNtkMgr::reset() {
    // info
    _InputList.clear();
    _OutputList.clear();
    _InoutList.clear();
    _FFList.clear();
    _ConstList.clear();
    _FFConst0List.clear();
    // map
    _id2FaninId.clear();
    _id2GVNetId.clear();
    _netId2Name.clear();
    _netName2Id.clear();
    _idRo2Ppi.clear();
    _idRo2Ri.clear();
    _idRi2Ro.clear();
    _id2Type.clear();
    // fanout info
    _id2Fanout.clear();
    // flag
    _miscList.clear();
    _globalMisc = 0; // Initial misc value = 0;
    // file type
    _fileType = 0;
}

// ----------------------------------------------------------------------
//  print GV network
// ----------------------------------------------------------------------
// recursively print the gia network
void
GVNtkMgr::print_rec(Gia_Man_t* pGia, Gia_Obj_t* pObj) {
    // if the TravId of the node is equal to the global TravId, return
    if (Gia_ObjIsTravIdCurrent(pGia, pObj)) {
        return;
    }

    // set the TravId of the node to be the same as the global TravId, that is
    // mark it as traversed
    Gia_ObjSetTravIdCurrent(pGia, pObj);

    // If we reach the combinational input(PI + RO (register output, or pseudo
    // PI)), return
    if (Gia_ObjIsCi(pObj)) {
        return;
    }

    /* AIG node: #fanin = 2 */
    if (Gia_ObjFaninNum(pGia, pObj) > 1) {
        // create a new GVNetId corresponding to abc's id
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_AIG);
        // map
        _id2Type[id.id] = id.type;
        // fanin phase
        id.fanin0Cp = Gia_ObjFaninC0(pObj);
        id.fanin1Cp = Gia_ObjFaninC1(pObj);
        createNet(id, GV_NTK_OBJ_AIG);

        /* if fanin id is RO, replace it with PPI */
        // fanin 0
        if (_fileType == GV_NTK_TYPE_V) {
            if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))) ==
                GV_NTK_OBJ_RO) {
                _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                    getPpiIdFromRoId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))));
                GVFanout fanout;
                fanout.id    = Gia_ObjId(pGia, pObj);
                fanout.fanin = 0;
                _id2Fanout[getPpiIdFromRoId(Gia_ObjId(pGia,
                Gia_ObjFanin0(pObj)))]
                    .push_back(fanout);
            } else {
                _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                    Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
                GVFanout fanout;
                fanout.id    = Gia_ObjId(pGia, pObj);
                fanout.fanin = 0;
                _id2Fanout[Gia_ObjId(pGia, Gia_ObjFanin0(pObj))].push_back(fanout);
            }
        }
        if (_fileType == GV_NTK_TYPE_AIG) {
            _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
            Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            GVFanout fanout;
            fanout.id    = Gia_ObjId(pGia, pObj);
            fanout.fanin = 0;
            _id2Fanout[Gia_ObjId(pGia, Gia_ObjFanin0(pObj))].push_back(fanout);
        }
        // recursive traverse its left child
        print_rec(pGia, Gia_ObjFanin0(pObj));

        // fanin 1
        if (_fileType == GV_NTK_TYPE_V) {
            if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin1(pObj))) ==
                GV_NTK_OBJ_RO) {
                _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                    getPpiIdFromRoId(Gia_ObjId(pGia, Gia_ObjFanin1(pObj))));
                GVFanout fanout;
                fanout.id    = Gia_ObjId(pGia, pObj);
                fanout.fanin = 1;
                _id2Fanout[getPpiIdFromRoId(Gia_ObjId(pGia,
                Gia_ObjFanin1(pObj)))]
                    .push_back(fanout);
            } else {
            _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin1(pObj)));
            GVFanout fanout;
            fanout.id    = Gia_ObjId(pGia, pObj);
            fanout.fanin = 1;
            _id2Fanout[Gia_ObjId(pGia, Gia_ObjFanin1(pObj))].push_back(fanout);
            }
        }
        if (_fileType == GV_NTK_TYPE_AIG) {
            _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
            Gia_ObjId(pGia, Gia_ObjFanin1(pObj)));
        }
        print_rec(pGia, Gia_ObjFanin1(pObj));
    }
    /* PO and RI: #fanin = 1 */
    else if (Gia_ObjFaninNum(pGia, pObj) == 1) {
        //  fanin 0
        if (_fileType == GV_NTK_TYPE_V) {
            if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))) ==
                GV_NTK_OBJ_RO) {
                _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                    getPpiIdFromRoId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))));
                GVFanout fanout;
                fanout.id    = Gia_ObjId(pGia, pObj);
                fanout.fanin = 0;
                _id2Fanout[getPpiIdFromRoId(Gia_ObjId(pGia,
                Gia_ObjFanin0(pObj)))]
                    .push_back(fanout);

            } else {
            _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            GVFanout fanout;
            fanout.id    = Gia_ObjId(pGia, pObj);
            fanout.fanin = 0;
            _id2Fanout[Gia_ObjId(pGia, Gia_ObjFanin0(pObj))].push_back(fanout);
            }
        }
        if (_fileType == GV_NTK_TYPE_AIG) {
            _id2FaninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            GVFanout fanout;
            fanout.id    = Gia_ObjId(pGia, pObj);
            fanout.fanin = 0;
            _id2Fanout[Gia_ObjId(pGia, Gia_ObjFanin0(pObj))].push_back(fanout);
        }
        // fanin phase
        _id2GVNetId[Gia_ObjId(pGia, pObj)].fanin0Cp = Gia_ObjFaninC0(pObj);

        // recursive traverse its left child
        print_rec(pGia, Gia_ObjFanin0(pObj));
    }
}

// ----------------------------------------------------------------------
//  construct GV network
// ----------------------------------------------------------------------
void
GVNtkMgr::createNet(const GVNetId& id, const int net_type) {
    // assert(!isGVNetInverted(id));
    if (net_type == GV_NTK_OBJ_PI) {
        _InputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_PO) {
        _OutputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_FF_CS) {
        _FFList.push_back(id);
    } else { // AIG node
        _id2GVNetId[id.id] = id;
    }
    return;
}

void
GVNtkMgr::createNetFromAbc(char* pFileName) {
    Gia_Man_t* pGia = NULL;            // the gia pointer of abc
    Gia_Obj_t *pObj, *pObjRi, *pObjRo; // the obj element of gia

    // abc function parameters
    char* pTopModule =
        NULL; // the top module can be auto detected by yosys, no need to set
    char* pDefines    = NULL;
    int   fBlast      = 1; // blast the ntk to gia (abc's aig data structure)
    int   fInvert     = 0;
    int   fTechMap    = 1;
    int   fSkipStrash = 0;
    int   fCollapse   = 0;
    int c, fVerbose = 1; // set verbose to 1 to see which yosys command is used
    int i, *pWire;

    // read and blast the RTL verilog file into gia
    if (_fileType == GV_NTK_TYPE_V) {
        pGia = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines,
                                      fSkipStrash, fInvert, fTechMap, fVerbose);
    } else if (_fileType == GV_NTK_TYPE_AIG) {
        cout << "reading file" << endl;
        pGia = Gia_AigerRead(pFileName, 0, fSkipStrash, 0);
        cout << "reading file end" << endl;
    } else {
        cout << "Error type!!!!!!" << endl;
        assert(false);
    }

    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);
    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));
    // create the PI and PPI
    if (_fileType == GV_NTK_TYPE_V) {
        Gia_ManForEachPi(pGia, pObj, i) {
            // cout << "fff " << i << endl;
            // PI
            if (i <= (Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia))) {
                // create a new GVNetId corresponding to abc's id
                GVNetId id =
                    GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PI);
                createNet(id, GV_NTK_OBJ_PI);
                // map
                _id2GVNetId[id.id] = id;
                _id2Type[id.id]    = id.type;
            }
            // PPI
            else {
                // create a new GVNetId corresponding to abc's id
                GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0,
                                                GV_NTK_OBJ_FF_CS);
                createNet(id, GV_NTK_OBJ_FF_CS);
                // map
                _id2GVNetId[id.id] = id;
                _id2Type[id.id]    = id.type;
            }
        }
    } else if (_fileType == GV_NTK_TYPE_AIG) {
        Gia_ManForEachPi(pGia, pObj, i) {
            // cout << "fff " << i << endl;
            // PI
            // if (i <= (Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia))) {
            // create a new GVNetId corresponding to abc's id
            GVNetId id =
                GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PI);
            createNet(id, GV_NTK_OBJ_PI);
            // map
            _id2GVNetId[id.id] = id;
            _id2Type[id.id]    = id.type;
            // }
            // // PPI
            // else {
            //     // create a new GVNetId corresponding to abc's id
            //     GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0,
            //                                     GV_NTK_OBJ_FF_CS);
            //     createNet(id, GV_NTK_OBJ_FF_CS);
            //     // map
            //     _id2GVNetId[id.id] = id;
            //     _id2Type[id.id]    = id.type;
            // }
        }
    }

    // create the PO
    Gia_ManForEachPo(pGia, pObj, i) {
        // create a new GVNetId corresponding to abc's id
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PO);
        createNet(id, GV_NTK_OBJ_PO);
        // map
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
    }

    // create the RI (register input, D in FF)
    bool hasConst = false;
    Gia_ManForEachRi(pGia, pObj, i) {
        // create a new GVNetId corresponding to abc's id
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_FF_NS);
        createNet(id, GV_NTK_OBJ_FF_NS);
        // map
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
        if (_fileType == GV_NTK_TYPE_V) {
            // the last FF is used to connect const0
            if (i == Gia_ManRegNum(pGia) - 1) {
                // FF
                _FFConst0List.push_back(id);
                // create a new GVNetId for const 0
                GVNetId id_const0 = GVNetId::makeNetId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj)), 0, GV_NTK_OBJ_CONST0);
                createNet(id_const0, GV_NTK_OBJ_CONST0);
                _ConstList.push_back(id_const0);
                // map
                _id2GVNetId[id_const0.id] = id_const0;
                _id2Type[id_const0.id]    = id_const0.type;
            }
        }
        if (_fileType == GV_NTK_TYPE_AIG) {
        // the last FF is used to connect const0
        // if (Gia_ObjId(pGia, Gia_ObjFanin0(pObj)) == 0) {
        //     hasConst = true;
        //     // FF
        //     _FFConst0List.push_back(id);
        //     // create a new GVNetId for const 0
        //     GVNetId id_const0 = GVNetId::makeNetId(
        //         Gia_ObjId(pGia, Gia_ObjFanin0(pObj)), 0, GV_NTK_OBJ_CONST0);
        //     createNet(id_const0, GV_NTK_OBJ_CONST0);
        //     _ConstList.push_back(id_const0);
        //     // map
        //     _id2GVNetId[id_const0.id] = id_const0;
        //     _id2Type[id_const0.id]    = id_const0.type;
        // }
    }
    // if(!hasConst) {
        // create a new GVNetId for const 0
        GVNetId id_const0 = GVNetId::makeNetId(0, 0, GV_NTK_OBJ_CONST0);
        createNet(id_const0, GV_NTK_OBJ_CONST0);
        _ConstList.push_back(id_const0);
        // map
        _id2GVNetId[id_const0.id] = id_const0;
        _id2Type[id_const0.id]    = id_const0.type;
    // }
    }

    // create the RO (register output, Q in FF)
    GVNetId id;
    Gia_ManForEachRo(pGia, pObj, i) {
        // create a new GVNetId corresponding to abc's id
        if (_fileType == GV_NTK_TYPE_V) {
            id =
                GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_RO);
        }
        if (_fileType == GV_NTK_TYPE_AIG) {
            id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_FF_CS);
        }
        // map
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
        // debug
        if (_fileType == GV_NTK_TYPE_AIG) {
            createNet(id, GV_NTK_OBJ_FF_CS);
        }
        // the last FF is used to connect const0
        if (_fileType == GV_NTK_TYPE_V) {
            if (i < Gia_ManRegNum(pGia) - 1) {
                _idRo2Ppi[id.id] = getFF(i).id;
            }
        }
    }

    // map RI and RO
    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        // map
        _idRo2Ri[Gia_ObjId(pGia, pObjRo)] = Gia_ObjId(pGia, pObjRi);
        _idRi2Ro[Gia_ObjId(pGia, pObjRi)] = Gia_ObjId(pGia, pObjRo);
        // the last FF is used to connect const0 (skip)
        if (i < Gia_ManRegNum(pGia)) {
            if (_fileType == GV_NTK_TYPE_V) {
                _id2FaninId[getPpiIdFromRoId(Gia_ObjId(pGia, pObjRo))].push_back(
                    Gia_ObjId(pGia, pObjRi));
            }
            else if (_fileType == GV_NTK_TYPE_AIG) {
                _id2FaninId[Gia_ObjId(pGia, pObjRo)].push_back(
                    Gia_ObjId(pGia, pObjRi));
            }
        }
    }

    // dfs traverse from each combinational output PO (primary output) and
    // RI (register input, which can be viewed as pseudo PO)
    Gia_ManForEachCo(pGia, pObj, i) {
        print_rec(pGia, pObj);
    }

    // propagate the bubble of Co's
    Gia_ManForEachCo(pGia, pObj, i) {
        _id2GVNetId[getFaninId(Gia_ObjId(pGia, pObj))[0]].cp ^= _id2GVNetId[Gia_ObjId(pGia, pObj)].fanin0Cp;
    }

    // construct the net id/name mapping
    if (_fileType == GV_NTK_TYPE_V) {
        parseAigMapping(pGia);
    }
}

string
netName(string name, int bit) {
    return name + "[" + to_string(bit) + "]";
}

// ----------------------------------------------------------------------
//  parse the aig mapping from the ".map.txt" file
// ----------------------------------------------------------------------
void
GVNtkMgr::parseAigMapping(Gia_Man_t* pGia) {
    string   buffer;
    ifstream mapFile;
    int      idx, bit;
    string   name;

    mapFile.open(".map.txt");
    assert(mapFile.is_open());
    while (mapFile) {
        if (!(mapFile >> buffer)) break;
        // input
        if (buffer == "input") {
            mapFile >> buffer;
            myStr2Int(buffer, idx);
            mapFile >> buffer;
            myStr2Int(buffer, bit);
            mapFile >> buffer;
            name = buffer;
            _netId2Name[Gia_ObjId(pGia, Gia_ManPi(pGia, idx))] =
                netName(name, bit);
            _netName2Id[netName(name, bit)] =
                Gia_ObjId(pGia, Gia_ManPi(pGia, idx));
        }
        // output
        else if (buffer == "output") {
            mapFile >> buffer;
            myStr2Int(buffer, idx);
            mapFile >> buffer;
            myStr2Int(buffer, bit);
            mapFile >> buffer;
            name = buffer;
            _netId2Name[Gia_ObjId(pGia, Gia_ManPo(pGia, idx))] =
                netName(name, bit);
            _netName2Id[netName(name, bit)] =
                Gia_ObjId(pGia, Gia_ManPo(pGia, idx));
        }
        // FF
        else if (buffer == "latch") {
            mapFile >> buffer;
            myStr2Int(buffer, idx);
            mapFile >> buffer;
            myStr2Int(buffer, bit);
            mapFile >> buffer;
            name = buffer;
            _netId2Name[Gia_ObjId(pGia,
                                  Gia_ObjRiToRo(pGia, Gia_ManRi(pGia, idx)))] =
                netName(name, bit);
            _netName2Id[netName(name, bit)] =
                Gia_ObjId(pGia, Gia_ObjRiToRo(pGia, Gia_ManRi(pGia, idx)));
            _netId2Name[Gia_ObjId(pGia, Gia_ManRi(pGia, idx))] =
                netName(name, bit) + "_ns";
            _netName2Id[netName(name, bit) + "_ns"] =
                Gia_ObjId(pGia, Gia_ManRi(pGia, idx));
        }
    }
}

// ----------------------------------------------------------------------
//  print the information of all PI's
// ----------------------------------------------------------------------
void
GVNtkMgr::printPi() {
    cout << "\nPI :" << endl;
    for (unsigned i = 0; i < getInputSize(); i++) {
        // if (getNetNameFromId(getInput(i).id).length() != 0)
        cout << "PI #" << setw(5) << i << " : net name = " << setw(20)
             << getNetNameFromId(getInput(i).id) << " net id = " << setw(10)
             << getInput(i).id << endl;
    }
}

// ----------------------------------------------------------------------
//  print the information of all PO's
// ----------------------------------------------------------------------
void
GVNtkMgr::printPo() {
    cout << "\nPO :" << endl;
    for (unsigned i = 0; i < getOutputSize(); i++) {
        cout << "PO #" << setw(5) << i << " : net name = " << setw(20)
             << getNetNameFromId(getOutput(i).id) << " net id = " << setw(10)
             << getOutput(i).id << endl;
    }
}

// ----------------------------------------------------------------------
//  print the information of all RI's
// ----------------------------------------------------------------------
void
GVNtkMgr::printRi() {
    cout << "\nFF :" << endl;
    for (unsigned i = 0; i < getFFSize(); i++) {
        cout << "FF #" << setw(5) << i << " : net name = " << setw(20)
             << getNetNameFromId(getFF(i).id) << " net id = " << setw(10)
             << getFF(i).id << endl;
    }
}

// ----------------------------------------------------------------------
//  print the information of all Obj in the aig ntk
// ----------------------------------------------------------------------
void
GVNtkMgr::printSummary() {
    // iterate through the net ids
    for (auto obj : _id2GVNetId) {
        cout << "net " << setw(7) << obj.first;
        // if it has fanin
        if (_id2FaninId.find(obj.first) != _id2FaninId.end()) {
            cout << " , fanin0 = " << setw(7) << _id2FaninId[obj.first][0];
            // if it has the second fanin
            if (_id2FaninId[obj.first].size() >= 2)
                cout << setw(7) << " , fanin1 = " << _id2FaninId[obj.first][1];
            // cout << endl;
        } else if (getGateType(getGVNetId(obj.first)) == GV_NTK_OBJ_PI) {
            cout << " , PI, No fanin.";
        } else if (getGateType(getGVNetId(obj.first)) == GV_NTK_OBJ_RO) {
            cout << " , RO, No fanin. (Please also note that RO is overlapped "
                    "with PI, so no BDD "
                    "node is created.)";
        }
        if (_id2Fanout.find(obj.first) != _id2Fanout.end()) {
            for (int i = 0; i < _id2Fanout[obj.first].size(); ++i) {
                cout << setw(7) << " fanout = " << setw(7)
                     << _id2Fanout[obj.first][i].id << "'s fanin"
                     << _id2Fanout[obj.first][i].fanin;
            }
            cout << endl;
        } else {
            cout << endl;
        }
    }
}
// ----------------------------------------------------------------------
//  create the new net
// ----------------------------------------------------------------------
GVNetId
GVNtkMgr::createNet() {
    GVNetId id = GVNetId::makeNetId(getNetSize());
    createNet(id, GV_NTK_OBJ_AIG);
    return id;
}

bool
GVNtkMgr::createGVAndGate(GVNetId& id, GVNetId id1, GVNetId id2) {
    id.type = GV_NTK_OBJ_AIG;
    // Modification for complement
    id.fanin0Cp     = id1.cp;
    id.fanin1Cp     = id2.cp;
    _id2Type[id.id] = id.type;
    vector<unsigned> faninIdList;
    faninIdList.push_back(id1.id);
    faninIdList.push_back(id2.id);
    _id2FaninId[id.id] = faninIdList;
    // Update the complement info
    createNet(id, GV_NTK_OBJ_AIG);
    return true;
}
