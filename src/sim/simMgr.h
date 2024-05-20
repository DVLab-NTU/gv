#ifndef SIM_MGR_H
#define SIM_MGR_H

#include <vector>

#include "json.hpp"

class SimMgr;
extern SimMgr* simMgr;

enum class SimMode {
    RANDOM = 0,
    FILE
};

class SimMgr {
public:
    SimMgr();
    SimMgr(int);

    virtual void fileSim(const bool& verbose)   = 0;
    virtual void randomSim(const bool& verbose) = 0;

    void setSimCylce(int c) { cycle = c; }

    int getSimCycle() { return cycle; }

private:
    int cycle;
};

#endif
