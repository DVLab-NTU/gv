/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
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
#include "cirDef.h"
#include "cirCut.h"

extern CirMgr* cirMgr;

enum CirFileType {
    VERILOG,
    AIGER
};

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
    // unsigned getNumPIs() const { return _numDecl[PI]; }
    // unsigned getNumPOs() const { return _numDecl[PO]; }
    unsigned getNumPIs() const { return _piList.size(); }
    unsigned getNumPOs() const { return _poList.size(); }
    unsigned getNumLATCHs() const { return _riList.size(); }
    unsigned getNumTots() const { return _totGateList.size(); }
    // unsigned getNumLATCHs() const { return _numDecl[LATCH]; }
    // unsigned getNumTots() const { return _numDecl[VARS] + _numDecl[PO] + 1; }
    CirPiGate* getPi(unsigned i) const { return _piList[i]; }
    CirPoGate* getPo(unsigned i) const { return _poList[i]; }
    CirRiGate* getRi(unsigned i) const { return _riList[i]; }
    CirRoGate* getRo(unsigned i) const { return _roList[i]; }
    GateList& getFanouts(unsigned i) const { return _fanoutInfo[i]; }

    // Member functions about circuit construction
    bool readCircuit(const string&);
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
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream&) const;
    void writeGate(ostream&, CirGate*) const;

    // Member functions about flags
    // for hidden command
    bool createMiter(CirMgr*, CirMgr*);
    static CirGate* _const0;
    // MODIFICATION FOR SOCV HOMEWORK
    void initCir(Gia_Man_t* pGia, const CirFileType& type);
    void initCir(Abc_Ntk_t* pNtk);
    void buildBdd(CirGate* gate);
    void buildNtkBdd();
    void addTotGate(CirGate* gate) { _totGateList.push_back(gate); };
    const bool readCirFromAbc(string fileName, CirFileType fileType);
    void readCirFromAbcNtk(Abc_Ntk_t* pNtk);
    const bool setBddOrder(const bool& file);
    // CirGate* createGate(const GateType& type);
    CirGate* createNotGate(CirGate* in0);
    CirGate* createAndGate(CirGate* in0, CirGate* in1);
    CirGate* createOrGate(CirGate* in0, CirGate* in1);
    CirGate* createXorGate(CirGate* in0, CirGate* in1);
    CirGate* _const1;

private:
    unsigned _numDecl[TOT_PARSE_PORTS];
    mutable unsigned _flag;
    PiArray _piList;
    PoArray _poList;
    RiArray _riList;
    RoArray _roList;
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

    // private member functions for circuit parsing
    bool parseHeader(ifstream&);
    bool parseInput(ifstream&);
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
