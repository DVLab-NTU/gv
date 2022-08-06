#ifndef GV_MOD_CMD_C
#define GV_MOD_CMD_C

#include "gvMsg.h"
#include "gvModMgr.h"
#include "gvModCmd.h"
#include <string>
#include <vector>
#include "util.h"

bool 
initModCmd() {
    if (gvModMgr) delete gvModMgr;
    gvModMgr = new GVModMgr;
    return (
         gvCmdMgr->regCmd("SEt System",   2,1,    new GVSetSystemCmd)
    );
}

GVCmdExecStatus
GVSetSystemCmd ::exec(const string& option) {
    if(!gvModMgr->getInputFileExist()){
        gvMsg(GV_MSG_IFO) << "[ERROR]: Please use command \"READ DESIGN\" to read the input file first !!\n";
        return GV_CMD_EXEC_NOP;
    }
    bool formal = false, simulation = false;
    string systemPromptStr;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() == 0) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string mode)>");
    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("formal", token, 1) == 0) {
            if (simulation) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {formal = true; systemPromptStr = "fv";}
        }
        else if (myStrNCmp("simulation", token, 1) == 0) {
            if (formal) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {simulation = true; systemPromptStr = "sm";}
        }
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    //gvCmdMgr->changeMode(GVCmdMode::GV_CMD_MODE_VERIFY);
    //gvCmdMgr->setModePrompt(systemPromptStr);

    return GV_CMD_EXEC_DONE;
}

void
GVSetSystemCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt System <formal | sim>" << endl;
}

void
GVSetSystemCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt System: " << "Switch to system mode." << endl;
}
#endif