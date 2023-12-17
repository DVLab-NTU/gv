#ifndef GV_CMD_COMM_C
#define GV_CMD_COMM_C

#include "gvCmdComm.h"

#include <string>

#include "gvMsg.h"
#include "gvUsage.h"
#include "util.h"


bool initCommonCmd() {
    return (gvCmdMgr->regCmd("DOfile", 2, new GVDofileCmd) &&
            gvCmdMgr->regCmd("HELp", 3, new GVHelpCmd) &&
            gvCmdMgr->regCmd("HIStory", 3, new GVHistoryCmd) &&
            gvCmdMgr->regCmd("USAGE", 5, new GVUsageCmd) &&
            gvCmdMgr->regCmd("Quit", 1, new GVQuitCmd));
}

//----------------------------------------------------------------------
// HELp [<(string cmd) [-Verbose]> | -Revealed]
//----------------------------------------------------------------------

GVCmdExecStatus
GVHelpCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    bool verbose = false, revealed = false;
    string cmd = "";

    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-Verbose", token, 2) == 0) {
            if (revealed)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            else if (verbose)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else verbose = true;
        } else if (myStrNCmp("-Revealed", token, 2) == 0) {
            if (verbose || cmd.size())
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            else if (revealed)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else revealed = true;
        } else {
            if (revealed)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            if (cmd.size()) cmd.append(" ");
            cmd.append(token);
        }
    }

    if (revealed) gvCmdMgr->printHelps(true);      // Print All Commands
    else if (!cmd.size()) gvCmdMgr->printHelps();  // Print Commands
    else {
        GVCmdExec* e = gvCmdMgr->getCmd(cmd);
        if (e) {
            e->usage(verbose);
            return GV_CMD_EXEC_DONE;
        }  // if exact match
        GVCmdExecSubSet list = gvCmdMgr->getCmdListFromPart(cmd);
        if (list.size()) {  // if partial match
            GVCmdExecSubSet::iterator it = list.begin();
            if (verbose)
                for (; it != list.end(); ++it) (*it)->usage();
            else
                for (; it != list.end(); ++it) (*it)->help();
            return GV_CMD_EXEC_DONE;
        } else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, cmd);
    }
    return GV_CMD_EXEC_DONE;
}

void GVHelpCmd::usage(const bool& verbose) const {
    cout << "Usage: HELp [<(string cmd) [-Verbose]>]" << endl;
    if (verbose) {
        cout
            << "Param: (string cmd): The (partial) name of the command."
            << endl;
        cout << "       -Verbose    : Print usage in more detail."
             << endl;
    }
}

void GVHelpCmd::help() const {
    cout << setw(20) << left << "HELp: "
         << "Print this help message." << endl;
}

//----------------------------------------------------------------------
// Quit [-Force]
//----------------------------------------------------------------------

GVCmdExecStatus
GVQuitCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() > 1)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if (options.size()) {
        if (myStrNCmp("-Forced", options[0], 2) == 0) return GV_CMD_EXEC_QUIT;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }

    cout << "Are you sure to quit (Yes/No)? [No] ";
    char str[1024];
    cin.getline(str, 1024);
    string ss = string(str);
    if (ss.size()) {
        size_t s = ss.find_first_not_of(' ', 0);
        if (s != string::npos) ss = ss.substr(s);
    }
    if (myStrNCmp("Yes", ss, 1) == 0) return GV_CMD_EXEC_QUIT;
    else return GV_CMD_EXEC_DONE;
}

void GVQuitCmd::usage(const bool& verbose) const {
    cout << "Usage: Quit [-Force]" << endl;
    if (verbose) {
        cout << "Param: -Force: Quit the program forcedly."
             << endl;
    }
}

void GVQuitCmd::help() const {
    cout << setw(20) << left << "Quit: "
         << "Quit the execution." << endl;
}

//----------------------------------------------------------------------
// HIStory [(int nPrint)]
//----------------------------------------------------------------------

GVCmdExecStatus
GVHistoryCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() > 1)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    int nPrint = -1;
    if (options.size() && !myStr2Int(options[0], nPrint))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    gvCmdMgr->printHistory(nPrint);
    return GV_CMD_EXEC_DONE;
}

void GVHistoryCmd::usage(const bool& verbose) const {
    cout << "Usage: HIStory [(int nPrint)]" << endl;
    if (verbose) {
        cout << "Param: (int nPrint): The number of the latest "
                "commands to be printed. (default = MAX)"
             << endl;
    }
}

void GVHistoryCmd::help() const {
    cout << setw(20) << left << "HIStory: "
         << "Print command history." << endl;
}

//----------------------------------------------------------------------
// DOfile <(string fileName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVDofileCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING,
                                      "<(string fileName)>");
    else if (options.size() > 1)
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if (!gvCmdMgr->openDofile(options[0])) {
        gvCmdMgr->closeDofile();
        return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void GVDofileCmd ::usage(const bool& verbose) const {
    cout << "Usage: DOfile <(string fileName)>" << endl;
    if (verbose) {
        cout
            << "Param: (string fileName): The file name of the script." << endl;
    }
}

void GVDofileCmd ::help() const {
    cout << setw(20) << left << "DOfile: "
         << "Execute the commands in the dofile." << endl;
}

//----------------------------------------------------------------------
// USAGE [-Time-only | -Memory-only] [-RESET]
//----------------------------------------------------------------------

GVCmdExecStatus
GVUsageCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    bool timeOnly = false, memoryOnly = false, reset = false;

    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-Time-only", token, 2) == 0) {
            if (timeOnly || memoryOnly)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else timeOnly = true;
        } else if (myStrNCmp("-Memory-only", token, 2) == 0) {
            if (timeOnly || memoryOnly)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else memoryOnly = true;
        } else if (myStrNCmp("-RESET", token, 6) == 0) {
            if (reset) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else reset = true;
        } else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }

    gvUsage.report(!memoryOnly, !timeOnly);
    if (reset) gvUsage.reset();
    return GV_CMD_EXEC_DONE;
}

void GVUsageCmd ::usage(const bool& verbose) const {
    cout << "Usage: USAGE [-Time-only | -Memory-only]" << endl;
    if (verbose) {
        cout
            << "Param: -Time-only  : Disable memory usage reporting." << endl;
        cout
            << "       -Memory-only: Disable time usage reporting." << endl;
    }
}

void GVUsageCmd ::help() const {
    cout << setw(20) << left << "USAGE: "
         << "Report resource usage." << endl;
}

#endif