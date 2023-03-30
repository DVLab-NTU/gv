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
    _FFConst0List.clear();
    // map
    _id2faninId.clear();
    _id2GVNetId.clear();
    _netId2Name.clear();
    _netName2Id.clear();
    _idRO2PPI.clear();
    _idRO2RI.clear();
    _idRI2RO.clear();
    _id2Type.clear();
    // flag
    _miscList.clear();
    _globalMisc = 0; // Initial misc value = 0;
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

    if (Gia_ObjIsCi(pObj)) {
        cout << "================" << endl;
        cout << "is Ci" << endl;
        cout << "node id : " << _id2GVNetId[Gia_ObjId(pGia, pObj)].id << endl;
        cout << "node type : " << _id2GVNetId[Gia_ObjId(pGia, pObj)].type
             << endl;
        cout << "================" << endl;
        return; // If we reach the combinational input(PI + Ro (register output,
                // or pseudo PI)), return.
    }
    cout << "node id / name  = " << Gia_ObjId(pGia, pObj) << " / "
         << " num fanin = " << Gia_ObjFaninNum(pGia, pObj)
         << endl; // print the gia Id of the current node
    cout << "\tfanin-0 id = " << Gia_ObjId(pGia, Gia_ObjFanin0(pObj))
         << "  /  phase = " << pObj->fCompl0
         << endl; // print the id of its left child
    cout << "\tfanin-1 id = " << Gia_ObjId(pGia, Gia_ObjFanin1(pObj))
         << "  /  phase = " << pObj->fCompl1
         << endl; // print the id of its right child
    // create the new GV net id
    if (Gia_ObjFaninNum(pGia, pObj) >
        1) { // PO: #fanin=1 ; AIG: #fanout=2 (Hugo --> bug)
        GVNetId id = GVNetId::makeNetId(Gia_ObjId(pGia, pObj), GV_NTK_OBJ_AIG);
        id.type    = GV_NTK_OBJ_AIG;
        _id2Type[id.id] = id.type;

        // --- for cp bug
        id.fanin0Cp = Gia_ObjFaninC0(pObj);
        id.fanin1Cp = Gia_ObjFaninC1(pObj);
        // --- end

        // id.cp      = Gia_ObjPhaseReal(pObj);
        cout << "================" << endl;
        cout << "aloha~~~~~~~~" << endl;
        cout << "fanin num : " << Gia_ObjFaninNum(pGia, pObj) << endl;
        cout << "node id : " << id.id << endl;
        cout << "node type : " << id.type << endl;
        cout << "node cp : " << id.cp << endl;
        cout << "================" << endl;
        createNet(id, GV_NTK_OBJ_AIG);
        // fanin id if is RO (119~204), replace it with PPI (33~118)
        // fanin 0
        if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))) ==
            GV_NTK_OBJ_RO) {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                getPPIidFromROid(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))));
        } else {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
        }

        // recursive traverse the left child
        // Gia_ObjFaninC0(pObj) = pObj->fCompl0 = fanin0 complement ?
        print_rec(pGia, Gia_ObjFanin0(pObj));
        // fanin 1
        if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin1(pObj))) ==
            GV_NTK_OBJ_RO) {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                getPPIidFromROid(Gia_ObjId(pGia, Gia_ObjFanin1(pObj))));
        } else {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin1(pObj)));
        }
        // add fanin
        // Gia_ObjFaninC1(pObj) = pObj->fCompl1 = fanin1 complement ?
        print_rec(pGia, Gia_ObjFanin1(pObj));
    } else if (Gia_ObjFaninNum(pGia, pObj) ==
               1) { // PO: #fanin=1 ; AIG: #fanout=2 (Hugo --> bug)
        // cout << "hugo~  :  " << phase << endl;
        //_id2GVNetId[Gia_ObjId(pGia, pObj)].cp = phase;
        //  id.cp   = Gia_ObjPhaseReal(pObj);
        // cout << "================" << endl;
        // cout << "fanin num : " << Gia_ObjFaninNum(pGia, pObj) << endl;
        // cout << "node id : " << _id2GVNetId[Gia_ObjId(pGia, pObj)].id <<
        // endl; cout << "node type : " << _id2GVNetId[Gia_ObjId(pGia,
        // pObj)].type
        //      << endl;
        // cout << "node cp : " << phase << endl;
        //  fanin 0
        if (getTypeFromId(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))) ==
            GV_NTK_OBJ_RO) {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                getPPIidFromROid(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))));
        } else {
            _id2faninId[Gia_ObjId(pGia, pObj)].push_back(
                Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
        }

        // --- for cp bug
        _id2GVNetId[Gia_ObjId(pGia, pObj)].fanin0Cp = Gia_ObjFaninC0(pObj);
        // --- end

        // recursive traverse the left child
        // Gia_ObjFaninC0(pObj) = pObj->fCompl0 = fanin0 complement ?
        print_rec(pGia, Gia_ObjFanin0(pObj));
    }
}

