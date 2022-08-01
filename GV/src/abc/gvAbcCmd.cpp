#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "gvMsg.h"
#include "gvAbcCmd.h"
#include "gvAbcMgr.h"
#include <string>
#include <vector>
#include "util.h"

bool initAbcCmd() {
    if (abcMgr) delete abcMgr;
    abcMgr = new AbcMgr;
    return (
         gvCmdMgr->regCmd("ABCRead",      4, new GVABCReadCmd   ) ,
         gvCmdMgr->regCmd("ABCPrint",      4, new GVABCPrintCmd   ) 
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
// ABCPrint
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCPrintCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVABCPrintCmd  " << endl;
    abcMgr -> abcPrintDesign();
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

#endif