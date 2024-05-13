
#ifndef VRLT_MGR_H
#define VRLT_MGR_H

#include <string>

class VRLTMgr;
extern VRLTMgr* vrltMgr;

class VRLTMgr {
public:
    VRLTMgr();
    bool preVrltSim();
    bool genVrltItf();
    bool genVrltMakefile();

private:
    std::string _itfFileName;
    std::string _itfPath;
    std::string _dirPath;
};

#endif