#pragma once

#include <experimental/filesystem>
#include <string>

#include "simMgr.h"

class CXXMgr;
extern CXXMgr* cxxMgr;

class CXXMgr : public SimMgr {
    using PATH = std::experimental::filesystem::path;

public:
    CXXMgr();
    bool preCXXSim(const bool&);
    bool runCXXSim(const bool&);
    void fileSim(const bool&);
    void randomSim(const bool&);

private:
    PATH _itfTmpFile;
    PATH _itfFile;
    PATH _cxxSimFile;
    PATH _cxxDir;
    std::string _macro;       // save the GV_PATH and Design Info to Verilator with macro
    std::string _make;        // save the "make" command
    std::string _verboseCmd;  // save the command for not printing command message

    // private member functions for simluation on Verilator
    bool preDesignInfo(const bool&);
    bool genMakeMacro(const bool&);
    bool genCXXItf(const bool&);
    bool genCXXDriver(const bool&);
    bool genCXXExe(const bool&);
    // private member functions to enable Simulation mode
    void enableFileSim();
    void enableRandomSim();
};
