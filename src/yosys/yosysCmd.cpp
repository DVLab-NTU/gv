#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "yosysCmd.h"

#include <string>
#include <vector>

#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "util.h"
#include "yosysMgr.h"

bool initYosysCmd() {
    if (yosysMgr) delete yosysMgr;
    yosysMgr = new YosysMgr;
    yosysMgr->init();
    return (gvCmdMgr->regCmd("YSYSet", 4, new YosysSetCmd)) &&
           (gvCmdMgr->regCmd("PRint INfo", 2, 2, new YosysPrintInfoCmd)) &&
           (gvCmdMgr->regCmd("SHow", 2, new YosysShowCmd));
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
    size_t n   = options.size();
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
    cout << "Usage: YSYSet <-log (int enable)>" << endl;
}

void YosysSetCmd::help() const {
    cout << setw(20) << left << "YSYSet: "
         << "Set the option of Yosys." << endl;
}

GVCmdExecStatus
YosysPrintInfoCmd::exec(const string& option) {
    bool verbose = false;
    // check options
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    // try to match options
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-Verbose", token, 2) == 0) {
            verbose = true;
            continue;
        } else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
        }
    }
    yosysMgr->printDesignInfo(verbose);
    return GV_CMD_EXEC_DONE;
}

void YosysPrintInfoCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: PRint Info [-Verbose]" << endl;
}

void YosysPrintInfoCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "PRint Info: "
                      << "Print circuit information extracted by our parser."
                      << endl;
}

//----------------------------------------------------------------------
// SHow
//----------------------------------------------------------------------
GVCmdExecStatus
YosysShowCmd::exec(const string& option) {
    yosysMgr->showSchematic();
    return GV_CMD_EXEC_DONE;
}
void YosysShowCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO)
        << "Usage: SHow" << endl;
}

void YosysShowCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SHow : "
                      << "Show the schematic of the design." << endl;
}
#endif