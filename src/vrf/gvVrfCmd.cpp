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
    int bmc_depth, bmc_S, bmc_T, bmc_H, bmc_G, bmc_C, bmc_D, bmc_J, bmc_I, bmc_P, bmc_Q, bmc_R;
    string bmc_option, bmc_L, bmc_W;
    // ubmc
    bool specifyPO = false; int PO_idx; char PO_name[128];
    char fname[128];
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

	// get filename, formal method (can keep doing error handling) 
	size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           BMC
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!ubmc) && (!pdr) && (!itp) && !myStrNCmp("-bmc", token, 4)) {
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
        if (bmc && ((!myStrNCmp("-S", token, 2)) || (!myStrNCmp("-T", token, 2)) || (!myStrNCmp("-H", token, 2)) || (!myStrNCmp("-G", token, 2)) || (!myStrNCmp("-C", token, 2)) ||
                   (!myStrNCmp("-D", token, 2)) || (!myStrNCmp("-J", token, 2)) || (!myStrNCmp("-I", token, 2)) || (!myStrNCmp("-P", token, 2)) || (!myStrNCmp("-Q", token, 2)) || (!myStrNCmp("-R", token, 2))))
        {
            // if no specify <num>
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            // if int_depth not an integer 
            else if (!strspn(options[i+1].c_str(), "0123456789")) 
            {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
            else 
            { 
                string bmc_sub_option;
                if (!myStrNCmp("-S", token, 2)) { bmc_S = stoi(options[i+1]); bmc_sub_option = " -S " + to_string(bmc_S); }
                else if (!myStrNCmp("-T", token, 2)) { bmc_T = stoi(options[i+1]); bmc_sub_option = " -T " + to_string(bmc_T); }
                else if (!myStrNCmp("-H", token, 2)) { bmc_H = stoi(options[i+1]); bmc_sub_option = " -H " + to_string(bmc_H); }
                else if (!myStrNCmp("-G", token, 2)) { bmc_G = stoi(options[i+1]); bmc_sub_option = " -G " + to_string(bmc_G); }
                else if (!myStrNCmp("-C", token, 2)) { bmc_C = stoi(options[i+1]); bmc_sub_option = " -C " + to_string(bmc_C); }
                else if (!myStrNCmp("-D", token, 2)) { bmc_D = stoi(options[i+1]); bmc_sub_option = " -D " + to_string(bmc_D); }
                else if (!myStrNCmp("-J", token, 2)) { bmc_J = stoi(options[i+1]); bmc_sub_option = " -J " + to_string(bmc_J); }
                else if (!myStrNCmp("-I", token, 2)) { bmc_I = stoi(options[i+1]); bmc_sub_option = " -I " + to_string(bmc_I); }
                else if (!myStrNCmp("-P", token, 2)) { bmc_P = stoi(options[i+1]); bmc_sub_option = " -P " + to_string(bmc_P); }
                else if (!myStrNCmp("-Q", token, 2)) { bmc_Q = stoi(options[i+1]); bmc_sub_option = " -Q " + to_string(bmc_Q); }
                else if (!myStrNCmp("-R", token, 2)) { bmc_R = stoi(options[i+1]); bmc_sub_option = " -R " + to_string(bmc_R); } 
                bmc_option += bmc_sub_option;
            }
        }
        if (bmc && ((!myStrNCmp("-L", token, 2)) || (!myStrNCmp("-W", token, 2))))
        {
            // if no specify <num>
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            else 
            { 
                string bmc_sub_option;
                if (!myStrNCmp("-L", token, 2)) { bmc_L = options[i+1]; bmc_sub_option = " -L " + bmc_L; }
                if (!myStrNCmp("-W", token, 2)) { bmc_W = options[i+1]; bmc_sub_option = " -W " + bmc_W; }
                bmc_option += bmc_sub_option;
            }
        }
        if (bmc && ((!myStrNCmp("-a", token, 2)))) { bmc_option += " -a"; }
        if (bmc && ((!myStrNCmp("-x", token, 2)))) { bmc_option += " -x"; }
        if (bmc && ((!myStrNCmp("-d", token, 2)))) { bmc_option += " -d"; }
        if (bmc && ((!myStrNCmp("-u", token, 2)))) { bmc_option += " -u"; }
        if (bmc && ((!myStrNCmp("-r", token, 2)))) { bmc_option += " -r"; }
        if (bmc && ((!myStrNCmp("-s", token, 2)))) { bmc_option += " -s"; }
        if (bmc && ((!myStrNCmp("-g", token, 2)))) { bmc_option += " -g"; }
        if (bmc && ((!myStrNCmp("-v", token, 2)))) { bmc_option += " -v"; }
        if (bmc && ((!myStrNCmp("-z", token, 2)))) { bmc_option += " -z"; }

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                          UBMC
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!pdr) && (!itp) && !myStrNCmp("-ubmc", token, 5)) 
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

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           PDR
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!ubmc) && (!itp) && !myStrNCmp("-pdr", token, 4)) 
        { 
            // if too much option
            if ((i+1) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            pdr = true; 
        }

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           ITP
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!ubmc) && (!pdr) && !myStrNCmp("-itp", token, 4)) 
        { 
            // if too much option
            if ((i+1) < n) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            itp = true; 
        }
    }

    // command 
    char Command[1024], inname[128], formal_option[1024];
    string cmd = "";
    string aigFileName = gvModMgr->getAigFileName();
    strcpy(inname, aigFileName.c_str());
    if (bmc) { strcpy(formal_option, bmc_option.c_str()); }

    if (bmc || pdr || itp)
    {
        sprintf( Command, "read %s", inname ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
        sprintf( Command, "strash" ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
    }
    // if specify multi-formal engine (-bmc 100 -pdr -itp), then execute all
    if (bmc) { cout << "\nSuccess: bmc " << endl; sprintf( Command, "bmc3 -F %d%s", bmc_depth, formal_option ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command ); }
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
    // BMC
    gvMsg(GV_MSG_IFO) << "Usage: Formal Verify -bmc <int_depth> [-STHGCDJIPQR <num>] [-LW <filename>] [-axdursgvzh]" << endl;
    gvMsg(GV_MSG_IFO) << "\t         performs bounded model checking with dynamic unrolling" << endl;
    gvMsg(GV_MSG_IFO) << "\t-S num : the starting time frame" << endl;
    gvMsg(GV_MSG_IFO) << "\t-T num : runtime limit, in seconds" << endl;
    gvMsg(GV_MSG_IFO) << "\t-H num : runtime limit per output, in miliseconds (with \"-a\")" << endl;
    gvMsg(GV_MSG_IFO) << "\t-G num : runtime gap since the last CEX, in seconds" << endl;
    gvMsg(GV_MSG_IFO) << "\t-C num : max conflicts at an output" << endl;
    gvMsg(GV_MSG_IFO) << "\t-D num : max conflicts after jumping (0 = infinity)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-J num : the number of timeframes to jump (0 = not used)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-I num : the number of PIs to abstract" << endl;
    gvMsg(GV_MSG_IFO) << "\t-P num : the max number of learned clauses to keep (0=unused)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-Q num : delta value for learned clause removal" << endl;
    gvMsg(GV_MSG_IFO) << "\t-R num : percentage to keep for learned clause removal" << endl;
    gvMsg(GV_MSG_IFO) << "\t-L file: the log file name" << endl;
    gvMsg(GV_MSG_IFO) << "\t-W file: the log file name with per-output details" << endl;
    gvMsg(GV_MSG_IFO) << "\t-a     : solve all outputs (do not stop when one is SAT)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-x     : toggle storing CEXes when solving all outputs" << endl;
    gvMsg(GV_MSG_IFO) << "\t-d     : toggle dropping (replacing by 0) SAT outputs" << endl;
    gvMsg(GV_MSG_IFO) << "\t-u     : toggle performing structural OR-decomposition" << endl;
    gvMsg(GV_MSG_IFO) << "\t-r     : toggle disabling periodic restarts" << endl;
    gvMsg(GV_MSG_IFO) << "\t-s     : toggle using Satoko by Bruno Schmitt" << endl;
    gvMsg(GV_MSG_IFO) << "\t-g     : toggle using Glucose 3.0 by Gilles Audemard and Laurent Simon" << endl;
    gvMsg(GV_MSG_IFO) << "\t-v     : toggle verbose output" << endl;
    gvMsg(GV_MSG_IFO) << "\t-z     : toggle suppressing report about solved outputs" << endl;
    // UBMC
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -ubmc <PO_idx> <PO_name>" << endl;
    // PDR
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -pdr" << endl;
    // ITP
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -itp" << endl;
}

void
GVFormalVerifyCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Formal Verify: " << "Use options to execute specific formal engine." << endl;
}

#endif