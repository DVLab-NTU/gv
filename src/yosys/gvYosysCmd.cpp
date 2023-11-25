#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "gvYosysCmd.h"

#include <string>
#include <vector>

#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "gvYosysMgr.h"
#include "util.h"

bool initYosysCmd() {
    if (yosysMgr) delete yosysMgr;
    yosysMgr = new YosysMgr; yosysMgr->init();
    return (gvCmdMgr->regCmd("YSYSet", 4, new YosysSetCmd));
}

//----------------------------------------------------------------------
// YSYSet <-log (int enable)>
//----------------------------------------------------------------------
GVCmdExecStatus YosysSetCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.empty())
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    else if (options.size() > 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool doLog = false, isValid = false;
    int enable = 0;
    size_t n = options.size();
    if (myStrNCmp("-LOG", options[0], 4) == 0)
        doLog = true;
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    if (doLog) {
        if (myStr2Int(options[1], enable))
            isValid = true;
        else
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    }
    if (isValid) yosysMgr->setLogging(enable);

    return GV_CMD_EXEC_DONE;
}

void YosysSetCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: YSYSet <-log (int enable)>" << endl;
}

void YosysSetCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "YSYSet: "
                      << "Set the option of Yosys." << endl;
}

#endif