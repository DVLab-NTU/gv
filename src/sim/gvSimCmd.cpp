#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvSimCmd.h"

#include <fstream>
#include <string>

#include "cirMgr.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "util.h"
#include "yosysMgr.h"

bool initSimCmd() {
    return (gvCmdMgr->regCmd("RAndom Sim", 2, 1, new GVRandomSimCmd) &&
            gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafe));
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

    string opt, rst = "reset", rst_n = "reset", clk = "clk", in_file_name,
                out_file_name, command = "random_sim ",
                vcd_file_name = ".waves.vcd";
    string stimulus_file_name, cycles;

    bool verbose = false, rst_set = false, rst_n_set, clk_set = false;
    bool out_file_name_set = false, file_name_set = false;

    command +=
        "-top " + yosys_design->top_module()->name.str().substr(
                      1, strlen(yosys_design->top_module()->name.c_str()) - 1);
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
            cycles = options[i];
            command += " -sim_cycle " + cycles;
            continue;
        }
        if (myStrNCmp("-input", token, 1) == 0) {
            ++i;
            in_file_name  = options[i];
            file_name_set = true;
            command += " -input " + in_file_name;
            continue;
        }
        if (myStrNCmp("-output", token, 1) == 0) {
            ++i;
            out_file_name = options[i];
            command += " -output " + out_file_name;
            ofstream outfile;
            outfile.open(stimulus_file_name, ios::in);
            if (!outfile)
                return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
            outfile.close();
            continue;
        }
        if (myStrNCmp("-file", token, 1) == 0) {
            ++i;
            stimulus_file_name = options[i];
            command += " -file " + stimulus_file_name;
            ifstream infile;
            infile.open(stimulus_file_name, ios::in);
            if (!infile)
                return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
            infile.close();
            continue;
        }
        if (myStrNCmp("-vcd", token, 4) == 0) {
            ++i;
            vcd_file_name = options[i];
            command += " -vcd " + vcd_file_name;
            continue;
        }
    }
    // load the random_sim plugin in yosys
    // TODO: Fix the extension path
    const string importPlugin = "plugin -i ";
    run_pass(importPlugin + GV_SIMSO_PATH + "sim.so");

    if (!file_name_set) command += " -input " + cirMgr->getFileName();
    // cout << "safe  =========================== " + gvModMgr -> getSafe() <<
    // endl;
    if (gvModMgr->getSafe() != -1)
        command += " -safe " + std::to_string((gvModMgr->getSafe()));

    cout << command << "\n";
    // execute "random_sim" command
    run_pass(command);
    return GV_CMD_EXEC_DONE;
}

void GVRandomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO)
        << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] "
           "[-rst rst_name] [-rst_n rst_n_name] [-clk clk_name] "
           "[-output out_file_name] [-v verbose print result] [-file stimulus]"
        << endl;
}

void GVRandomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: "
                      << "Conduct random simulation and print the results."
                      << endl;
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------

GVCmdExecStatus
GVRandomSetSafe::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSetSafe " << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() != 1) {
        gvMsg(GV_MSG_IFO) << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    gvModMgr->setSafe(stoi(options[0]));
    return GV_CMD_EXEC_DONE;
}

void GVRandomSetSafe::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SAfe <#PO>" << endl;
}

void GVRandomSetSafe::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt SAfe: "
                      << "Set safe property for random sim." << endl;
}

#endif
