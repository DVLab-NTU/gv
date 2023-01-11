#ifndef GV_MOD_CMD_C
#define GV_MOD_CMD_C

#include "gvModCmd.h"
#include "gvAbcMgr.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "kernel/yosys.h"
#include "util.h"
#include <fstream>
#include <string>
#include <vector>

bool
GVinitModCmd() {
    if (gvModMgr) delete gvModMgr;
    gvModMgr = new GVModMgr;
    return (gvCmdMgr->regCmd("SEt SYStem", 2, 3, new GVSetSystemCmd) &&
            gvCmdMgr->regCmd("RESET SYStem", 5, 3, new GVResetCmd) &&
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
            else setup = true;

        } else if (myStrNCmp("vrf", token, 3) == 0) {
            if (setup) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else vrf = true;
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
GVResetCmd ::exec(const string& option) {

    bool delete_abc   = true;
    bool delete_yosys = true;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() > 1)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    if (options.size() == 1) {
        const string& token = options[0];
        if (myStrNCmp("abc", token, 1) == 0) {
            delete_yosys = false;
        } else if (myStrNCmp("yosys", token, 1) == 0) {
            delete_abc = false;
        } else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
        }
    }

    if (delete_abc) {
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), "empty");
        if (abcMgr) delete abcMgr;
        abcMgr = new AbcMgr;
    }

    if (delete_yosys) {
        Yosys::run_pass("delete");
    }

    myUsage.reset();
    if (gvModMgr) delete gvModMgr;
    gvModMgr = new GVModMgr;
}

void
GVResetCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: RESET SYStem [ -Abc | -Yosys ]" << endl;
    gvMsg(GV_MSG_IFO)
        << "       If engine is specified, only delete the ntk stored inside. "
           "However, it may be risky to only delete partial datas."
        << endl;
}

void
GVResetCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RESET SYStem: "
                      << "Delete all ntks in gv and reset to setup mode."
                      << endl;
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
    string      bound;

    while (1) {
        getline(infile, bound);
        if (stoi(bound) == 0) break;
        promptBound.push_back(stoi(bound));
        cout << bound << endl;
        for (int i = 0; i < stoi(bound); ++i) {
            getline(infile, prompt);
            gvModMgr->setWizardContent(prompt);
        }
    }

    // Start the GV tutorial wizard
    int  promptPos = 0, wizardIdx = 0, progress = 1;
    bool firstPrompt = true, debug = false;
    system("clear");
    while (status != GV_CMD_EXEC_QUIT) {
        gvModMgr->printWizardPrompt(promptPos, promptBound[wizardIdx]);
        if (!firstPrompt)
            gvModMgr->printWizardProgress(progress, promptBound.size());

        status = gvCmdMgr->execOneCmd();

        if (wizardIdx == promptBound.size() - 1) {
            system("clear");
            break;
        }

        if (status == GV_CMD_EXEC_DONE || firstPrompt || debug) {
            promptPos = promptBound[wizardIdx] + promptPos;
            wizardIdx++;
        }
        cout << endl;

        if (!firstPrompt) {
            if (status == GV_CMD_EXEC_DONE || debug) {
                gvModMgr->printWizardPrompt(-1, promptBound[wizardIdx]);
                progress++;
            } else gvModMgr->printWizardPrompt(-2, promptBound[wizardIdx]);
        } else firstPrompt = false;

        system("clear");
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
