#include "gvV3Mgr.h"
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

V3Mgr* v3Mgr;

extern bool initAlgCmd();
extern bool initDfxCmd();
extern bool initNtkCmd();
extern bool initStgCmd();
extern bool initVrfCmd();
extern bool initV3MCCmd();
extern bool initTransCmd();
extern bool initCommonCmd();

string V3Msg::_allName = "";
ofstream V3Msg::_allout;
V3MsgMgr Msg;
V3Usage v3Usage;
V3CmdMgr* v3CmdMgr = new V3CmdMgr("v3");
V3Handler v3Handler;
V3CmdExec* v3_exe = 0;

void
V3Mgr::init()
{
    // initialize command library
    if (!(initAlgCmd() && initDfxCmd() && initNtkCmd() && initStgCmd() && initVrfCmd() &&
            initV3MCCmd() && initTransCmd() && initCommonCmd())) {
        cout << "ERROR: Command Register Failed !!!" << endl; exit(0);
    }
}

V3CmdExec*
V3Mgr::parseCmd(string raw_cmd, string& final_cmd)
{
    V3CmdExec* e_temp = 0;
    // space count: indicates how many words there are in cmd
    unsigned spCount = 0;
    for (size_t i = 0, n = raw_cmd.size(); i < n; ++i)
        if (raw_cmd[i] == ' ') ++spCount;

    // try to match commands
    size_t idx = 0;
    string cmd;
    for (unsigned i = 0; (i < spCount + 1) && i < 2; ++i)
    {
        idx = raw_cmd.find(' ', idx + 1);
        cmd = raw_cmd.substr(0, idx);

        e_temp = v3CmdMgr->getCmd(cmd);
        if (e_temp) { break; } 
    }
    size_t opt = raw_cmd.find_first_not_of(' ', idx);
    if (opt != string::npos)
        final_cmd = raw_cmd.substr(opt);
    
    // return 
    return e_temp;
}