//----------------------------------------------------------------------
// construct GV network
//----------------------------------------------------------------------
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
        cout << "!!!!!!!!!!!!!!!!!!!!!!!" << endl;
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
    pGia = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines, fSkipStrash,
                                  fInvert, fTechMap, fVerbose);

    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);
    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));

    // // dfs traverse from each combinational output Po (primary output) and
    // // Ri (register input, which can be understand as pseudo Po)
    // Gia_ManForEachCo(pGia, pObj, i) {
    //     print_rec(pGia, pObj);
    //     // we get the fanin of Co. you can imagine that the po net is simply
    //     an
    //     // one bit buf
    // }
    // create the PI's (including Ro and PI here, although
    // it is named PI = =)
    cout << "fuck u   -->   " << Gia_ManCiNum(pGia) << endl;
    Gia_ManForEachPi(pGia, pObj, i) { // id: 1 ~ 118 #fanin=0
        // pure PI (1 ~ 32 among 1 ~ 118)
        if (i <= (Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia))) {
            GVNetId id =
                GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PI);
            // id.cp      = Gia_ObjPhaseReal(pObj);
            // id.type = GV_NTK_OBJ_PI;
            cout << "================" << endl;
            cout << "node id : " << id.id << endl;
            cout << "node type : " << id.type << endl;
            cout << "node cp : " << id.cp << endl;
            cout << " num fanin (PI: 1~32) = " << Gia_ObjFaninNum(pGia, pObj)
                 << endl; // print the gia Id of the current node
            cout << "================" << endl;
            // create the input for GVNtk
            createNet(id, GV_NTK_OBJ_PI);
            // cout << "PI id " << Gia_ObjId(pGia, pObj) << endl;
            _id2GVNetId[id.id] = id;
            _id2Type[id.id]    = id.type;
        } else { // PPI
            // Ask : this may be the issue !?
            GVNetId id =
                GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_FF_CS);
            // id.cp      = Gia_ObjPhaseReal(pObj);
            // id.type = GV_NTK_OBJ_PI;
            cout << "================" << endl;
            cout << "node id : " << id.id << endl;
            cout << "node type : " << id.type << endl;
            cout << "node cp : " << id.cp << endl;
            cout << " num fanin (RO: 33~118) = " << Gia_ObjFaninNum(pGia, pObj)
                 << endl; // print the gia Id of the current node
            cout << "================" << endl;
            // create the input for GVNtk
            // Ask : this may be the issue !?
            createNet(id, GV_NTK_OBJ_FF_CS);
            // cout << "PI id " << Gia_ObjId(pGia, pObj) << endl;
            _id2GVNetId[id.id] = id;
            _id2Type[id.id]    = id.type;
        }
    }

    // create the PO's
    Gia_ManForEachPo(pGia, pObj, i) { // id: 5725 ~ 5755
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_PO);
        // id.cp      = Gia_ObjPhaseReal(pObj);
        id.type = GV_NTK_OBJ_PO;
        cout << "================" << endl;
        cout << "node id : " << id.id << endl;
        cout << "node type : " << id.type << endl;
        cout << "node cp : " << id.cp << endl;
        cout << "================" << endl;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_PO);
        // cout << id.id << endl;
        // cout << "PO id " << Gia_ObjId(pGia, pObj) << endl;
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
    }

    // create the registers (PPO)
    Gia_ManForEachRi(pGia, pObj, i) { // id: 5756 ~ 5842
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_FF_NS);
        // id.cp      = Gia_ObjPhaseReal(pObj);
        // id.type = GV_NTK_OBJ_FF_NS;
        cout << "================" << endl;
        cout << "node id : " << id.id << endl;
        cout << "node type : " << id.type << endl;
        cout << "node cp : " << id.cp << endl;
        cout << " num fanin (Ri: 5756~5842) = " << Gia_ObjFaninNum(pGia, pObj)
             << endl; // print the gia Id of the current node
        cout << "================" << endl;
        // create the input for GVNtk
        createNet(id, GV_NTK_OBJ_FF_NS);
        // cout << "Ro id " << Gia_ObjId(pGia, pRo) << " Ri " << Gia_ObjId(pGia,
        // pRi) << endl;
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
        // RI for constant 0 (5842)
        // Ask : this may be the issue !?
        if (i == Gia_ManRegNum(pGia) - 1) {
            _FFConst0List.push_back(id);
            // const 0
            GVNetId id_const0 = GVNetId::makeNetId(
                Gia_ObjId(pGia, Gia_ObjFanin0(pObj)), 0, GV_NTK_OBJ_CONST0);
            cout << "================" << endl;
            cout << "node id : " << id_const0.id << endl;
            cout << "node type : " << id_const0.type << endl;
            cout << "node cp : " << id_const0.cp << endl;
            cout << "================" << endl;
            // create the input for GVNtk
            createNet(id_const0, GV_NTK_OBJ_CONST0);

            _ConstList.push_back(id_const0);
            // cout << id.id << endl;
            // cout << "PO id " << Gia_ObjId(pGia, pObj) << endl;
            _id2GVNetId[id_const0.id] = id_const0;
            _id2Type[id_const0.id]    = id_const0.type;
        }
    }

    // create the registers output Q (RO)
    Gia_ManForEachRo(pGia, pObj, i) { // id: 119 ~ 205 #fanin=0
        GVNetId id =
            GVNetId::makeNetId(Gia_ObjId(pGia, pObj), 0, GV_NTK_OBJ_RO);
        // id.cp      = Gia_ObjPhaseReal(pObj);
        // id.type = GV_NTK_OBJ_PI;
        cout << "================" << endl;
        cout << "node id : " << id.id << endl;
        cout << "node type : " << id.type << endl;
        cout << "node cp : " << id.cp << endl;
        cout << " num fanin (Ro: 119~205) = " << Gia_ObjFaninNum(pGia, pObj)
             << endl; // print the gia Id of the current node
        cout << "================" << endl;
        // create the input for GVNtk
        // Ask : this may be the issue !?
        // createNet(id, GV_NTK_OBJ_FF_CS);
        // cout << "Ro id " << Gia_ObjId(pGia, pRo) << " Ri " << Gia_ObjId(pGia,
        // pRi) << endl;
        _id2GVNetId[id.id] = id;
        _id2Type[id.id]    = id.type;
        // Ask : this may be the issue !?
        if (i < Gia_ManRegNum(pGia) - 1) {
            // map 119 ~ 204 to 33 ~ 118
            // Ask : this may be the issue !?
            _idRO2PPI[id.id] = getLatch(i).id;
        }
    }

    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        cout << "lol  -->  " << Gia_ObjId(pGia, pObjRi) << endl;
        cout << "lol1  -->  " << Gia_ObjId(pGia, pObjRo) << endl;
        // map 119 ~ 205 to 5756 ~ 5842
        _idRO2RI[Gia_ObjId(pGia, pObjRo)] = Gia_ObjId(pGia, pObjRi);
        // map 5756 ~ 5842 to 119 ~ 205
        _idRI2RO[Gia_ObjId(pGia, pObjRi)] = Gia_ObjId(pGia, pObjRo);
        if (i < Gia_ManRegNum(pGia)) {
            // insert 33 ~ 118 fanin0 to 5756 ~ 5841
            _id2faninId[getPPIidFromROid(Gia_ObjId(pGia, pObjRo))].push_back(
                Gia_ObjId(pGia, pObjRi));
        }
    }

    // dfs traverse from each combinational output Po (primary output) and
    // Ri (register input, which can be understand as pseudo Po)
    Gia_ManForEachCo(pGia, pObj, i) {
        cout << "~~~~~~~~~~~~~~~~" << endl;
        cout << "fuck ; " << Gia_IsComplement(pObj) << endl;
        cout << "bitch : " << Gia_ObjPhase(pObj) << endl;
        cout << "md : " << Gia_ObjPhaseReal(pObj) << endl;
        cout << "shit : " << Gia_ObjPhaseRealLit(pGia, Gia_Obj2Lit(pGia, pObj))
             << endl;
        // cout << "hi : " << Gia_ObjFanout0(pGia, pObj)->fCompl0 << endl;
        cout << "~~~~~~~~~~~~~~~~" << endl;
        print_rec(pGia, pObj);
        // we get the fanin of Co. you can imagine that the po net is simply an
        // one bit buf
    }

    // Constant value
    // GVNetId id     = GVNetId::makeNetId(0, 0, GV_NTK_OBJ_NONE);
    // _id2GVNetId[0] = id;

    // construct the net id/name mapping
    parseAigMapping(pGia);
    cout << "PPPPPPPP   -->  " << _id2GVNetId.size() << endl;
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

