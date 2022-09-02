#ifndef GV_V3_MGR
#define GV_V3_MGR

#include <string>

#include "v3Msg.h"
#include "v3Usage.h"
#include "v3CmdMgr.h"
#include "v3StrUtil.h"
#include "v3NtkHandler.h"

using namespace std;

class V3Mgr;
extern V3Mgr* v3Mgr;
extern V3MsgMgr Msg;
extern V3Usage v3Usage;
extern V3CmdMgr* v3CmdMgr;
extern V3Handler v3Handler;
extern V3CmdExec* v3_exe;


class V3Mgr
{
    public:
        V3Mgr() { init(); }
        ~V3Mgr() {}

        void init();
        V3CmdExec* parseCmd(string raw_cmd, string& final_cmd);
};


#endif
