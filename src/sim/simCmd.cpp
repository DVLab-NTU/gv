#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "simCmd.h"

#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

#include "cirMgr.h"
#include "cxxMgr.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "simMgr.h"
#include "util.h"
#include "vcdMgr.h"
#include "vrltMgr.h"
#include "yosysMgr.h"

bool initSimCmd() {
    if (vrltMgr) delete vrltMgr;
    if (simMgr) delete simMgr;
    if (vcdMgr) delete vcdMgr;
    vcdMgr  = new VCDMgr();
    cxxMgr  = new CXXMgr();
    vrltMgr = new VRLTMgr();

    return (gvCmdMgr->regCmd("SEt SAfe", 2, 2, new GVRandomSetSafe) &&
            gvCmdMgr->regCmd("SEt VSIM", 2, 2, new SetVSim) &&
            gvCmdMgr->regCmd("VSIMulate", 4, new VSimulate) &&
            gvCmdMgr->regCmd("VCDPrint", 4, new VCDPrint));
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
    cirMgr->getYosysMgr()->setSafeProperty(stoi(options[0]));

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
// SEt VSIM <CXXrtl | VERilator>
//----------------------------------------------------------------------
GVCmdExecStatus SetVSim::exec(const string& option) {
    assert(cxxMgr != nullptr);
    assert(vrltMgr != nullptr);

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n              = options.size();
    std::string simulator = "";
    bool isCXX = false, isVRLT = false;

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("CXXrtl", token, 3) == 0) {
            if (isVRLT) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else isCXX = true;
            continue;
        }
        if (myStrNCmp("VERilator", token, 3) == 0) {
            if (isCXX) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
            else isVRLT = true;
            continue;
        }
    }

    if (isCXX) {
        simMgr    = cxxMgr;
        simulator = "CXXRTL";
    } else {
        simMgr    = vrltMgr;
        simulator = "VERILATOR";
    }

    fmt::print(fg(fmt::color::light_green), "Current simulator: {0}\n", simulator);
    return GV_CMD_EXEC_DONE;
}

void SetVSim::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt VSIM <CXXrtl | VERilator>" << endl;
}

void SetVSim::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt VSIM: "
                      << "Set the specified simulator (CXXRTL/Verilator)." << endl;
}
//----------------------------------------------------------------------
// VSIMulate <-File <string(patternFile)>| -Random <int(cycle)>> [-Output <string(fileName)>] [-Verbose]
//----------------------------------------------------------------------
GVCmdExecStatus VSimulate::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n       = options.size();
    string vcdFile = "", patternFile = "";
    bool random = false, verbose = false;
    int cycle = 0;
    if (cirMgr == nullptr) {
        gvMsg(GV_MSG_IFO) << "Please read the design !!" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, "");
    }

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-File", token, 2) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            patternFile = options[i];
        }
        if (myStrNCmp("-Output", token, 2) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            vcdFile = options[i];
        }
        if (myStrNCmp("-Random", token, 2) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            if (!myStr2Int(options[i], cycle))
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            random = true;
        }
        if (myStrNCmp("-Verbose", token, 2) == 0) {
            verbose = true;
        }
    }
    if (simMgr == nullptr) {
        fmt::print(fg(fmt::color::yellow),
                   "WARNING: Simulate using the default simulator (VERILATOR).");
        fmt::println("");
        simMgr = new VRLTMgr();
    }
    simMgr->setSimCycle(cycle);
    simMgr->setVcdFileName(vcdFile);
    simMgr->setPatternFileName(patternFile);

    if (random) simMgr->randomSim(verbose);
    else simMgr->fileSim(verbose);

    return GV_CMD_EXEC_DONE;
}

void VSimulate::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: VSIMulate <-File <string(patternFile)> | -Random <int(cycle)>>\n"
                      << "                 [-Output <string(fileName)>]\n"
                      << "                 [-Verbose]" << endl;
}

void VSimulate::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "VSIMulate: "
                      << "Simulate the verilog design with the specified simulator." << endl;
}

//----------------------------------------------------------------------
// VCDPrint <string<VCDFile>> [-Row <int(rowLimit)>] [-Column <int(columnLimit)>] [-List]
//----------------------------------------------------------------------
GVCmdExecStatus VCDPrint::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n       = options.size();
    string vcdFile = "";
    std::vector<std::string> selectedSignals;
    int rowLimit = 0, colLimit = 0;
    bool verbose = false, isList = false, isSelected = false;

    if (n < 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-COLumn", token, 4) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            if (!myStr2Int(options[i], colLimit))
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            vcdMgr->setColLimit(colLimit);
        } else if (myStrNCmp("-ROW", token, 4) == 0) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            if (!myStr2Int(options[i], rowLimit))
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            vcdMgr->setRowLimit(rowLimit);
        } else if (myStrNCmp("-List", token, 2) == 0) {
            isList = true;
        } else if (myStrNCmp("-Signal", token, 2) == 0) {
            if (++i == n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            }
            selectedSignals.push_back(options[i]);
            isSelected = true;
        } else {
            if (vcdFile.size())
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            vcdFile = options[i];
            ifstream infile;
            infile.open(options[i].c_str());
            if (!infile)
                return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
            infile.close();
        }
    }
    if (!vcdMgr->readVCDFile(vcdFile)) {
        cout << "ERROR: Cannot open the VCD file " << vcdFile << " !!\n";
        return GV_CMD_EXEC_NOP;
    }
    if (isList) vcdMgr->printAllSignalInfo();
    else if (isSelected) vcdMgr->printSignal(selectedSignals);
    else vcdMgr->printAllSignals();

    return GV_CMD_EXEC_DONE;
}

void VCDPrint::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: VCDPrint "
                      << "<string(VCDFile)>\n"
                      << "                [-COLumn<int(num)>]\n"
                      << "                [-Row<int(num)>]\n"
                      << "                [-Signal<string(signalName)>]" << endl;
}

void VCDPrint::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "VCDPrint: "
                      << "Print Verilog value change dump (VCD) files in tabular form." << endl;
}
#endif
