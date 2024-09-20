#pragma once

#include <unistd.h>

#include <string>

namespace gv {
namespace sim {
class SimMgr;
}
}  // namespace gv

extern gv::sim::SimMgr* simMgr;

namespace gv {
namespace sim {

class SimMgr {
    enum class SimMode {
        RANDOM = 0,
        FILE
    };

public:
    SimMgr();
    SimMgr(int);

    virtual void fileSim(const bool& verbose)   = 0;
    virtual void randomSim(const bool& verbose) = 0;

    void setSimCycle(int c) { _cycle = c; }
    void setVcdFileName(const std::string& vcdFile) { _vcdFileName = vcdFile; }
    void setPatternFileName(const std::string& patternFile) { _patternFileName = patternFile; }

    std::string getVcdFileName() { return _vcdFileName; }
    std::string getPatternFileName() { return _patternFileName; }
    int getSimCycle() { return _cycle; }

private:
    int _cycle;
    std::string _vcdFileName;
    std::string _patternFileName;
};
}  // namespace sim
}  // namespace gv
