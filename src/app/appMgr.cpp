#include "appMgr.h"

#include <cassert>
#include <cstdlib>
#include <fstream>

#include "gvCmdMgr.h"
#include "modMgr.h"

extern "C" {
#include <readline/history.h>
#include <readline/readline.h>
}

GVCmdExecStatus appCmdHandlerItf() { appCmdMgr->appCmdHandler(); }

AppCmdMgr::AppCmdMgr() : GVCmdMgr("app") {
    updateModPrompt("app");
}

GVCmdExecStatus AppCmdMgr::execAppCmd(const string& command) {
    system(command.c_str());
    return GV_CMD_EXEC_DONE;
}

GVCmdExecStatus AppCmdMgr::execAppCmd() {
    // Read User Command Input
    string str        = "";
    char* execCmd     = new char[1024];
    ifstream& _dofile = gvCmdMgr->getDofile();
    if (_dofile.is_open()) {
        getline(_dofile, str);
        strcpy(execCmd, str.c_str());
        // Detect dofile comment(#) and blank command
        if (execCmd[0] != '#' && str.size() > 0)
            cout << getPrompt() << execCmd << endl;
        if (_dofile.eof()) closeDofile();
    } else
        execCmd = readline(getPrompt().c_str());
    assert(execCmd);

    // Detect dofile comment(#) for debugging
    if (execCmd[0] == '#') return GV_CMD_EXEC_NOP;

    if (addHistory(execCmd)) {
        // add_history(_appCmdHistory.back().c_str());
        string option = "";
        string appCmd(execCmd);
        GVCmdExec* e = parseCmd(option);
        if (e) return e->exec(option);
        else execAppCmd(appCmd);
    }
    delete[] execCmd;
    return GV_CMD_EXEC_NOP;
}

GVCmdExecStatus AppCmdMgr::appCmdHandler() {
    GVCmdExecStatus status = GV_CMD_EXEC_DONE;
    while (status != GV_CMD_EXEC_QUIT) {
        appCmdMgr->setPrompt();
        status = execAppCmd();
    }
    modeMgr->setGVMode();
    return GV_CMD_EXEC_DONE;
}