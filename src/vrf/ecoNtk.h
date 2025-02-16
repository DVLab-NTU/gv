#ifndef ECO_NTK_H
#define ECO_NTK_H

#include "cirMgr.h"
#include "cirGate.h"
#include <vector>

#include "abcMgr.h"
#include "yosysMgr.h"
#include "cirGate.h"
#include "cirDef.h"
// using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.
#include "gvType.h"
namespace gv {
namespace cir {
class EcoNtk;
class EcoGate;
class EcoCir;
class EcoCirGate;
}}

namespace gv {
namespace cir {

// used to store the primitive gate level network
class EcoNtk {
  public:
    // file parsing functions
    void parseNtkFile(const string& dir);
    void parseGate(const vector<string>& line);
  private:
    // PI list
    vector<EcoGate*> PIList;
    // PO list
    vector<EcoGate*> POList;
    // Gate List
    vector<EcoGate*> GateVec;
    // store the AIG version of the ntk
    EcoCir* cirV;
};


class EcoGate {
  public:
    string getGateTypeName();
    unsigned getGateType() { return _gateType; }
    unsigned getNumFanins() { return _fanins.size(); }
  private:
    // Use to represent the primitive gate
    enum EcoGateType {
      ECO_CONST_0_GATE = 0,
      ECO_CONST_1_GATE = 1,
      ECO_AND_GATE = 2,
      ECO_OR_GATE = 3,
      ECO_NAND_GATE = 4,
      ECO_NOR_GATE = 5,
      ECO_XOR_GATE = 6,
      ECO_XNOR_GATE = 7,
      ECO_BUF_GATE = 8,
      ECO_NOT_GATE = 9
    };
    unsigned _gateType;
    string _gateName;
    vector<EcoGate*> _fanins;
};

// inherited from CirMgr, modified to store some extra information for ECO usage
class EcoCir {
  
public:
EcoCir() : _ecoCirV(NULL) {}
  void readCirFromAbcNtk(Abc_Ntk_t* pNtk);
private:
  gv::cir::CirMgr* _ecoCirV;
  unordered_map<CirGate*, vector<CirGate*>> _fanoutMap; // record the fanout mapping of the gates
  
// enum CirMgrFlag { NO_FEC = 0x1 };
//     enum ParsePorts { VARS = 0,
//                       PI,
//                       LATCH,
//                       PO,
//                       AIG,
//                       TOT_PARSE_PORTS };
//     friend class EcoMgr;
// public:
//     EcoCir() : _flag(0), _piList(0), _poList(0), _totGateList(0), _fanoutInfo(0), _simLog(0) {}
//     ~EcoCir() { deleteCircuit(); }

//     // Access functions
//     CirGate* operator[](unsigned gid) const { return _totGateList[gid]; }
//     // return '0' if "gid" corresponds to an undefined gate.
//     CirGate* getGate(unsigned gid) const {
//         if (gid >= getNumTots()) {
//             return 0;
//         }
//         return _totGateList[gid];
//     }
//     CirGate* litId2Gate(unsigned litId) const { return getGate(litId / 2); }
//     CirGateV litId2GateV(unsigned litId) const;
//     bool isFlag(CirMgrFlag f) const { return _flag & f; }
//     void setFlag(CirMgrFlag f) const { _flag |= f; }
//     void unsetFlag(CirMgrFlag f) const { _flag &= ~f; }
//     void resetFlag() const { _flag = 0; }
//     void setFileName(const string& f) { _fileName = f; }
//     void setFileType(const FileType& t) { _fileType = t; }

//     unsigned getNumPIs() const { return _piList.size(); }
//     unsigned getNumPOs() const { return _poList.size(); }
//     unsigned getNumLATCHs() const { return _riList.size(); }
//     unsigned getNumAIGs() const { return _aigList.size(); }
//     unsigned getNumTots() const { return _totGateList.size(); }

//     CirPiGate* getPi(unsigned i) const { return _piList[i]; }
//     CirPoGate* getPo(unsigned i) const { return _poList[i]; }
//     CirRiGate* getRi(unsigned i) const { return _riList[i]; }
//     CirRoGate* getRo(unsigned i) const { return _roList[i]; }
//     CirAigGate* getAig(unsigned i) const { return _aigList[i]; }
//     GateVec& getFanouts(unsigned i) const { return _fanoutInfo[i]; }

//     string getFileName() const { return _fileName; }
//     FileType getFileType() const { return _fileType; }

//     // Member functions about circuit construction
//     bool readCircuit(const string fileName);
//     void deleteCircuit();
//     void genConnections(EcoCir* pCir);
//     void genDfsList();
//     void updateUndefList();
//     void checkFloatList();
//     void checkUnusedList();
//     size_t checkConnectedGate(size_t);

//     // Member functions about circuit optimization
//     void sweep();
//     void optimize();
//     bool checkAigOptimize(CirGate*, const CirGateV&, const CirGateV&, CirGateV&) const;
//     void deleteAigGate(CirGate*);
//     void deleteUndefGate(CirGate*);

//     // Member functions about simulation
//     void randomSim();
//     void randomSimOneWord();
//     void fileSim(ifstream&);
//     void setSimLog(ofstream* logFile) { _simLog = logFile; }
//     void ReadSimVal();
//     void cutSim(CirGate* rootGate, CirCut* cut);

//     // Member functions about fraig
//     void strash();
//     IdList* getFECGrps(size_t i) const { return _fecGrps[i]; }
//     void printFEC() const;
//     void fraig();

//     // Member functions about circuit reporting
//     // Member functins about circuit reporting
//     void printSummary() const;
//     void printNetlist() const;
//     void printPIs() const;
//     void printPOs() const;
//     void printFloatGates() const;
//     void printFECPairs() const;
//     void writeAag(ostream&) const;
//     void writeBlif(const string&) const;
//     void writeGate(ostream&, CirGate*) const;

//     // Member functions about flags
//     // for hidden command
//     bool createMiter(CirMgr*, CirMgr*);
//     static CirGate* _const0;
//     // MODIFICATION FOR SOCV HOMEWORK
//     void initCir(const int&, const int&, const int&, const int&);
//     void buildBdd(CirGate*);
//     void buildNtkBdd();
//     void addTotGate(CirGate* gate) { _totGateList.push_back(gate); };
//     const bool readCirFromAbc(string, FileType);
//     const bool readBlif(const string&) const;
//     const bool setBddOrder(const bool&);
//     // CirGate* createGate(const GateType& type);
//     CirGate* createNotGate(CirGate*);
//     CirGate* createAndGate(CirGate*, CirGate*);
//     CirGate* createOrGate(CirGate*, CirGate*);
//     CirGate* createXorGate(CirGate*, CirGate*);
//     CirGate* _const1;

//     // Function for building a sub-circuit formed by a cut
//     // void writeCutAag(EcoMgr* pEco, ostream& outfile, CirGate* rootGate, CirCut* cut, bool verbose);
//     // void writeCutAagWithConstInsert(EcoMgr* pEco, ostream& outfile, CirGate* rootGate, CirCut* cut, pair<CirGate*, bool> constInsert);
//     void collectCutConeGates(CirGate* rootGate, const CirCut* cut, GateVec& gates, bool verbose);
//     // Member functions for creating gate from the Gia object
//     void createInput(const int& idx, const int& gateId);
//     void createOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, string poName);
//     void createRi(const int& idx, const int& gateId, const int& in0Id, const int& inv);
//     int createRo(const int& idx, const int& gateId, const FileType& fileType);
//     void createRiRo(const int& riGid, const int& roGid);
//     void createAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv);
//     void createConst0();
//     void createConst1();

//     // Reorder the gate id for the ABC pAig
//     void reorderGateId(IDMap& aigIdMap);

//     // Andrew ECO functions
//     void addFanout(CirGate* gate, CirGate* fanoutGate) { _fanoutMap[gate].push_back(fanoutGate); }  
//     const bool readCirFromAbcNtk(Abc_Ntk_t* pNtk);

// private:
//     // unsigned _numDecl[TOT_PARSE_PORTS];
//     unsigned _numDecl[TOT_GATE];
//     mutable unsigned _flag;
//     PiVec _piList;
//     PoVec _poList;
//     RiVec _riList;
//     RoVec _roList;
//     AigVec _aigList;
//     // IDs in _undefList are NOT sorted!!
//     IdList _undefList;
//     // Make sure the IDs of the following lists are sorted!!
//     IdList _floatList;   // gates with fanin(s) undefined
//     IdList _unusedList;  // gates defined but not used
//     GateVec _totGateList;
//     GateVec _dfsList;
//     GateVec* _fanoutInfo;
//     vector<IdList*> _fecGrps;  // store litId; FECHash<GatePValue, IdList*>
//     SimVector _fecVector;
//     ofstream* _simLog;
//     string _fileName;
//     FileType _fileType;

//     // private member functions for circuit parsing
//     bool parseHeader(ifstream&);
//     bool parseInput(ifstream&);
//     bool parseLatch(ifstream&);
//     bool parseOutput(ifstream&);
//     bool parseAig(ifstream&);
//     bool parseSymbol(ifstream&);
//     bool parseComment(ifstream&);
//     bool checkId(unsigned&, const string&);
//     CirGate* checkGate(unsigned&, ParsePorts, const string&);

//     // private member functions for circuit optimization
//     CirGateV constSimplify(CirGate*, const CirGateV&, const CirGateV&) const;

//     // private member functions about simulation
//     void setRandPPattern() const;
//     void setPPattern(SimPattern const patterns) const;
//     unsigned gatherPatterns(ifstream&, SimPattern, size_t);
//     void pSim1Pattern() const;
//     void outputSimLog(size_t nPatterns = 64);

//     // private member functions about FRAIG
//     bool simAndCheckFEC();
//     bool initFEC();
//     bool checkFEC();
//     bool checkFECRecur(const IdList&, vector<IdList*>&);
//     void finalizeFEC();
//     void simplifyFECGrps();
//     void clearFECGrps();
//     void initProofModel(SatSolver&, const GateVec&);
//     bool satCheckConst(SatSolver&, CirGate*, bool, SimPattern);
//     bool satCheckFEC(SatSolver&, CirGate*, CirGate*, bool, SimPattern);
//     void getSatAssignment(SatSolver&, SimPattern) const;
//     void simplifyByEQ();
//     void updateFECbySatPattern(SimPattern);


};

}}


#endif