//----------------------------------------------------------------------
// print the information of all PI's
//----------------------------------------------------------------------
void
GVNtkMgr::printPi() {
    cout << "\nPI :" << endl;
    for (unsigned i = 0; i < getInputSize(); i++) {
        if (getNetNameFromId(getInput(i).id).length() != 0)
            cout << "PI #" << setw(5) << i << " : net name = " << setw(20)
                 << getNetNameFromId(getInput(i).id) << " net id = " << setw(10)
                 << getInput(i).id << endl;
    }
}

//----------------------------------------------------------------------
// print the information of all PO's
//----------------------------------------------------------------------
void
GVNtkMgr::printPo() {
    cout << "\nPO :" << endl;
    for (unsigned i = 0; i < getOutputSize(); i++) {
        cout << "PO #" << setw(5) << i << " : net name = " << setw(20)
             << getNetNameFromId(getOutput(i).id) << " net id = " << setw(10)
             << getOutput(i).id << endl;
    }
}

//----------------------------------------------------------------------
// print the information of all RI's
//----------------------------------------------------------------------
void
GVNtkMgr::printRi() {
    cout << "\nFF :" << endl;
    for (unsigned i = 0; i < getFFSize(); i++) {
        cout << "FF #" << setw(5) << i << " : net name = " << setw(20)
             << getNetNameFromId(getLatch(i).id) << " net id = " << setw(10)
             << getLatch(i).id << endl;
    }
}

//----------------------------------------------------------------------
// print the information of all Obj in the aig ntk
//----------------------------------------------------------------------
void
GVNtkMgr::printSummary() {
    // ietrate through the net ids
    for (auto obj : _id2GVNetId) {
        cout << "net " << setw(7) << obj.first;
        // if it has fanin
        if (_id2faninId.find(obj.first) != _id2faninId.end()) {
            cout << " , fanin0 = " << setw(7) << _id2faninId[obj.first][0];
            // if it has the second fanin
            if (_id2faninId[obj.first].size() >= 2)
                cout << setw(7) << " , fanin1 = " << _id2faninId[obj.first][1];
            cout << endl;
        }
    }
}