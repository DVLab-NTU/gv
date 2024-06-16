
#ifndef CXX_MGR_H
#define CXX_MGR_H

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
    bool runVrltSim(const bool&);
    void fileSim(const bool&);
    void randomSim(const bool&);

private:
    PATH _itfTmpFile;
    PATH _itfFile;
    // std::string _itfPath;      // interface.hpp path
    std::string _itfFileName;  // save the name of interface.hpp file
    std::string _dirPath;      // verilator directory path
    std::string _designPath;   // input design path
    std::string _macro;        // save the GV_PATH and Design Info to Verilator with macro
    std::string _make;         // save the "make" command
    std::string _verboseCmd;   // save the command for not printing command message

    // private member functions for simluation on Verilator
    bool preDesignInfo(const bool&);
    bool genVrltBuild(const bool&);
    bool genCXXItf(const bool&);
    bool genVrltMakefile(const bool&);
    // private member functions to enable Simulation mode
    void enableFileSim();
    void enableRandomSim();
};

#endif