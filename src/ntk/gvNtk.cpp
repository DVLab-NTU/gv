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
    _id2faninId.clear();
    _id2GVNetId.clear();
}

//----------------------------------------------------------------------
// print GV network
//----------------------------------------------------------------------
// recursively print the gia network
void
GVNtkMgr::print_rec(Gia_Man_t* pGia, Gia_Obj_t* pObj) {
    if (Gia_ObjIsTravIdCurrent(pGia, pObj))
        return; // if the TravId of the node is equal to the global TravId,
                // return
    // if (Gia_ObjIsPi(pGia, pObj))
    //     cout << "Pi reached, id = " << Gia_ObjId(pGia, pObj) << " " << endl;
    // if (Gia_ObjIsRo(pGia, pObj))
    //     cout << "Ro reached, id = " << Gia_ObjId(pGia, pObj) << endl;
    Gia_ObjSetTravIdCurrent(
        pGia, pObj); // set the TravId of the node to be the same as the global
                     // TravId, that is mark it as traversed

    if (Gia_ObjIsCi(pObj))
        return; // If we reach the combinational input(PI + Ro (register output,
                // or pseudo PI)), return.
    // cout << "node id = " << Gia_ObjId(pGia, pObj)
    //      << " num fanin = " << Gia_ObjFaninNum(pGia, pObj)
    //      << endl; // print the gia Id of the current node
    // cout << "\tfanin-0 id = " << Gia_ObjId(pGia, Gia_ObjFanin0(pObj))
    //      << endl; // print the id of its left child
    // cout << "\tfanin-1 id = " << Gia_ObjId(pGia, Gia_ObjFanin1(pObj))
    //      << endl; // print the id of its right child
    // create the new GV net id
    GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj));
    id.type = GV_NTK_OBJ_AIG;
    createNet(id,GV_NTK_OBJ_AIG);
    _id2faninId[Gia_ObjId(pGia, pObj)].push_back(Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
    if (Gia_ObjFanin1(pObj))
        _id2faninId[Gia_ObjId(pGia, pObj)].push_back(Gia_ObjId(pGia, Gia_ObjFanin1(pObj)));

    if (Gia_ObjFanin0(pObj)) {
        // add fanin
        
        
        // recursive traverse the left child
        print_rec(pGia, Gia_ObjFanin0(pObj));
    }
    if (Gia_ObjFanin1(pObj)) {
        // add fanin
        // _id2faninId[Gia_ObjId(pGia, pObj)].push_back(Gia_ObjId(pGia, Gia_ObjFanin1(pObj)));
        // create the new GV net id
        // GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
        // id.type = GV_NTK_OBJ_AIG;
        // createNet(id,GV_NTK_OBJ_AIG);
        // recursive traverse the right child
        print_rec(pGia, Gia_ObjFanin1(pObj));
    }
}

//----------------------------------------------------------------------
// construct GV network
//----------------------------------------------------------------------
void
GVNtkMgr::createNet(const GVNetId& id, const int net_type) {
    assert(!isGVNetInverted(id));
    if (net_type == GV_NTK_OBJ_PI) {
        _InputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_PO) {
        _OutputList.push_back(id);
    } else if (net_type == GV_NTK_OBJ_FF) {
        _FFList.push_back(id);
    } else { // AIG node
        _id2GVNetId[id.id] = id;
    }
    return;
}

void
GVNtkMgr::createNetFromAbc(char* pFileName) {
    Gia_Man_t* pGia = NULL; // the gia pointer of abc
    Gia_Obj_t* pObj;        // the obj element of gia

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
    pGia = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines, fSkipStrash,
                                  fInvert, fTechMap, fVerbose);

    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);
    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));

    // dfs traverse from each combinational output Po (primary output) and
    // Ri (register input, which can be understand as pseudo Po)
    Gia_ManForEachCo(pGia, pObj, i) {
        print_rec(pGia, pObj);
        // we get the fanin of Co. you can imagine that the po net is simply an
        // one bit buf
    }

    // create the PI's (including Ro and PI here, although it is named PI = =)
    Gia_ManForEachPi(pGia, pObj, i) {
        GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj));
        id.type    = GV_NTK_OBJ_PI;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_PI);
        // cout << "PI id " << Gia_ObjId(pGia, pObj) << endl;
        _id2GVNetId[id.id] = id;
    }

    // create the PO's
    Gia_ManForEachPo(pGia, pObj, i) {
        GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj));
        id.type    = GV_NTK_OBJ_PO;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_PO);
        // cout << id.id << endl;
        // cout << "PO id " << Gia_ObjId(pGia, pObj) << endl;
        _id2GVNetId[id.id] = id;
    }

    // create the registers (only has to create the Ri because Ro is created by
    // PI)
    Gia_ManForEachRi(pGia, pObj, i) {
        GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj));
        id.type    = GV_NTK_OBJ_FF;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_FF);
        // cout << "Ro id " << Gia_ObjId(pGia, pRo) << " Ri " << Gia_ObjId(pGia,
        // pRi) << endl;
        _id2GVNetId[id.id] = id;
    }

    // create the registers (only has to create the Ri because Ro is created by
    // PI)
    Gia_ManForEachRo(pGia, pObj, i) {
        GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj));
        id.type = GV_NTK_OBJ_PI;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_PI);
        // cout << "Ro id " << Gia_ObjId(pGia, pRo) << " Ri " << Gia_ObjId(pGia,
        // pRi) << endl;
        _id2GVNetId[id.id] = id;
    }


    // construct the net id/name mapping
    parseAigMapping(pGia);
}

string
netName(string name, int bit) {
    return name + "[" + to_string(bit) + "]";
}

//----------------------------------------------------------------------
// parse the aig mapping from the ".map.txt" file
//----------------------------------------------------------------------
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
            // cout << Gia_ObjId(pGia, Gia_ManPi(pGia, idx)) << " " <<
            // netName(name, bit) << endl;
        } else if (buffer == "output") {
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
            // cout << Gia_ObjId(pGia, Gia_ManPo(pGia, idx)) << " " <<
            // netName(name, bit) << endl;
        } else if (buffer == "latch") {
            mapFile >> buffer;
            myStr2Int(buffer, idx);
            mapFile >> buffer;
            myStr2Int(buffer, bit);
            mapFile >> buffer;
            name = buffer;
            _netId2Name[Gia_ObjId(pGia, Gia_ManRi(pGia, idx))] =
                netName(name, bit);
            _netName2Id[netName(name, bit)] =
                Gia_ObjId(pGia, Gia_ManRi(pGia, idx));
            // cout << Gia_ObjId(pGia, Gia_ManRi(pGia, idx)) << " " <<
            // netName(name, bit) << endl;
        }
    }
}
