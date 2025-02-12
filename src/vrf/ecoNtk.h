#ifndef ECO_NTK_H
#define ECO_NTK_H

#include "cirMgr.h"
#include "ecoMgr.h"
#include <vector>

class EcoNtk;
class EcoGate;
class EcoCir;

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
    vector<EcoGate*> GateList;
    // store the AIG version of the ntk
    EcoCir* pCir;
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
class EcoCir : public gv::cir::CirMgr {

};

#endif