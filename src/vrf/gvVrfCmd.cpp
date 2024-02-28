#ifndef GV_VRF_CMD_C
#define GV_VRF_CMD_C

#include "gvVrfCmd.h"

#include <stdio.h>

#include <cstring>
#include <iostream>
#include <string>

#include "abcMgr.h"
#include "cirDef.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

bool initVrfCmd() {
    return (gvCmdMgr->regCmd("PDR", 3, new GVPdrCmd));
}

//----------------------------------------------------------------------
//    PDR [-Verbose]
//----------------------------------------------------------------------
GVCmdExecStatus GVPdrCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    bool verbose = false;
    string fileName;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-Verbose", options[i], 2) == 0)
            verbose = true;
    }
    if (cirMgr) {
        IDMap aigIdMap;
        cirMgr->reorderGateId(aigIdMap);
        abcMgr->cirToAig(aigIdMap);
        abcMgr->runPDR(verbose);
    }
    return GV_CMD_EXEC_DONE;
}

void GVPdrCmd::usage(const bool& verbose) const {
    cout << "Usage: PDR [-Verbose]" << endl;
}

void GVPdrCmd::help() const {
    cout << setw(20) << left << "PDR:"
         << "Model checking using property directed reachability in abc." << endl;
}

#endif