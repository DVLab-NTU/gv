#ifndef GV_VRF_CMD_C
#define GV_VRF_CMD_C

#include "gvMsg.h"
#include "gvVrfCmd.h"
#include <string>
#include "util.h"

bool initVrfCmd() {
    return (
         gvCmdMgr->regCmd("Formal Verify",      1, 1, new GVFormalVerifyCmd   ) 
    );
}

//----------------------------------------------------------------------
// Formal Verify [-bmc <int_depth> | -pdr | -itp]
//----------------------------------------------------------------------

GVCmdExecStatus
GVFormalVerifyCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVFormalVerifyCmd " << endl;
    return GV_CMD_EXEC_DONE;
}

void
GVFormalVerifyCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Formal Verify " << endl;
}

void
GVFormalVerifyCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Formal Verify: " << "Use options to execute specific formal engine." << endl;
}

#endif