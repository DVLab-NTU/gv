#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvSimCmd.h"

#include <fstream>
#include <string>

#include "abcMgr.h"
#include "cirDef.h"
#include "cirMgr.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "kernel/yosys.h"
#include "util.h"

USING_YOSYS_NAMESPACE

bool initSimCmd() {
    return (gvCmdMgr->regCmd("RAndom Sim", 2, 1, new GVRandomSimCmd) &&
            gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafe) &&
            gvCmdMgr->regCmd("SHow", 2, new GVShowCmd));
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------
GVCmdExecStatus
GVRandomSimCmd ::exec(const string& option) {
    cout << "I am GVRandomSimCmd " << endl;
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
        if (myStrNCmp("-reset", token, 3) == 0) {
            rst_set = true;
            ++i;
            rst = options[i];
            // error handling: Yosys will rename "_" to "__", so please don't name the reset signal with underline, e.g. "i_rst"
            // TODO: fix the renaming rule in Yosys
            if (find(rst.begin(), rst.end(), '_') != rst.end()) {
                cout << "Please avoid naming the reset signal with underline, e.g. \"i_rst\" is illegal but \"rst\" is okay " << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, rst);
            }
            command += " -reset " + rst;
            continue;
        }
        if (myStrNCmp("-n_reset", token, 3) == 0) {
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
            // error handling: Yosys will rename "_" to "__", so please don't name the clock signal with underline, e.g. "i_clk"
            // TODO: fix the renaming rule in Yosys
            if (find(clk.begin(), clk.end(), '_') != clk.end()) {
                cout << "Please avoid naming the clock signal with underline, e.g. \"i_clk\" is illegal but \"clk\" is okay " << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, clk);
            }
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
            continue;
        }
        if (myStrNCmp("-file", token, 1) == 0) {
            ++i;
            stimulus_file_name = options[i];
            command += " -file " + stimulus_file_name;
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
    run_pass("plugin -i ./src/ext/sim.so");

    // if (!file_name_set) command += " -input " + gvModMgr->getInputFileName();
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
    cout << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] "
            "[-reset rst_name] [-n_reset rst_n_name] [-clk clk_name] "
            "[-output out_file_name] [-v verbose print result] [-file stimulus]"
         << endl;
}

void GVRandomSimCmd ::help() const {
    cout << setw(20) << left << "RAndom Sim: "
         << "Conduct random simulation and print the results."
         << endl;
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------

GVCmdExecStatus
GVRandomSetSafe::exec(const string& option) {
    cout << "I am GVRandomSetSafe " << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() != 1) {
        cout << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    gvModMgr->setSafe(stoi(options[0]));
    return GV_CMD_EXEC_DONE;
}

void GVRandomSetSafe::usage(const bool& verbose) const {
    cout << "Usage: SEt SAfe <#PO>" << endl;
}

void GVRandomSetSafe::help() const {
    cout << setw(20) << left << "SEt SAfe: "
         << "Set safe property for random simulation." << endl;
}

//----------------------------------------------------------------------
// SHow
//----------------------------------------------------------------------
GVCmdExecStatus
GVShowCmd::exec(const string& option) {
    cout << "I am GVSHowVCDCmd " << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n       = options.size();
    bool inputFile = false, vcd = false, schematic = false;

    string vcd_file_name = "";
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-SCHematic", token, 4) == 0) {
            if (schematic)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            schematic = true;
            continue;
        } else if (myStrNCmp("-Vcd", token, 2) == 0) {
            if (vcd) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            vcd = true;
            continue;
        } else {
            if (vcd) vcd_file_name = token;
            else if (!vcd)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            continue;
        }
    }

    if (vcd) {
        ifstream infile;
        infile.open(vcd_file_name);
        if (!infile.is_open()) {
            cout << "[ERROR]: Please input the VCD file name !!\n";
            return GV_CMD_EXEC_NOP;
        } else run_command("gtkwave " + vcd_file_name + " &");

        infile.close();
    } else if (schematic) {
        // Problem!! --- (2)
        if (cirMgr == 0) {
            cout << "[ERROR]: Please use command \"READ DESIGN\" to read the file first !!\n ";
            return GV_CMD_EXEC_NOP;
        }
        string top_module_name =
            yosys_design->top_module()->name.str().substr(1, strlen(yosys_design->top_module()->name.c_str()) - 1);

        run_pass("hierarchy -top " + top_module_name);
        run_pass("proc");
        run_pass("opt");
        run_pass("show");
    }

    return GV_CMD_EXEC_DONE;
}
void GVShowCmd::usage(const bool& verbose) const {
    cout << "Usage: SHow < -Vcd <string vcd_filename> | -SCHematic >" << endl;
}

void GVShowCmd::help() const {
    cout << setw(20) << left << "SHow : "
         << "Show the waveform or schematic." << endl;
}

#endif
