#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "gvMsg.h"
#include "gvAbcCmd.h"
#include "gvAbcMgr.h"
#include <string>
#include <vector>
#include "util.h"

bool GVinitAbcCmd() {
    if (abcMgr) delete abcMgr;
    abcMgr = new AbcMgr;
    return (
         gvCmdMgr->regCmd("ABCRead",           4, new GVABCReadCmd   ) ,
         gvCmdMgr->regCmd("ABCPrint",          4, new GVABCPrintCmd   ) ,
         gvCmdMgr->regCmd("ABCNtk2Aig",        4, new GVABCNtk2AigCmd   ) ,
         gvCmdMgr->regCmd("ABCSweep",          4, new GVABCSweepCmd   ) ,
         gvCmdMgr->regCmd("AIGPrint",          4, new GVAIGPrintCmd   ) ,
         gvCmdMgr->regCmd("AIGFraig",          4, new GVAIGFraigCmd   ) ,
         gvCmdMgr->regCmd("AIGRAndomSim",      5, new GVAIGRAndomSimCmd   )
    );
}

//----------------------------------------------------------------------
// ABCRead <(string filename)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCReadCmd ::exec(const string& option) {
    vector<string> options;
    string fileName = ""; 
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
      const string& token = options[i];
      if (fileName == "") fileName = options[i];
      else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    if (fileName == "") return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string fileName)>");
    abcMgr -> abcReadDesign(fileName);
    return GV_CMD_EXEC_DONE;
}

void
GVABCReadCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCRead " << endl;
}

void
GVABCReadCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCRead: " << "Read netlist by ABC." << endl;
}

//----------------------------------------------------------------------
// ABCPrint [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCPrintCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_abcNtkMgr()) -> printSummary();
    return GV_CMD_EXEC_DONE;
}

void
GVABCPrintCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCPrint " << endl;
}

void
GVABCPrintCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCPrint: " << "Print netlist information." << endl;
}

//----------------------------------------------------------------------
// ABCNtk2Aig
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCNtk2AigCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();
    abcMgr -> abcNtk2Aig();
    return GV_CMD_EXEC_DONE;
}

void
GVABCNtk2AigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCNtk2Aig " << endl;
}

void
GVABCNtk2AigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCNtk2Aig: " << "Transform netlist into AIG." << endl;
}

//----------------------------------------------------------------------
// ABCSweep [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCSweepCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_abcNtkMgr()) -> sweep(verbose);
    return GV_CMD_EXEC_DONE;
}

void
GVABCSweepCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: GVABCSweep " << endl;
}

void
GVABCSweepCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "GVABCSweep: " << "Do sweep operation on current netlist." << endl;
}

//----------------------------------------------------------------------
// AIGPrint [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGPrintCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> printSummary(verbose);
    return GV_CMD_EXEC_DONE;
}

void
GVAIGPrintCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGPrint " << endl;
}

void
GVAIGPrintCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGPrint: " << "Print AIG information." << endl;
}

//----------------------------------------------------------------------
// AIGFraig 
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGFraigCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> fraig();
    return GV_CMD_EXEC_DONE;
}

void
GVAIGFraigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGFraig " << endl;
}

void
GVAIGFraigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGFraig: " << "Do FRAIG on current netlist." << endl;
}

//----------------------------------------------------------------------
// AIGRAndomSim 
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGRAndomSimCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> randomSim();
    return GV_CMD_EXEC_DONE;
}

void
GVAIGRAndomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGRAndomSim " << endl;
}

void
GVAIGRAndomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGRAndomSim: " << "Random Simulation." << endl;
}

#endif