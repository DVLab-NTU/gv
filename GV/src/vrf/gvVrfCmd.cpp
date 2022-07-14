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
    Msg(MSG_IFO) << "I am GVFormalVerifyCmd " << endl;
}

void
GVFormalVerifyCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: Formal Verify " << endl;
}

void
GVFormalVerifyCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "Formal Verify: " << "Use options to execute specific formal engine." << endl;
}

#endif