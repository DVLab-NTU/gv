/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "abcMgr.h"
#include "yosysMgr.h"

// using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.
#include "cirDef.h"
#include "gvType.h"

extern CirMgr* cirMgr;

class AbcMgr;
class CirMgr {
    friend class CirComb;
    friend class CirSeq;

    enum CirMgrFlag { NO_FEC = 0x1 };
    enum ParsePorts { VARS = 0,
                      PI,
                      LATCH,
                      PO,
                      AIG,
                      TOT_PARSE_PORTS };

public:
    CirMgr() : _piList(0), _poList(0), _totGateList(0), _fanoutInfo(0),
               _abcMgr(new AbcMgr()), _ysyMgr(new YosysMgr()), _fileName("") {}

    virtual ~CirMgr() { deleteCircuit(); }

    // Access functions
    CirGate* operator[](unsigned gid) const { return _totGateList[gid]; }
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate* getGate(unsigned gid) const {
        if (gid >= getNumTots()) {
            return 0;
        }
        return _totGateList[gid];
    }

    void setFileName(const string& f) { _fileName = f; }
    void setFileType(const FileType& t) { _fileType = t; }

    unsigned getNumLATCHs() const { return _riList.size(); }
    // virtual unsigned getNumLATCHs() const { return 0; }
    unsigned getNumPIs() const { return _piList.size(); }
    unsigned getNumPOs() const { return _poList.size(); }
    unsigned getNumAIGs() const { return _aigList.size(); }
    unsigned getNumTots() const { return _totGateList.size(); }

    CirRiGate* getRi(unsigned i) const { return _riList[i]; }
    CirRoGate* getRo(unsigned i) const { return _roList[i]; }
    // virtual CirRiGate* getRi(unsigned i) const { return 0; }
    // virtual CirRoGate* getRo(unsigned i) const { return 0; }
    CirPiGate* getPi(unsigned i) const { return _piList[i]; }
    CirPoGate* getPo(unsigned i) const { return _poList[i]; }
    CirAigGate* getAig(unsigned i) const { return _aigList[i]; }
    GateVec& getFanouts(unsigned i) const { return _fanoutInfo[i]; }
    string getFileName() const { return _fileName; }
    FileType getFileType() const { return _fileType; }

    // Member functions about circuit construction
    // virtual bool readCircuit(){};
    virtual bool readCircuit() { return false; };
    bool readCircuitNew();
    void deleteCircuit();
    void genConnections();
    void genDfsList();
    void updateUndefList();
    void checkFloatList();
    void checkUnusedList();
    size_t checkConnectedGate(size_t);

    // Member functins about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void writeAag(ostream&) const;
    void writeBlif(const string&) const;
    void writeGate(ostream&, CirGate*) const;

    // Member functions about flags
    // for hidden command
    bool createMiter(CirMgr*, CirMgr*);
    static CirGate* _const0;
    // MODIFICATION FOR SOCV HOMEWORK
    void initCir(const int&, const int&, const int&, const int&);
    void buildBdd(CirGate*);
    void buildNtkBdd();
    void addTotGate(CirGate* gate) { _totGateList.push_back(gate); };
    const bool readCirFromAbc(string, FileType);
    const bool readBlif(const string&) const;
    const bool setBddOrder(const bool&);

    CirGate* createNotGate(CirGate*);
    CirGate* createAndGate(CirGate*, CirGate*);
    CirGate* createOrGate(CirGate*, CirGate*);
    CirGate* createXorGate(CirGate*, CirGate*);
    CirGate* _const1;

    // Member functions for creating gate from the Gia object
    void createInput(const int& idx, const int& gateId);
    void createOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, string poName);
    void createRi(const int& idx, const int& gateId, const int& in0Id, const int& inv);
    int createRo(const int& idx, const int& gateId, const FileType& fileType);
    void createRiRo(const int& riGid, const int& roGid);
    void createAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv);
    void createConst0();
    void createConst1();

    // Reorder the gate id for the ABC pAig
    void reorderGateId(IDMap& aigIdMap);

    // Engine Manager Pointer
    YosysMgr* getYosysMgr() { return _ysyMgr; }
    AbcMgr* getAbcMgr() { return _abcMgr; }

private:
    // unsigned _numDecl[TOT_PARSE_PORTS];
    unsigned _numDecl[TOT_GATE];
    PiVec _piList;
    PoVec _poList;
    RiVec _riList;
    RoVec _roList;
    AigVec _aigList;
    GateVec _totGateList;
    GateVec _dfsList;
    GateVec* _fanoutInfo;
    string _fileName;
    FileType _fileType;
    // Engine Managers
    AbcMgr* _abcMgr;
    YosysMgr* _ysyMgr;
};

class CirComb : public CirMgr {
public:
    CirComb() : CirMgr() {}
    ~CirComb() override {}
    CirType getCirType() { return CirType::COMB; }
    bool readCircuit() override;

private:
};

class CirSeq : public CirMgr {
public:
    CirSeq() : CirMgr() {}
    ~CirSeq() override {}
    CirType getCirType() { return CirType::SEQ; }
    CirRiGate* getRi(unsigned i) const { return _riList[i]; }
    CirRoGate* getRo(unsigned i) const { return _roList[i]; }
    bool readCircuit() override;

private:
    std::vector<CirRiGate*> _riList;
    std::vector<CirRoGate*> _roList;
};
