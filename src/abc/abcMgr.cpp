#include "abcMgr.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "base/abc/abc.h"
#include "bdd/cudd/cudd.h"
#include "cirGate.h"

// #include "cirMgr.h"
#include "cirMgr.h"
#include "sat/cnf/cnf.h"
#include "util.h"

AbcMgr* abcMgr;

AbcMgr::AbcMgr() : pGia(NULL) {
    init();
}

AbcMgr::~AbcMgr() {
    reset();
}

void AbcMgr::init() {
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
}

void AbcMgr::reset() {
    delete pAbc;
}

static string gateName(const string& name, const int& bit) {
    return name + "[" + to_string(bit) + "]";
}

void AbcMgr::readVerilog(const ABCParam& opt) {
    char* pFileName  = opt.pFileName;
    char* pTopModule = opt.pTopModule;
    char* pDefines   = opt.pDefines;
    int fInvert      = opt.fInvert;
    int fTechMap     = opt.fTechMap;
    int fSkipStrash  = opt.fSkipStrash;
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

void AbcMgr::buildAigName(map<unsigned, string>& id2Name) {
    ifstream mapFile;
    string buffer;
    string type;
    string name;
    int idx = 0, bit = 0;

    mapFile.open(".map.txt");
    assert(mapFile.is_open());

    while (mapFile >> type) {
        mapFile >> buffer;
        myStr2Int(buffer, idx);
        mapFile >> buffer;
        myStr2Int(buffer, bit);
        mapFile >> buffer;
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

void AbcMgr::travAllObj(CirMgr* _cirMgr, const CirFileType& fileType, map<unsigned, string> id2Name) {
    Gia_Obj_t *pObj, *pObjRi, *pObjRo;  // the obj element of gia
    size_t i, iPi = 0, iPpi = 0, iPo = 0, iRi = 0, iRo = 0;
    map<int, int> PPI2RO;

    Gia_ManForEachObj(pGia, pObj, i) {
        if (Gia_ObjIsPi(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if (gateId <= _cirMgr->_piList.size()) {
                CirPiGate* gate               = new CirPiGate(Gia_ObjId(pGia, pObj), 0);
                _cirMgr->_piList[iPi++]       = gate;
                _cirMgr->_totGateList[gateId] = gate;
            } else {
                CirRoGate* gate               = new CirRoGate(Gia_ObjId(pGia, pObj), 0);
                _cirMgr->_roList[iRo++]       = gate;
                _cirMgr->_totGateList[gateId] = gate;
            }
        } else if (Gia_ObjIsPo(pGia, pObj)) {
            string poName   = "";
            CirPoGate* gate = new CirPoGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            poName          = (id2Name.count(gate->getGid())) ? id2Name[gate->getGid()] : to_string(Gia_ObjId(pGia, pObj));
            char* n         = new char[poName.size() + 1];
            strcpy(n, poName.c_str());
            gate->setName(n);
            gate->setIn0(_cirMgr->getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            _cirMgr->_poList[iPo++]                      = gate;
            _cirMgr->_totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsAnd(pObj)) {
            CirAigGate* gate = new CirAigGate(Gia_ObjId(pGia, pObj), 0);
            int fanin0       = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int fanin1       = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
            if (PPI2RO.count(fanin0)) fanin0 = PPI2RO[fanin0];
            if (PPI2RO.count(fanin1)) fanin1 = PPI2RO[fanin1];
            gate->setIn0(_cirMgr->getGate(fanin0), Gia_ObjFaninC0(pObj));
            gate->setIn1(_cirMgr->getGate(fanin1), Gia_ObjFaninC1(pObj));
            _cirMgr->_totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsRo(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if (fileType == VERILOG) {
                PPI2RO[gateId] = 0;
                if (iPpi < iRo) PPI2RO[gateId] = _cirMgr->_roList[iPpi]->getGid();
                iPpi++;
            } else if (fileType == AIGER) {
                CirRoGate* gate = new CirRoGate(Gia_ObjId(pGia, pObj), 0);

                _cirMgr->_roList[iRo++]       = gate;
                _cirMgr->_totGateList[gateId] = gate;
            }
        } else if (Gia_ObjIsRi(pGia, pObj)) {
            CirRiGate* gate = new CirRiGate(Gia_ObjId(pGia, pObj), 0, Gia_ObjId(pGia, Gia_ObjFanin0(pObj)));
            string str      = to_string(Gia_ObjId(pGia, pObj));
            str             = str + "_ns";
            char* n         = new char[str.size() + 1];
            strcpy(n, str.c_str());
            gate->setName(n);
            gate->setIn0(_cirMgr->getGate(Gia_ObjId(pGia, Gia_ObjFanin0(pObj))), Gia_ObjFaninC0(pObj));
            _cirMgr->_riList[iRi++]                      = gate;
            _cirMgr->_totGateList[Gia_ObjId(pGia, pObj)] = gate;
        } else if (Gia_ObjIsConst0(pObj)) {
            _cirMgr->_totGateList[0] = CirMgr::_const0;
        } else {
            cout << "not defined gate type" << endl;
            assert(true);
        }
    }

    Gia_ManForEachRiRo(pGia, pObjRi, pObjRo, i) {
        if (i == _cirMgr->getNumLATCHs()) break;
        int riGid = Gia_ObjId(pGia, pObjRi), roGid = 0;
        if (fileType == VERILOG) roGid = PPI2RO[Gia_ObjId(pGia, pObjRo)];
        else if (fileType == AIGER) roGid = Gia_ObjId(pGia, pObjRo);
        CirGate* riGate   = _cirMgr->getGate(riGid);
        CirRoGate* roGate = static_cast<CirRoGate*>(_cirMgr->getGate(roGid));
        roGate->setIn0(riGate, false);
    }

    // CONST1 Gate
    _cirMgr->_const1 = new CirAigGate(_cirMgr->getNumTots(), 0);
    _cirMgr->addTotGate(_cirMgr->_const1);
    _cirMgr->_const1->setIn0(_cirMgr->_const0, true);
    _cirMgr->_const1->setIn1(_cirMgr->_const0, true);
}

void AbcMgr::initCir(CirMgr* _cirMgr, const CirFileType& fileType) {
    // Create lists
    int piNum = 0, regNum = 0, poNum = 0, totNum = 0;
    if (fileType == VERILOG) {
        piNum  = Gia_ManPiNum(pGia) - Gia_ManRegNum(pGia) + 1;
        regNum = Gia_ManRegNum(pGia) - 1;
    } else if (fileType == AIGER) {
        piNum  = Gia_ManPiNum(pGia);
        regNum = Gia_ManRegNum(pGia);
    }
    poNum  = Gia_ManPoNum(pGia);
    totNum = Gia_ManObjNum(pGia);

    _cirMgr->_piList.resize(piNum);
    _cirMgr->_riList.resize(regNum);
    _cirMgr->_roList.resize(regNum);
    _cirMgr->_poList.resize(poNum);
    _cirMgr->_totGateList.resize(totNum);
}

void AbcMgr::execCmd(char* cmd) {
    // calling abc's command
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), cmd);
}
