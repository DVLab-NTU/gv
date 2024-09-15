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

namespace gv {
namespace cir {
class CirMgr;
class CirGateV;
class CirGate;
class CirPiGate;
class CirPoGate;
class CirRiGate;
class CirRoGate;
class CirAigGate;
class CirCut;
class CirCutMan;
class CirPValue;
}  // namespace cir
}  // namespace gv
class SatSolver;

typedef vector<unsigned> IdList;
typedef vector<gv::cir::CirGate*> GateVec;
typedef vector<gv::cir::CirGateV> GateVVec;
typedef vector<gv::cir::CirGate*> GateVec;
typedef vector<gv::cir::CirRiGate*> RiVec;
typedef vector<gv::cir::CirRoGate*> RoVec;
typedef vector<gv::cir::CirPiGate*> PiVec;
typedef vector<gv::cir::CirPoGate*> PoVec;
typedef vector<gv::cir::CirAigGate*> AigVec;
typedef vector<int> VarVec;
typedef HashMap<gv::cir::CirPValue, IdList*> FECHash;
typedef unordered_map<unsigned, unsigned> IDMap;
typedef size_t* SimPattern;
typedef vector<SimPattern> SimVector;

enum GateType {
    UNDEF_GATE = 0,
    PI_GATE    = 1,
    PO_GATE    = 2,
    AIG_GATE   = 3,
    CONST_GATE = 4,
    RO_GATE    = 5,
    RI_GATE    = 6,

    TOT_GATE
};

enum class CirType {
    COMB = 0,
    SEQ  = 1,
    ECO  = 2,
};
