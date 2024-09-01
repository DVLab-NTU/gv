#include "appMgr.h"

#include <cassert>
#include <cstdlib>
#include <fstream>

#include "gvCmdMgr.h"
#include "gvType.h"
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

GVCmdExec* AppCmdMgr::parseCmd(string& appCmd) {
    vector<string> _currHistory = getHistory();
    assert(_currHistory.size());
    string str = _currHistory.back();
    assert(str[0] != 0 && str[0] != ' ');

    GVCmdExec* e = 0;
    string cmd;

    // normalize: keep only one space between two words
    unsigned delCount = 0;
    for (size_t i = 2, n = str.size(); i < n; ++i) {
        if ((str[i - 2] == ' ') && (str[i - 1] == ' ')) {
            for (size_t j = i; j < n; ++j) str[j - 1] = str[j];
            --i;
            ++delCount;
        }
    }
    str.erase(str.length() - delCount);

    // space count: indicates how many words there are in cmd
    unsigned spCount = 0;
    for (size_t i = 0, n = str.size(); i < n; ++i)
        if (str[i] == ' ') ++spCount;

    // try to match commands
    size_t idx = 0;
    for (unsigned i = 0; (i < spCount + 1) && i < 2; ++i) {
        idx = str.find(' ', idx + 1);
        cmd = str.substr(0, idx);

        e = getCmd(cmd);
        if (e) break;
    }

    // can't match any command
    if (!e) {
        // Chengyin added : Support linux commands
        string _cmd = "";
        if (str.size() >= 2) {
            // Support comments in dofile, NO execution
            if ((str[0] == '/') && (str[1] == '/')) return e;
        }
        for (idx = 0; idx < str.size(); ++idx) {
            if (str[idx] == ' ') break;
        }
        _cmd = str.substr(0, idx);
        if ((_cmd == "ls") || (_cmd == "vi") || (_cmd == "vim") ||
            (_cmd == "echo") || (_cmd == "cat") || (_cmd == "clear"))
            system(str.c_str());
        // else
        //     gvMsg(GV_MSG_ERR) << "Illegal command!! (" << str << ")" << endl;
    } else if (idx != string::npos) {
        size_t opt = str.find_first_not_of(' ', idx);
        if (opt != string::npos) appCmd = str.substr(opt);
    }

    return e;
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
        add_history(getHistory().back().c_str());
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
        cout << endl;
    }
    // Reset current mode to the setup mode
    modeMgr->setGVMode(MOD_TYPE_SETUP);
    return GV_CMD_EXEC_DONE;
}