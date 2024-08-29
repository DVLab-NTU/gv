/****************************************************************************
  FileName     [ satMgr.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       []
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#pragma once

#include <cassert>

#include "cirMgr.h"

namespace gv {
namespace sat {

// Top-level SAT solver manager
class SatSolverMgr {
    enum class Solver {
        MINISAT = 0,
        GLUCOSE
    };

public:
    /*SatSolverMgr(CirMgr*);*/
    SatSolverMgr(CirMgr*);
    ~SatSolverMgr();

private:
    CirMgr* _cirMgr;
    Solver _solver;
};

}  // namespace sat
}  // namespace gv
