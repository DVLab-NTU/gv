#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "gvSimCmd.h"

#include <fstream>
#include <string>
#include <vector>

#include "cirMgr.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "util.h"

using namespace std;

using gv::cir::CirMgr;

extern gv::cir::CirMgr* cirMgr;

//----------------------------------------------------------------------------
//  Command registration
//----------------------------------------------------------------------------
bool initSimCmd() {
    return (gvCmdMgr->regCmd("RAndom Sim", 2, 1, new GVRandomSimCmd) &&
            gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafeCmd));
}

//----------------------------------------------------------------------------
//  RAndom Sim
//----------------------------------------------------------------------------
GVCmdExecStatus GVRandomSimCmd::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSimCmd " << endl;

    if (!cirMgr) {
        cout << "[ERROR]: Please use command \"CIRRead\" to read the input file first !!\n";
        return GV_CMD_EXEC_ERROR;
    }

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    string rst = "reset", rst_n = "reset_n", clk = "clk";
    string inFileName, outFileName, vcdFileName = ".waves.vcd";
    string stimulusFileName, cycles;

    bool verbose         = false;
    bool rstSet          = false;
    bool rstNSet         = false;
    bool clkSet          = false;
    bool outFileSet      = false;
    bool inFileSet       = false;
    bool stimulusSet     = false;
    bool simCycleSet     = false;
    bool vcdFileSet      = false;

    // Base command
    string command = "random_sim ";

    // Top module name (from YosysMgr)
    auto* yMgr = cirMgr->getYosysMgr();
    string topModule = yMgr->getTopModuleName();
    if (topModule.empty()) {
        yMgr->saveTopModuleName();
        topModule = yMgr->getTopModuleName();
    }
    command += "-top " + topModule;

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-v", token, 1) == 0) {
            verbose = true;
            command += " -v";
            continue;
        }
        if (myStrNCmp("-rst_n", token, 4) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            rstNSet = true;
            rst_n   = options[i];
            command += " -reset_n " + rst_n;
            continue;
        }
        if (myStrNCmp("-rst", token, 3) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            rstSet = true;
            rst    = options[i];
            command += " -reset " + rst;
            continue;
        }
        if (myStrNCmp("-clk", token, 1) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            clkSet = true;
            clk    = options[i];
            command += " -clk " + clk;
            continue;
        }
        if (myStrNCmp("-sim_cycle", token, 1) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            simCycleSet = true;
            cycles      = options[i];
            command += " -sim_cycle " + cycles;
            continue;
        }
        if (myStrNCmp("-input", token, 1) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            inFileName = options[i];
            inFileSet  = true;
            command += " -input " + inFileName;
            continue;
        }
        if (myStrNCmp("-output", token, 1) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            outFileName = options[i];
            outFileSet  = true;
            command += " -output " + outFileName;

            ofstream outfile;
            outfile.open(outFileName.c_str(), ios::out);
            if (!outfile)
                return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
            outfile.close();
            continue;
        }
        if (myStrNCmp("-file", token, 1) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            stimulusFileName = options[i];
            stimulusSet      = true;
            command += " -file " + stimulusFileName;
            ifstream infile;
            infile.open(stimulusFileName.c_str(), ios::in);
            if (!infile)
                return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
            infile.close();
            continue;
        }
        if (myStrNCmp("-vcd", token, 4) == 0) {
            if (++i >= n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            vcdFileName = options[i];
            vcdFileSet  = true;
            command += " -vcd " + vcdFileName;
            continue;
        }
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }

    // If input file not specified, use the circuit's file name
    if (!inFileSet) {
        inFileName = cirMgr->getFileName();
        if (inFileName.empty())
            return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "-input");
        command += " -input " + inFileName;
    }

    // Safe property index (if set)
    unsigned propIdx = yMgr->getSafeProperty();
    if (propIdx != static_cast<unsigned>(-1))
        command += " -safe " + std::to_string(propIdx);

    // Load the random_sim plugin in Yosys
    yMgr->loadSimPlugin();

    cout << command << "\n";
    // Execute "random_sim" command
    yMgr->runPass(command);
    return GV_CMD_EXEC_DONE;
}

void GVRandomSimCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO)
        << "Usage: RAndom Sim <-input file_name.v> [sim_cycle num_cycle_sim] "
           "[-rst rst_name] [-rst_n rst_n_name] [-clk clk_name] "
           "[-output out_file_name] [-v verbose print result] [-file stimulus]"
        << endl;
}

void GVRandomSimCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "RAndom Sim: "
                      << "Conduct random simulation and print the results."
                      << endl;
}

//----------------------------------------------------------------------------
//  SEt SAfe
//----------------------------------------------------------------------------
GVCmdExecStatus GVRandomSetSafeCmd::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVRandomSetSafeCmd " << endl;

    if (!cirMgr) {
        cout << "[ERROR]: Please use command \"CIRRead\" to read the input file first !!\n";
        return GV_CMD_EXEC_ERROR;
    }

    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() != 1) {
        gvMsg(GV_MSG_IFO) << "Please enter a valid value!" << endl;
        return GV_CMD_EXEC_DONE;
    }

    int idx = 0;
    if (!myStr2Int(options[0], idx) || idx < 0) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }

    cirMgr->getYosysMgr()->setSafeProperty(static_cast<unsigned>(idx));
    return GV_CMD_EXEC_DONE;
}

void GVRandomSetSafeCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt SAfe <#PO>" << endl;
}

void GVRandomSetSafeCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt SAfe: "
                      << "Set safe property for random sim." << endl;
}

#endif

