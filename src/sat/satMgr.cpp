#include "satMgr.h"

namespace gv::sat {

SatSolverMgr::SatSolverMgr(CirMgr* c) : _cirMgr(c), _solver(Solver::MINISAT) {
}

SatSolverMgr::~SatSolverMgr() {}

}  // namespace gv::sat
