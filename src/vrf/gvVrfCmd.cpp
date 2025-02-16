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

#include "ecoMgr.h"

using namespace std;

bool initVrfCmd() {
    return (gvCmdMgr->regCmd("PDR", 3, new GVPdrCmd) &&
    gvCmdMgr->regCmd("FUNCtional ECO", 4, 3, new GVFunctionalEcoCmd));
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

//----------------------------------------------------------------------
// FUNCtional ECO <-old old.v -new new.v>
//----------------------------------------------------------------------

GVCmdExecStatus
GVFunctionalEcoCmd::exec(const string& option) {
    // cout << "I am GVRandomSetSafe " << endl;

    string newName="", oldName="", inputFolder;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    cout << "option size = " << options.size() << endl;
    if (options.size() != 2) {
        cout << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    
    inputFolder = options[1];
    oldName = inputFolder + "g1.v";
    newName = inputFolder + "r2.v";

    
    
    gv::eco::EcoMgr* pEcoMgr = new gv::eco::EcoMgr;
    // EcoParams* pParams = new EcoParams(resynNewCircuit, doPrepatch, doCutMatch, outputSideNPN, reIte, doOutputMatch);
    pEcoMgr->doEco(oldName, newName);

    delete pEcoMgr;
    // delete pParams;
    
    // gvModMgr->setSafe(stoi(options[0]));
    return GV_CMD_EXEC_DONE;
}

void
GVFunctionalEcoCmd::usage(const bool& verbose) const {
    cout << "FUNCtional ECO <-old old.v -new new.v>" << endl;
}

void
GVFunctionalEcoCmd::help() const {
    cout << setw(20) << left << "FUNCtional ECO: "
                      << "Perform Functional ECO on the two given circuits as cad contest2021." << endl;
}


#endif