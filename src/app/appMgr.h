#ifndef APP_MGR_H
#define APP_MGR_H

#include <string>

#include "gvCmdMgr.h"
using namespace std;

class AppCmdMgr;
extern AppCmdMgr* appCmdMgr;

class AppCmdMgr : public GVCmdMgr {
public:
    AppCmdMgr();
    GVCmdExecStatus execAppCmd();
    GVCmdExecStatus execAppCmd(const string& command);
    GVCmdExecStatus appCmdHandler();
    GVCmdExec* parseCmd(string&);

private:
};

#endif