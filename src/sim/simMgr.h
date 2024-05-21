#ifndef SIM_MGR_H
#define SIM_MGR_H

#include <string>

class SimMgr;
extern SimMgr* simMgr;

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

    inline void setSimCylce(int c) { _cycle = c; }
    inline void setVcdFileName(const std::string& vcdFile) { _vcdFileName = vcdFile; }
    inline void setPatternFileName(const std::string& patternFile) { _patternFileName = patternFile; }

    inline int getSimCycle() { return _cycle; }

    inline std::string getVcdFileName() { return _vcdFileName; }
    inline std::string getPatternFileName() { return _patternFileName; }

private:
    int _cycle;
    std::string _vcdFileName;
    std::string _patternFileName;
};

#endif
