#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "abcCmd.h"

#include <string.h>

#include <vector>
#include <iomanip>

#include "abcMgr.h"

bool initAbcCmd() {
    if (abcMgr) delete abcMgr;
    abcMgr = new AbcMgr;
    return (gvCmdMgr->regCmd("ABCCMD", 6, new GVABCOriginalCmd));
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
    abcMgr->execCmd(Command);
}

void GVABCOriginalCmd::usage(const bool& verbose) const {
    cout << "Usage: ABCCMD <command in ABC> " << endl;
}

void GVABCOriginalCmd::help() const {
    cout << setw(20) << left << "ABCCMD: "
         << "Directly call ABC's command." << endl;
}

#endif
