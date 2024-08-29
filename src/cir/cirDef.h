/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once


#include <unordered_map>
#include <vector>

#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

class CirPValue;
class CirGateV;
class CirGate;
class CirPiGate;
class CirPoGate;
class CirRiGate;
class CirRoGate;
class CirAigGate;
class CirMgr;
class SatSolver;
class CirCut;
class CirCutMan;

typedef vector<unsigned> IdList;
typedef vector<CirGate*> GateVec;
typedef vector<CirGateV> GateVVec;
typedef vector<CirGate*> GateVec;
typedef vector<CirRiGate*> RiVec;
typedef vector<CirRoGate*> RoVec;
typedef vector<CirPiGate*> PiVec;
typedef vector<CirPoGate*> PoVec;
typedef vector<CirAigGate*> AigVec;
typedef HashMap<CirPValue, IdList*> FECHash;
typedef unordered_map<unsigned, unsigned> IDMap;
typedef size_t* SimPattern;
typedef vector<SimPattern> SimVector;

enum GateType {
    UNDEF_GATE = 0,
    PI_GATE = 1,
    PO_GATE = 2,
    AIG_GATE = 3,
    CONST_GATE = 4,
    RO_GATE = 5,
    RI_GATE = 6,

    TOT_GATE
};

enum class CirType {
    COMB = 0,
    SEQ = 1,
    ECO = 2,
};


