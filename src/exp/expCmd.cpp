#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "expCmd.h"

#include <cstddef>

#include "cirMgr.h"
#include "gvMsg.h"
#include "util.h"

bool initExpCmd() {
    return (gvCmdMgr->regCmd("EXPeriment", 3, new ExpCmd));
}

//! Extern Declaration
extern void testVCDMgr();
extern void testGlucose();

//----------------------------------------------------------------------
// EXPeriment
//----------------------------------------------------------------------
GVCmdExecStatus ExpCmd::exec(const string& option) {
    //! place your experimental function here
    // testVCDMgr();
    // testGlucose();
    if (cirMgr != nullptr)
        cirMgr->getYosysMgr()->extractFSM();

    return GV_CMD_EXEC_DONE;
}

void ExpCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Experiment " << endl;
}

void ExpCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "EXPeriment: "
                      << "Command for the testing of the experimental functions." << endl;
}

#endif
