#pragma once


#include "cirMgr.h"

class CirECO : public CirMgr {
public:
    CirECO() : CirMgr() {}
    ~CirECO() override {}

private:
    CirMgr* _cir0;
    CirMgr* _cir1;
};

