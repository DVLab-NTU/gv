#include "appCmd.h"

#include <iomanip>

#include "appMgr.h"
#include "gvCmdMgr.h"

// GVCmdMgr* appCmdMgr = 0;

AppCmdMgr* appCmdMgr = 0;
bool initAppCmd() {
    if (!appCmdMgr) appCmdMgr = new AppCmdMgr;
    return (appCmdMgr->regCmd("SAVE", 4, new AppSaveCmd));
}

//----------------------------------------------------------------------
//    Save
//----------------------------------------------------------------------
GVCmdExecStatus AppSaveCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    return GV_CMD_EXEC_QUIT;

    return GV_CMD_EXEC_DONE;
}

void AppSaveCmd::usage(const bool& verbose) const {
    cout << "Usage: CIRRead <-Verilog | -Aiger | -Blif> <(string fileName)> [-Replace]" << endl;
}

void AppSaveCmd::help() const {
    cout << setw(20) << left << "SAVE:"
         << "Save the commands into the script." << endl;
}