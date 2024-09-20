#include "simMgr.h"

//! Global Simulation Manager pointer
gv::sim::SimMgr* simMgr = nullptr;

namespace gv {
namespace sim {
SimMgr::SimMgr() : _cycle(0) {
}

SimMgr::SimMgr(int c) : _cycle(c) {
}

}  // namespace sim
}  // namespace gv
