#ifndef CIR_CUT_H
#define CIR_CUT_H
#include <unordered_map>
#include "cirMgr.h"
#include "cirGate.h"

class CirCut {
    public:
    int cutSize;
    GateList leafNodes;
};

class CirCutMan {
    public:
    vector<CirCut*> gateGetCuts(CirGate* root, int k);
    unordered_map<CirGate*, vector<CirCut*>> gate2Cut;

};



#endif