#ifndef GV_VRF_CMD_C
#define GV_VRF_CMD_C

#include "gvMsg.h"
#include "gvVrfCmd.h"
#include "gvUsage.h"
#include "gvAbcMgr.h"
#include "util.h"
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

extern AbcMgr* abcMgr;

bool GVinitVrfCmd() {
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

    if( gvModMgr->getAigFileName() == "" ){
        gvMsg(GV_MSG_IFO) << "[ERROR]: Please use command \"READ DESIGN\" or \"VErilog2 Aig\" to read/make the aig file first !!\n";
        return GV_CMD_EXEC_NOP;
    }

    bool bmc = false, pdr = false, itp = false;
    int bmc_depth;
    char fname[128];
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

	// get filename, formal method (can keep doing error handling) 
	size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        // cout << "option[" << i << "]: " << options[i] << endl;
        if (myStrNCmp("-bmc", token, 4) == 0) {
            // if no specify int_depth
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            // if int_depth not an integer 
            else if (!strspn(options[i+1].c_str(), "0123456789")) 
            {
                cout << "[ERROR]: Please input an \"integer\" time frame for BMC (int_depth) !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
            else { bmc = true; bmc_depth = stoi(options[i+1]); }
        }
        else if (myStrNCmp("-pdr", token, 4) == 0) { pdr = true; }
        else if (myStrNCmp("-itp", token, 4) == 0) { itp = true; }
    }

    // command 
    char Command[1024], inname[128];
    string aigFileName = gvModMgr->getAigFileName();
    strcpy(inname, aigFileName.c_str());
    sprintf( Command, "read %s", inname ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
    sprintf( Command, "strash" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
    // if specify multi-formal engine (-bmc 100 -pdr -itp), then execute all
    if (bmc) { cout << "\nSuccess: bmc " << endl; sprintf( Command, "bmc3 -F %d", bmc_depth ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    if (pdr) { cout << "\nSuccess: pdr " << endl; sprintf( Command, "pdr" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    if (itp) { cout << "\nSuccess: itp " << endl; sprintf( Command, "int" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    if ((!bmc) && (!pdr) && (!itp)) { cout << "\nSuccess: pdr (default) " << endl; sprintf( Command, "int" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); } // default pdr
    return GV_CMD_EXEC_DONE;
}

void
GVFormalVerifyCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Formal Verify [-bmc <int_depth> | -pdr | -itp] " << endl;
}

void
GVFormalVerifyCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Formal Verify: " << "Use options to execute specific formal engine." << endl;
}

#endif