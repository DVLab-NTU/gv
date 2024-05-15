
#ifndef VRLT_MGR_H
#define VRLT_MGR_H

#include <string>

class VRLTMgr;
extern VRLTMgr* vrltMgr;

class VRLTMgr {
public:
    VRLTMgr();
    bool preVrltSim(const bool& verbose);
    bool runVrltSim(const bool& verbose);

private:
    std::string _itfPath;      // interface.hpp path
    std::string _dirPath;      // verilator directory path
    std::string _designPath;   // input design path
    std::string _itfFileName;  // save the name of interface.hpp file

    // private member functions for simluation on Verilator
    bool genVrltBuild(const bool& verbose);
    bool genVrltItf(const bool& verbose);
    bool genVrltMakefile(const bool& verbose);
};

#endif