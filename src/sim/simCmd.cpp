#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "simCmd.h"

#include <fstream>
#include <string>

#include "cirMgr.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "simMgr.h"
#include "util.h"
#include "vrltMgr.h"
#include "yosysMgr.h"

bool initSimCmd() {
    if (vrltMgr) delete vrltMgr;
    if (simMgr) delete simMgr;
    // vrltMgr = new VRLTMgr();
    // simMgr  = new SimMgr();
    return (gvCmdMgr->regCmd("RAndom Sim", 2, 1, new GVRandomSimCmd) &&
            gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafe) &&
            gvCmdMgr->regCmd("VSIMulate", 4, new VSimulate));
}

//----------------------------------------------------------------------
// RAndom Sim
//----------------------------------------------------------------------
GVCmdExecStatus
GVRandomSimCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n   = options.size();
    string rst = "reset", rst_n = "reset", clk = "clk";
    string command = "random_sim ", vcd_file_name = ".waves.vcd", simulator = "verilator";
    string opt, in_file_name, out_file_name, stimulus_file_name, cycles;
    bool verbose = false, rst_set = false, rst_n_set, clk_set = false;
    bool out_file_name_set = false, file_name_set = false;

    command += "-top " + yosysMgr->getTopModuleName();
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
        if (myStrNCmp("-cxxrtl", token, 3)) {
            simulator = "cxxrtl";
            continue;
        }
    }

    // if (simulator == "verilator") {
    //     vrltMgr->preVrltSim(true);
    //     vrltMgr->runVrltSim(true);
    // }
    // } else
    // if (true) {
    if (!file_name_set) command += " -input " + cirMgr->getFileName();
    if (yosysMgr->getSafeProperty() != -1) command += " -safe " + to_string((yosysMgr->getSafeProperty()));
    // load the random_sim plugin in yosys
    yosysMgr->loadSimPlugin();
    // execute "random_sim" command
    yosysMgr->runPass(command);
    // }

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
// SEt SAfe <#PO>
//----------------------------------------------------------------------
GVCmdExecStatus
GVRandomSetSafe::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() != 1) {
        gvMsg(GV_MSG_IFO) << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }
    yosysMgr->setSafeProperty(stoi(options[0]));

    return GV_CMD_EXEC_DONE;
}

void GVRandomSetSafe::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SAfe <#PO>" << endl;
}

void GVRandomSetSafe::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt SAfe: "
                      << "Set safe property for random sim." << endl;
}

//----------------------------------------------------------------------
// VSIMulate <-File <string(patternFile)>| -Random> [-Cycle <int(cycleNum)>][-Output <string(fileName)>][-Verbose]
//----------------------------------------------------------------------
GVCmdExecStatus
VSimulate::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n       = options.size();
    string vcdFile = "", patternFile = "";
    bool random = false, verbose = false;
    int cycle = 0;

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-Cycle", token, 1) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            if (!myStr2Int(options[i], cycle))
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        if (myStrNCmp("-File", token, 1) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            patternFile = options[i];
        }
        if (myStrNCmp("-Output", token, 1) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            vcdFile = options[i];
        }
        if (myStrNCmp("-Random", token, 1) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            random = true;
        }
        if (myStrNCmp("-Verbose", token, 1) == 0) {
            verbose = true;
        }
    }
    if (simMgr == nullptr) {
        simMgr = new VRLTMgr();
    }
    simMgr->setSimCylce(cycle);
    simMgr->setVcdFileName(vcdFile);
    simMgr->setPatternFileName(patternFile);

    if (random) simMgr->randomSim(verbose);
    else simMgr->fileSim(verbose);

    return GV_CMD_EXEC_DONE;
}

void VSimulate::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: VSIMulate [-Cycle<int(cycleNum)>] [-Verbose]" << endl;
}

void VSimulate::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "VSIMulate: "
                      << "Simulate the verilog design with CXXRTL or Verilator." << endl;
}
#endif
