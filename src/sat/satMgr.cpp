#include "satMgr.h"

namespace gv {
namespace sat {

/*SatSolverMgr::SatSolverMgr(CirMgr* c) : _cirMgr(c), _solver(Solver::MINISAT) {*/
/*}*/
SatSolverMgr::SatSolverMgr(CirMgr* c) : _cirMgr(c) {}

SatSolverMgr::~SatSolverMgr() {}

}  // namespace sat
}  // namespace gv
