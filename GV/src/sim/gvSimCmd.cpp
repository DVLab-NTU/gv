#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvMsg.h"
#include "gvSimCmd.h"
#include <string>
#include "util.h"

bool initSimCmd() {
    return (
         gvCmdMgr->regCmd("RAndom Sim",      2, 1, new GVRandomSimCmd   ) 
    );
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------

GVCmdExecStatus
GVRandomSimCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVRandomSimCmd " << endl;
}

void
GVRandomSimCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: RAndom Sim " << endl;
}

void
GVRandomSimCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "RAndom Sim: " << "Conduct random simulation and print the results." << endl;
}

#endif