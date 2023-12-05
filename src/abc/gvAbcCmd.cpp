#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "gvAbcCmd.h"

#include <string.h>

#include <vector>

#include "gvAbcMgr.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "util.h"

bool initAbcCmd() {
    if (abcMgr) delete abcMgr;
    abcMgr = new AbcMgr;
    return (gvCmdMgr->regCmd("ABCRead", 4, new GVABCReadCmd),
            gvCmdMgr->regCmd("ABCPrint", 4, new GVABCPrintCmd),
            gvCmdMgr->regCmd("ABCCMD", 6, new GVABCOriginalCmd));
}

//----------------------------------------------------------------------
// ABCRead <(string filename)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCReadCmd::exec(const string& option) {
    vector<string> options;
    string fileName = "";
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (fileName == "")
            fileName = options[i];
        else
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    if (fileName == "")
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING,
                                      "<(string fileName)>");
    abcMgr->abcReadDesign(fileName);
    return GV_CMD_EXEC_DONE;
}

void GVABCReadCmd::usage(const bool& verbose) const {
    cout << "Usage: ABCRead " << endl;
}

void GVABCReadCmd::help() const {
    cout << setw(20) << left << "ABCRead: "
         << "Read netlist by ABC." << endl;
}

//----------------------------------------------------------------------
// ABCPrint [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCPrintCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if (options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0)
            verbose = true;
        else
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr->get_abcNtkMgr())->printSummary();
    return GV_CMD_EXEC_DONE;
}

void GVABCPrintCmd::usage(const bool& verbose) const {
    cout << "Usage: ABCPrint " << endl;
}

void GVABCPrintCmd::help() const {
    cout << setw(20) << left << "ABCPrint: "
         << "Print netlist information." << endl;
}

//----------------------------------------------------------------------
// ABCCMD <command in ABC>
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCOriginalCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    string command;
    for (size_t i = 0; i < n; ++i) {
        command += options[i];
        if (i < n - 1) {
            command += " ";
        }
    }

    // calling abc's command
    char Command[1024], abcCmd[128];
    strcpy(abcCmd, command.c_str());
    sprintf(Command, "%s", abcCmd);
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
}

void GVABCOriginalCmd::usage(const bool& verbose) const {
    cout << "Usage: ABCCMD <command in ABC> " << endl;
}

void GVABCOriginalCmd::help() const {
    cout << setw(20) << left << "ABCCMD: "
         << "Directly call ABC's command." << endl;
}

#endif