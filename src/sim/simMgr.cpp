#include "simMgr.h"

SimMgr* simMgr = nullptr;

SimMgr::SimMgr() : cycle(0) {
}

SimMgr::SimMgr(int c) : cycle(c) {
}