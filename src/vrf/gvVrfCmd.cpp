#ifndef GV_VRF_CMD_C
#define GV_VRF_CMD_C

#include "gvMsg.h"
#include "gvVrfCmd.h"
#include "gvUsage.h"
#include "gvAbcMgr.h"
#include "gvV3Mgr.h"
#include "util.h"
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>


using namespace std;

bool GVinitVrfCmd() {
    return (
         gvCmdMgr->regCmd("Formal Verify",      1, 1, new GVFormalVerifyCmd   ) 
    );
}

//----------------------------------------------------------------------
// Formal Verify [-bmc <int_depth> | -ubmc [<PO_idx> <PO_name>] | -pdr | -itp]
//----------------------------------------------------------------------

GVCmdExecStatus
GVFormalVerifyCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVFormalVerifyCmd " << endl;

    if( gvModMgr->getAigFileName() == "" ){
        gvMsg(GV_MSG_IFO) << "[ERROR]: Please use command \"READ DESIGN\" or \"VErilog2 Aig\" to read/make the aig file first !!\n";
        return GV_CMD_EXEC_NOP;
    }

    v3Mgr->init();

    bool bmc = false, pdr = false, itp = false, ubmc = false; 
    // bmc 
    int bmc_depth;
    // ubmc
    bool specifyPO = false; int PO_idx; char PO_name[128];
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
            // if too much option
            else if ((i+2) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            // if int_depth not an integer 
            else if (!strspn(options[i+1].c_str(), "0123456789")) 
            {
                cout << "[ERROR]: Please input an \"integer\" time frame for BMC (int_depth) !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
            else { bmc = true; bmc_depth = stoi(options[i+1]); }
        }
        else if (myStrNCmp("-ubmc", token, 5) == 0) 
        {
            // if not completely specify "PO_idx" and "PO_name"
            if ((i+2) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            // if too much option
            else if ((i+3) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            // if PO_idx not an integer 
            else if (!strspn(options[i+1].c_str(), "0123456789")) 
            {
                cout << "[ERROR]: Please input an \"integer\" index for UBMC PO (PO_idx) !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
            else 
            { 
                ubmc = true; specifyPO = true;
                PO_idx = stoi(options[i+1]);
                strcpy(PO_name, options[i+2].c_str());
            }
        }
        else if (myStrNCmp("-pdr", token, 4) == 0) 
        { 
            // if too much option
            if ((i+1) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            pdr = true; 
        }
        else if (myStrNCmp("-itp", token, 4) == 0) 
        { 
            // if too much option
            if ((i+1) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            itp = true; 
        }
    }

    // command 
    char Command[1024], inname[128];
    string cmd = "";
    string aigFileName = gvModMgr->getAigFileName();
    strcpy(inname, aigFileName.c_str());

    if (bmc || pdr || itp)
    {
        sprintf( Command, "read %s", inname ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
        sprintf( Command, "strash" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
    }
    // if specify multi-formal engine (-bmc 100 -pdr -itp), then execute all
    if (bmc) { cout << "\nSuccess: bmc " << endl; sprintf( Command, "bmc3 -F %d", bmc_depth ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    else if (pdr) { cout << "\nSuccess: pdr " << endl; sprintf( Command, "pdr" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    else if (itp) { cout << "\nSuccess: itp " << endl; sprintf( Command, "int" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
    else if (ubmc)
    {
        sprintf(Command, "read aig %s", inname);
        string command = string(Command);
        v3_exe = v3Mgr->parseCmd(command, cmd);
        v3_exe->exec(cmd);

        if (specifyPO)
        {
            sprintf(Command, "set safety %d -name %s", PO_idx, PO_name);
            command = string(Command);
            cmd = "";
            v3_exe = v3Mgr->parseCmd(command, cmd);
            v3_exe->exec(cmd);

            sprintf(Command, "verify umc %s", PO_name);
            command = string(Command);
            cmd = "";
            v3_exe = v3Mgr->parseCmd(command, cmd);
            v3_exe->exec(cmd);
        }
    }
    if ((!bmc) && (!pdr) && (!itp) && (!ubmc)) { cout << "\n[ERROR]: Please specify a formal technique ([-bmc <int_depth> | -ubmc [<PO_idx> <PO_name>] | -pdr | -itp ])" << endl; } 
    return GV_CMD_EXEC_DONE;
}

void
GVFormalVerifyCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: Formal Verify [-bmc <int_depth> | -ubmc [<PO_idx> <PO_name>] | -pdr | -itp ] " << endl;
}

void
GVFormalVerifyCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Formal Verify: " << "Use options to execute specific formal engine." << endl;
}

#endif