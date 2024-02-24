/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "base/abc/abc.h"
#include "cirCut.h"
#include "cirDef.h"
#include "fileType.h"

extern CirMgr* cirMgr;

class AbcMgr;
class CirMgr {
    enum CirMgrFlag { NO_FEC = 0x1 };
    enum ParsePorts { VARS = 0,
                      PI,
                      LATCH,
                      PO,
                      AIG,
                      TOT_PARSE_PORTS };

public:
    CirMgr() : _flag(0), _piList(0), _poList(0), _totGateList(0), _fanoutInfo(0), _simLog(0) {}
    ~CirMgr() { deleteCircuit(); }

    // Abc Interface
    friend class AbcMgr;

    // Access functions
    CirGate* operator[](unsigned gid) const { return _totGateList[gid]; }
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate* getGate(unsigned gid) const {
        if (gid >= getNumTots()) {
            return 0;
        }
        return _totGateList[gid];
    }
    CirGate* litId2Gate(unsigned litId) const { return getGate(litId / 2); }
    CirGateV litId2GateV(unsigned litId) const;
    bool isFlag(CirMgrFlag f) const { return _flag & f; }
    void setFlag(CirMgrFlag f) const { _flag |= f; }
    void unsetFlag(CirMgrFlag f) const { _flag &= ~f; }
    void resetFlag() const { _flag = 0; }
    void setFileName(const string& f) { fileName = f; }

    unsigned getNumPIs() const { return _piList.size(); }
    unsigned getNumPOs() const { return _poList.size(); }
    unsigned getNumLATCHs() const { return _riList.size(); }
    unsigned getNumAIGs() const { return _aigList.size(); }
    unsigned getNumTots() const { return _totGateList.size(); }

    CirPiGate* getPi(unsigned i) const { return _piList[i]; }
    CirPoGate* getPo(unsigned i) const { return _poList[i]; }
    CirRiGate* getRi(unsigned i) const { return _riList[i]; }
    CirRoGate* getRo(unsigned i) const { return _roList[i]; }
    CirAigGate* getAig(unsigned i) const { return _aigList[i]; }
    GateList& getFanouts(unsigned i) const { return _fanoutInfo[i]; }

    string getFileName() const { return fileName; }

    // Member functions about circuit construction
    // bool readCircuit(const string&);
    void deleteCircuit();
    void genConnections();
    void genDfsList();
    void updateUndefList();
    void checkFloatList();
    void checkUnusedList();
    size_t checkConnectedGate(size_t);

    // Member functions about circuit optimization
    void sweep();
    void optimize();
    bool checkAigOptimize(CirGate*, const CirGateV&, const CirGateV&, CirGateV&) const;
    void deleteAigGate(CirGate*);
    void deleteUndefGate(CirGate*);

    // Member functions about simulation
    void randomSim();
    void fileSim(ifstream&);
    void setSimLog(ofstream* logFile) { _simLog = logFile; }
    void ReadSimVal();
    void cutSim(CirGate* rootGate, CirCut* cut);

    // Member functions about fraig
    void strash();
    IdList* getFECGrps(size_t i) const { return _fecGrps[i]; }
    void printFEC() const;
    void fraig();

    // Member functions about circuit reporting
    // Member functins about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream&) const;
    void writeBlif(const string&) const;
    void writeGate(ostream&, CirGate*) const;

    // Member functions about flags
    // for hidden command
    bool createMiter(CirMgr*, CirMgr*);
    static CirGate* _const0;
    // MODIFICATION FOR SOCV HOMEWORK
    void initCir(Gia_Man_t* pGia, const FileType& type);
    void initCir(const int&, const int&, const int&, const int&);
    void buildBdd(CirGate*);
    void buildNtkBdd();
    void addTotGate(CirGate* gate) { _totGateList.push_back(gate); };
    const bool readCirFromAbc(string, FileType);
    const bool readBlif(const string&) const;
    const bool setBddOrder(const bool&);
    // CirGate* createGate(const GateType& type);
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

    // Reorder the gate id
    void reorderGateId(IDMap& aigIdMap);

private:
    // unsigned _numDecl[TOT_PARSE_PORTS];
    unsigned _numDecl[TOT_GATE];
    mutable unsigned _flag;
    PiArray _piList;
    PoArray _poList;
    RiArray _riList;
    RoArray _roList;
    AigArray _aigList;
    // IDs in _undefList are NOT sorted!!
    IdList _undefList;
    // Make sure the IDs of the following lists are sorted!!
    IdList _floatList;   // gates with fanin(s) undefined
    IdList _unusedList;  // gates defined but not used
    GateArray _totGateList;
    GateList _dfsList;
    GateList* _fanoutInfo;
    vector<IdList*> _fecGrps;  // store litId; FECHash<GatePValue, IdList*>
    SimVector _fecVector;
    ofstream* _simLog;
    string fileName;

    // private member functions for circuit parsing
    bool parseHeader(ifstream&);
    // bool parseInput(ifstream&);
    bool parseLatch(ifstream&);
    bool parseOutput(ifstream&);
    bool parseAig(ifstream&);
    bool parseSymbol(ifstream&);
    bool parseComment(ifstream&);
    bool checkId(unsigned&, const string&);
    CirGate* checkGate(unsigned&, ParsePorts, const string&);

    // private member functions for circuit optimization
    CirGateV constSimplify(CirGate*, const CirGateV&, const CirGateV&) const;

    // private member functions about simulation
    void setRandPPattern() const;
    void setPPattern(SimPattern const patterns) const;
    unsigned gatherPatterns(ifstream&, SimPattern, size_t);
    void pSim1Pattern() const;
    void outputSimLog(size_t nPatterns = 64);

    // private member functions about FRAIG
    bool simAndCheckFEC();
    bool initFEC();
    bool checkFEC();
    bool checkFECRecur(const IdList&, vector<IdList*>&);
    void finalizeFEC();
    void simplifyFECGrps();
    void clearFECGrps();
    void initProofModel(SatSolver&, const GateList&);
    bool satCheckConst(SatSolver&, CirGate*, bool, SimPattern);
    bool satCheckFEC(SatSolver&, CirGate*, CirGate*, bool, SimPattern);
    void getSatAssignment(SatSolver&, SimPattern) const;
    void simplifyByEQ();
    void updateFECbySatPattern(SimPattern);
};

#endif  // CIR_MGR_H
