#ifndef CIR_ECO_H
#define CIR_ECO_H

#include "cirMgr.h"

class CirECO : public CirMgr {
public:
    CirECO() : CirMgr() {}
    ~CirECO() override {}

private:
    CirMgr* _cir0;
    CirMgr* _cir1;
};

#endif