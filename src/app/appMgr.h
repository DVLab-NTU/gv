#ifndef APP_MGR_H
#define APP_MGR_H

#include <fstream>
#include <string>

#include "gvCmdMgr.h"
#include "gvType.h"
using namespace std;

class AppCmdMgr;
extern AppCmdMgr* appCmdMgr;

class AppCmdMgr : public GVCmdMgr {
public:
    AppCmdMgr();
    GVCmdExecStatus execAppCmd();
    GVCmdExecStatus execAppCmd(const string& command);
    GVCmdExecStatus appCmdHandler();

private:
    ModType saveType;
    vector<string> _appCmdHistory;
};

#endif