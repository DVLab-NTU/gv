#include "gvAbcMgr.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "base/abc/abc.h"
#include "bdd/cudd/cudd.h"
#include "cirMgr.h"
#include "cirGate.h"
#include "gvAbcNtk.h"
#include "sat/cnf/cnf.h"
#include "util.h"

AbcMgr* abcMgr;

AbcMgr::AbcMgr() : pGia(NULL) {
    init();
}

void AbcMgr::init() {
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
}

void AbcMgr::reset() {
    delete pAbc;
}

void AbcMgr::abcReadDesign(string& fileName) {
    char pFileName[128];
    strcpy(pFileName, fileName.c_str());
    char Command[1000];
    sprintf(Command, "read %s", pFileName);
    Cmd_CommandExecute(pAbc, Command);
    pNtkMgr = new abcNtkMgr(pAbc->pNtkCur);
}

void AbcMgr::readVerilog(const ABCParam& opt) {
    char* pFileName  = opt.pFileName;
    char* pTopModule = opt.pTopModule;
    char* pDefines   = opt.pDefines;
    int fBlast       = opt.fBlast;
    int fInvert      = opt.fInvert;
    int fTechMap     = opt.fTechMap;
    int fSkipStrash  = opt.fSkipStrash;
    int fCollapse    = opt.fCollapse;
    int fVerbose     = opt.fVerbose;
    pGia             = Wln_BlastSystemVerilog(pFileName, pTopModule, pDefines, fSkipStrash, fInvert, fTechMap, fVerbose);
}

void AbcMgr::readAig(const ABCParam& opt) {
    char* pFileName = opt.pFileName;
    int fSkipStrash = opt.fSkipStrash;
    int fGiaSimple  = opt.fGiaSimple;
    int fCheck      = opt.fCheck;
    pGia            = Gia_AigerRead(pFileName, fGiaSimple, fSkipStrash, fCheck);
}

static string gateName(const string& name, const int& bit) {
    return name + "[" + to_string(bit) + "]";
}

void AbcMgr::buildAigName(map<unsigned, string>& id2Name) {
    ifstream mapFile;
    string buffer;
    string type;
    string name;
    int idx = 0, bit = 0;

    mapFile.open(".map.txt");
    assert(mapFile.is_open());
    while (mapFile) {
        if (!(mapFile >> type)) break;
        mapFile >> buffer;  // idx
        myStr2Int(buffer, idx);
        mapFile >> buffer;  // bit
        myStr2Int(buffer, bit);
        mapFile >> buffer;  // name
        name = buffer;
        if (type == "input") id2Name[Gia_ObjId(pGia, Gia_ManPi(pGia, idx))] = gateName(name, bit);
        else if (type == "output") id2Name[Gia_ObjId(pGia, Gia_ManPo(pGia, idx))] = gateName(name, bit);
        else if (type == "latch") {
            Gia_Obj_t* ro = Gia_ObjRiToRo(pGia, Gia_ManRi(pGia, idx));
            Gia_Obj_t* ri = Gia_ManRi(pGia, idx);

            id2Name[Gia_ObjId(pGia, ro)] = gateName(name, bit);
            id2Name[Gia_ObjId(pGia, ri)] = gateName(name, bit) + "_ns";
        }
    }
}

void AbcMgr::travPreprocess() {
    // increment the global travel id for circuit traversing usage
    Gia_ManIncrementTravId(pGia);

    // since we don't want to traverse the constant node, set the TravId of the
    // constant node to be as the global one
    Gia_ObjSetTravIdCurrent(pGia, Gia_ManConst0(pGia));
}

void AbcMgr::travAllObj(vector<CirPiGate*>& piList, vector<CirPoGate*>& poList, vector<CirRoGate*>& roList, vector<CirRiGate*>& riList,
                        vector<CirGate*>& totGateList, map<unsigned, string>& id2Name, const CirFileType& fileType) {
    Gia_Obj_t *pObj, *pObjRi, *pObjRo;  // the obj element of gia
    size_t i, iPi = 0, iPpi = 0, iPo = 0, iRi = 0, iRo = 0;
    map<int, int> PPI2RO;

    Gia_ManForEachObj(pGia, pObj, i) {
        if (Gia_ObjIsPi(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if (gateId <= piList.size()) {
                CirPiGate* gate     = new CirPiGate(Gia_ObjId(pGia, pObj), 0);
                piList[iPi++]       = gate;
                totGateList[gateId] = gate;
            } else {
                CirRoGate* gate     = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                roList[iRo++]       = gate;
                totGateList[gateId] = gate;
            }
        } else if (Gia_ObjIsPo(pGia, pObj)) {
            string poName   = "";
            CirPoGate* gate = new CirPoGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            poName          = (id2Name.count(gate->getGid())) ? id2Name[gate->getGid()] : to_string(Gia_ObjId(pGia, pObj));
            char* n         = new char[poName.size() + 1];
            strcpy(n, poName.c_str());
            gate->setName(n);
            gate->setIn0(cirMgr->getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            poList[iPo++]                      = gate;
            totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsAnd(pObj)) {
            CirAigGate* gate = new CirAigGate(Gia_ObjId(pGia, pObj), 0);
            int fanin0       = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int fanin1       = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
            if (PPI2RO.count(fanin0)) fanin0 = PPI2RO[fanin0];
            if (PPI2RO.count(fanin1)) fanin1 = PPI2RO[fanin1];
            gate->setIn0(cirMgr->getGate(fanin0), Gia_ObjFaninC0(pObj));
            gate->setIn1(cirMgr->getGate(fanin1), Gia_ObjFaninC1(pObj));
            totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsRo(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if (fileType == VERILOG) {
                PPI2RO[gateId] = 0;
                if (iPpi < iRo) PPI2RO[gateId] = roList[iPpi]->getGid();
                iPpi++;
            } else if (fileType == AIGER) {
                CirRoGate* gate     = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                roList[iRo++]       = gate;
                totGateList[gateId] = gate;
            }
        } else if (Gia_ObjIsRi(pGia, pObj)) {
            CirRiGate* gate = new CirRiGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            string str      = to_string(Gia_ObjId(pGia, pObj));
            str             = str + "_ns";
            char* n         = new char[str.size() + 1];
            strcpy(n, str.c_str());
            gate->setName(n);
            gate->setIn0(cirMgr->getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            riList[iRi++]                      = gate;
            totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsConst0(pObj)) {
            totGateList[0] = CirMgr::_const0;
        } else {
            cout << "not defined gate type" << endl;
            assert(true);
        }
    }

    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        if (i == cirMgr->getNumLATCHs()) break;

        int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
        // int roGid = PPI2RO[Gia_ObjId(pGia,pObjRo)];
        // int roGid = Gia_ObjId(pGia,pObjRo);
        if (fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia, pObjRo)];
        else if (fileType == AIGER) roGid = Gia_ObjId(pGia, pObjRo);
        CirGate* riGate   = cirMgr->getGate(riGid);
        CirRoGate* roGate = static_cast<CirRoGate*>(cirMgr->getGate(roGid));
        roGate->setIn0(riGate, false);
    }
}