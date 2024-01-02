#include "abcMgr.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "abcExt.h"
#include "base/abc/abc.h"
#include "bdd/cudd/cudd.h"
#include "cirGate.h"
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

void AbcMgr::execCmd(char* cmd) {
    // calling abc's command
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), cmd);
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
            if (inputIsPi(gateId)) parseInput(iPi++, gateId);
            else parseRo(iRo++, gateId);

        } else if (Gia_ObjIsPo(pGia, pObj)) {
            int gateId    = Gia_ObjId(pGia, pObj);
            int in0Id     = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int inv       = Gia_ObjFaninC0(pObj);
            string poName = (id2Name.count(gateId)) ? id2Name[gateId] : to_string(gateId);
            parseOutput(iPo++, gateId, in0Id, inv, poName);

        } else if (Gia_ObjIsAnd(pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            int in0Id  = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int in1Id  = Gia_ObjId(pGia, Gia_ObjFanin1(pObj));
            int in0Inv = Gia_ObjFaninC0(pObj);
            int in1Inv = Gia_ObjFaninC1(pObj);
            if (PPI2RO.count(in0Id)) in0Id = PPI2RO[in0Id];
            if (PPI2RO.count(in1Id)) in1Id = PPI2RO[in1Id];
            parseAig(gateId, in0Id, in0Inv, in1Id, in1Inv);

        } else if (Gia_ObjIsRo(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            if (fileType == VERILOG) {
                PPI2RO[gateId] = 0;
                if (iPpi < iRo) PPI2RO[gateId] = _cirMgr->_roList[iPpi]->getGid();
                iPpi++;
            } else if (fileType == AIGER) {
                parseRo(iRo++, gateId);
            }
        } else if (Gia_ObjIsRi(pGia, pObj)) {
            int gateId = Gia_ObjId(pGia, pObj);
            int in0Id  = Gia_ObjId(pGia, Gia_ObjFanin0(pObj));
            int inv    = Gia_ObjFaninC0(pObj);
            parseRi(iRi++, gateId, in0Id, inv);
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
        parseRiRo(riGid, roGid);
    }
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

    initCirMgr(piNum, poNum, regNum, totNum);
}
