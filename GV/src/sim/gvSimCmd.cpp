#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvMsg.h"
#include "gvSimCmd.h"
#include "gvAbcMgr.h"                                                                                                           
#include <string>
#include "util.h"

#include "kernel/yosys.h"
USING_YOSYS_NAMESPACE
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
    gvMsg(GV_MSG_IFO) << "I am GVRandomSimCmd " << endl;

    // load the random_sim plugin in yosys
    run_pass("plugin -i ./ext/sim.so");

    // check if "random_sim" in "help" list !!
    run_pass("help");

    // execute "random_sim" command
    run_pass("random_sim");
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: RAndom Sim " << endl;
}

void
GVRandomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: " << "Conduct random simulation and print the results." << endl;
}

#endif