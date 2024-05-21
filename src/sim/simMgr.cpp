#include "simMgr.h"

SimMgr* simMgr = nullptr;

SimMgr::SimMgr() : _cycle(0) {
}

SimMgr::SimMgr(int c) : _cycle(c) {
}