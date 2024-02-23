/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

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

typedef vector<CirGate*> GateList;
typedef vector<CirGateV> GateVList;
typedef vector<unsigned> IdList;
// typedef CirGate**                     GateArray;
typedef vector<CirGate*> GateArray;
// typedef CirPiGate**                   PiArray;
// typedef CirPoGate**                   PoArray;
// typedef CirRiGate**                   RiArray;
// typedef CirRoGate**                   RoArray;
typedef vector<CirRiGate*> RiArray;
typedef vector<CirRoGate*> RoArray;
typedef vector<CirPiGate*> PiArray;
typedef vector<CirPoGate*> PoArray;
typedef HashMap<CirPValue, IdList*> FECHash;
typedef vector<CirAigGate*> AigArray;
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

#endif  // CIR_DEF_H
