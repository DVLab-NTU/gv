#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "expCmd.h"

#include <cstddef>
#include <iomanip>

#include "cirMgr.h"
#include "glucoseMgr.h"
#include "gvMsg.h"
#include "iostream"
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
    //! Place your experimental functions and commands here

    gv_sat::GlucoseMgr* glcMgr = new gv_sat::GlucoseMgr(cirMgr);

    CirGate* realPo = cirMgr->getPo(0)->getIn0Gate();
    bool invert     = cirMgr->getPo(0)->getIn0().isInv();
    glcMgr->addBoundedVerifyData(cirMgr->getPo(0), 0);
    /*glcMgr->assertProperty(realPo, 1, 0);*/
    if (glcMgr->solve()) {
        cout << "SAT !!";
    } else {
        cout << "UNSAT !!";
    }

    return GV_CMD_EXEC_DONE;
}

void ExpCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Experiment " << endl;
}

void ExpCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << std::left << "EXPeriment: "
                      << "Command for the testing of the experimental functions." << endl;
}

#endif
