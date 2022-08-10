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
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    string opt, rst = "reset", rst_n = "reset", clk = "clk", in_file_name, out_file_name, command = "random_sim ";
    bool verbose = false, rst_set = false, rst_n_set, clk_set = false;
    bool out_file_name_set = false, file_name_set = false;

    command += "-top " + yosys_design->top_module()->name.str().substr(1,strlen(yosys_design->top_module()->name.c_str()) - 1);
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-v", token, 1) == 0) {
            verbose = true;
            command += " -v ";
            continue;
        }
        if (myStrNCmp("-rst", token, 3) == 0) {
            rst_set = true;
            ++i;
            rst = options[i];
            command += " -reset " + rst;
            continue;
        }
        if (myStrNCmp("-rst_n", token, 4) == 0) {
            rst_n_set = true;
            ++i;
            rst_n = options[i];
            command += " -reset_n " + rst_n;
            continue;
        }
        if (myStrNCmp("-clk", token, 1) == 0) {
            clk_set = true;
            ++i;
            clk = options[i];
            command += " -clk " + clk;
            continue;
        }
        if (myStrNCmp("-sim_cycle", token, 1) == 0) {
            rst_n_set = true;
            ++i;
            rst_n = options[i];
            command += " -sim_cycle " + rst_n;
            continue;
        }
        if (myStrNCmp("-input", token, 1) == 0) {
            ++i;
            in_file_name = options[i];
            command += " -input " + in_file_name;
            continue;
        }
        if (myStrNCmp("-output", token, 1) == 0) {
            ++i;
            out_file_name = options[i];
            command += " -output " + out_file_name;
            continue;
        }
    }
    // load the random_sim plugin in yosys
    run_pass("plugin -i ./ext/sim.so");

    // execute "random_sim" command
    // command = "random_sim -top vendingMachine -reset_n reset -clk clk -sim_cycle 42 -input /home/yenlu_mepu/gv0/test_design/vending/vending-origin.v -output out.txt";
    run_pass(command);
    return GV_CMD_EXEC_DONE;
}

void
GVRandomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] [-rst rst_name] [-rst_n rst_n_name] [-clk clk_name] [-output out_file_name] [-v verbose print result]" << endl;
}

void
GVRandomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: " << "Conduct random simulation and print the results." << endl;
}

#endif