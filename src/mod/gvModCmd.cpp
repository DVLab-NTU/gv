#ifndef GV_MOD_CMD_C
#define GV_MOD_CMD_C

#include "gvModCmd.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "util.h"
#include <fstream>
#include <string>
#include <vector>

bool
GVinitModCmd() {
    if (gvModMgr) delete gvModMgr;
    gvModMgr = new GVModMgr;
    return (gvCmdMgr->regCmd("SEt SYStem", 2, 3, new GVSetSystemCmd),
            gvCmdMgr->regCmd("WIZard", 3, new GVSetWizardCmd));
}

GVCmdExecStatus
GVSetSystemCmd::exec(const string& option) {
    if (!gvModMgr->getInputFileExist()) {
        gvMsg(GV_MSG_IFO) << "[ERROR]: Please use command \"READ DESIGN\" to "
                             "read the input file first !!\n";
        return GV_CMD_EXEC_NOP;
    }

    bool           setup = false, vrf = false;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string mode)>");

    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("setup", token, 3) == 0) {
            if (vrf) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {
                setup = true;
            }
        } else if (myStrNCmp("vrf", token, 3) == 0) {
            if (setup) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {
                vrf = true;
            }
        } else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    if (setup) gvModMgr->setGVMode(GV_MOD_TYPE_SETUP);
    else if (vrf) gvModMgr->setGVMode(GV_MOD_TYPE_VERIFY);

    return GV_CMD_EXEC_DONE;
}

void
GVSetSystemCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SYStem <setup | vrf>" << endl;
}

void
GVSetSystemCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt System: "
                      << "Switch to setup/vrf mode." << endl;
}

GVCmdExecStatus
GVSetWizardCmd::exec(const string& option) {
    GVCmdExecStatus status         = GV_CMD_EXEC_DONE;
    bool            isFile         = false;
    string          wizardFileName = "";
    vector<string>  options;

    gvModMgr->setWizard(true);
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING,
                                      "<-File> <(string wizardFileName)>");

    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-File", token, 2) == 0) {
            if (isFile) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {
                isFile = true;
            }
        } else if (isFile) {
            if (wizardFileName != "")
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else {
                wizardFileName = token;
            }
        } else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    // Start read the wizard file content
    ifstream infile(wizardFileName);
    if (!infile) {
        gvMsg(GV_MSG_IFO) << "[ERROR]: Wizard file name \"" + wizardFileName +
                                 "\" not found !!\n";
        return GV_CMD_EXEC_ERROR;
    }

    string      prompt = "";
    vector<int> promptBound;
    // int         bound;
    string bound;
    while (1) {
        getline(infile, bound);
        if (stoi(bound) == 0) break;
        promptBound.push_back(stoi(bound));
        cout << bound << endl;
        for (int i = 0; i < stoi(bound); ++i) {
            getline(infile, prompt);
            gvModMgr->setWizardContent(prompt);
            cout << "Prompt : " << i << " -> " << prompt << endl;
        }
    }

    // Start the GV tutorial wizard
    int promptPos = 0;
    int wizardIdx = 0;
    while (status != GV_CMD_EXEC_QUIT) {
        system("clear");
        gvModMgr->printWizardPrompt(promptPos, promptBound[wizardIdx]);
        status = gvCmdMgr->execOneCmd();
        if (status == GV_CMD_EXEC_DONE || true) {
            promptPos = promptBound[wizardIdx] + promptPos;
            wizardIdx++;
        }
        if (wizardIdx == promptBound.size()) break;
        cout << endl;
    }
    return GV_CMD_EXEC_DONE;
}

void
GVSetWizardCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Type \"WIZard\" " << endl;
}

void
GVSetWizardCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "WIZard: "
                      << "Start the GV turtorial wizard." << endl;
}
#endif
