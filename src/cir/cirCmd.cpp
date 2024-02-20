/****************************************************************************
  FileName     [ cirCmd.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define basic cir package commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirCmd.h"

#include <cassert>
#include <iomanip>
#include <iostream>

#include "cirGate.h"
#include "cirMgr.h"
#include "fileType.h"
#include "gvCmdMgr.h"
#include "gvModMgr.h"
#include "gvMsg.h"
#include "util.h"
#include "yosysMgr.h"

using namespace std;

extern CirMgr* cirMgr;
extern int effLimit;

bool initCirCmd() {
    return (gvCmdMgr->regCmd("CIRRead", 4, new CirReadCmd) &&
            gvCmdMgr->regCmd("CIRPrint", 4, new CirPrintCmd) &&
            gvCmdMgr->regCmd("CIRGate", 4, new CirGateCmd)) &&
           gvCmdMgr->regCmd("CIRWrite", 4, new CirWriteCmd);
}

enum CirCmdState {
    // Order matters! Do not change the order!!
    CIRINIT,
    CIRREAD,
    CIROPT,
    CIRSTRASH,
    CIRSIMULATE,
    // dummy end
    CIRCMDTOT
};

static CirCmdState curCmd = CIRINIT;

//----------------------------------------------------------------------
//    CIRRead <-Verilog | -Aiger> <(string fileName)> [-Replace]
//----------------------------------------------------------------------
GVCmdExecStatus CirReadCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    CirFileType fileType = VERILOG;

    if (options.empty())
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    bool doReplace = false;
    string fileName;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-Replace", options[i], 2) == 0) {
            if (doReplace) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            doReplace = true;
        } else if (myStrNCmp("-Verilog", options[i], 2) == 0) {
            fileType = VERILOG;
        } else if (myStrNCmp("-Aiger", options[i], 2) == 0) {
            fileType = AIGER;
        } else if (myStrNCmp("-Blif", options[i], 2) == 0) {
            fileType = BLIF;
        } else {
            if (fileName.size())
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            fileName = options[i];
        }
    }

    if (cirMgr != 0) {
        if (doReplace) {
            cerr << "Note: original circuit is replaced..." << endl;
            curCmd = CIRINIT;
            delete cirMgr;
            cirMgr = 0;
            yosysMgr->reset();
        } else {
            cerr << "Error: circuit already exists!!" << endl;
            return GV_CMD_EXEC_ERROR;
        }
    }
    cirMgr = new CirMgr;
    if (fileType == BLIF) {
        cirMgr->readBlif(fileName);
    } else {
        if (!cirMgr->readCirFromAbc(fileName, fileType)) {
            delete cirMgr;
            cirMgr = 0;
        }
        // Save the word-leve information
        if (fileType == VERILOG) yosysMgr->readVerilog(fileName);
    }
    // yosysMgr->writeAiger("vending.aig");

    return GV_CMD_EXEC_DONE;
}

void CirReadCmd::usage(const bool& verbose) const {
    cout << "Usage: CIRRead <-Verilog | -Aiger> <(string fileName)> [-Replace]" << endl;
}

void CirReadCmd::help() const {
    cout << setw(20) << left << "CIRRead:"
         << "Read in a circuit and construct the netlist" << endl;
}

//----------------------------------------------------------------------
//    CIRPrint [-Summary | -Netlist | -PI | -PO | -FLoating | -FECpairs]
//----------------------------------------------------------------------
GVCmdExecStatus
CirPrintCmd::exec(const string& option) {
    // check option
    string token;
    GVCmdExec::lexSingleOption(option, token);

    if (!cirMgr) {
        cerr << "Error: circuit is not yet constructed!!" << endl;
        return GV_CMD_EXEC_ERROR;
    }
    if (token.empty() || myStrNCmp("-Summary", token, 2) == 0)
        cirMgr->printSummary();
    else if (myStrNCmp("-Netlist", token, 2) == 0)
        cirMgr->printNetlist();
    else if (myStrNCmp("-PI", token, 3) == 0)
        cirMgr->printPIs();
    else if (myStrNCmp("-PO", token, 3) == 0)
        cirMgr->printPOs();
    else if (myStrNCmp("-FLoating", token, 3) == 0)
        cirMgr->printFloatGates();
    else if (myStrNCmp("-FECpairs", token, 4) == 0)
        cirMgr->printFECPairs();
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);

    return GV_CMD_EXEC_DONE;
}

void CirPrintCmd::usage(const bool& verbose) const {
    cout << "Usage: CIRPrint [-Summary | -Netlist | -PI | -PO | -FLoating | -FECpairs]" << endl;
}

void CirPrintCmd::help() const {
    cout << setw(20) << left << "CIRPrint: "
         << "Print circuit" << endl;
}

//----------------------------------------------------------------------
//    CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>
//----------------------------------------------------------------------
GVCmdExecStatus
CirGateCmd::exec(const string& option) {
    if (!cirMgr) {
        cerr << "Error: circuit has not been read!!" << endl;
        return GV_CMD_EXEC_ERROR;
    }

    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.empty())
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    int gateId = -1, level = 0;
    bool doFanin = false, doFanout = false;
    CirGate* thisGate = 0;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        bool checkLevel = false;
        if (myStrNCmp("-FANIn", options[i], 5) == 0) {
            if (doFanin || doFanout)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            doFanin    = true;
            checkLevel = true;
        } else if (myStrNCmp("-FANOut", options[i], 5) == 0) {
            if (doFanin || doFanout)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            doFanout   = true;
            checkLevel = true;
        } else if (!thisGate) {
            if (!myStr2Int(options[i], gateId) || gateId < 0)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            thisGate = cirMgr->getGate(gateId);
            if (!thisGate) {
                cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
            }
        } else if (thisGate)
            return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
        else
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        if (checkLevel) {
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            if (!myStr2Int(options[i], level) || level < 0)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            checkLevel = true;
        }
    }

    if (!thisGate) {
        cerr << "Error: Gate id is not specified!!" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options.back());
    }

    if (doFanin)
        thisGate->reportFanin(level);
    else if (doFanout)
        thisGate->reportFanout(level);
    else
        thisGate->reportGate();

    return GV_CMD_EXEC_DONE;
}

void CirGateCmd::usage(const bool& verbose) const {
    cout << "Usage: CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>" << endl;
}

void CirGateCmd::help() const {
    cout << setw(20) << left << "CIRGate:"
         << "Report a gate" << endl;
}

//----------------------------------------------------------------------
//    CIRWrite <-Aag [(int gateId)] | -Aig | -Blif> <-Output (string fileName)>
//----------------------------------------------------------------------
GVCmdExecStatus
CirWriteCmd::exec(const string& option) {
    if (!cirMgr) {
        cerr << "Error: circuit is not yet constructed!!" << endl;
        return GV_CMD_EXEC_ERROR;
    }
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.empty()) {
        cirMgr->writeAag(cout);
        return GV_CMD_EXEC_DONE;
    }
    CirFileType fileType;
    bool hasFile = false;
    int gateId;
    CirGate* thisGate = NULL;
    string outFileName;
    ofstream outfile;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-Aiger", options[i], 2) == 0) {
            fileType = AIGER;
        } else if (myStrNCmp("-Aag", options[i], 2) == 0) {
            fileType = AAG;
        } else if (myStrNCmp("-Blif", options[i], 2) == 0) {
            fileType = BLIF;
        } else if (myStrNCmp("-Output", options[i], 2) == 0) {
            if (hasFile)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i - 1]);
            outFileName = options[i].c_str();
            // outfile.open(options[i].c_str(), ios::out);
            // if (!outfile)
            // return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[1]);
            hasFile = true;
            cirMgr->setFileName(outFileName);
        } else if (myStr2Int(options[i], gateId) && gateId >= 0) {
            if (fileType != AAG)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            if (thisGate != NULL)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            thisGate = cirMgr->getGate(gateId);
            if (!thisGate) {
                cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            }
            if (!thisGate->isAig()) {
                cerr << "Error: Gate(" << gateId << ") is NOT an AIG!!" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
            }
        } else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
    }

    if (!hasFile) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "-Output");

    cout << outFileName << "\n";
    if (fileType == BLIF) yosysMgr->writeBlif(outFileName);
    else if (fileType == AIGER) yosysMgr->writeAiger(outFileName);
    else {
        if (!thisGate) {
            assert(hasFile);
            cirMgr->writeAag(outfile);
        } else if (hasFile) cirMgr->writeGate(outfile, thisGate);
        else cirMgr->writeGate(cout, thisGate);
    }
    return GV_CMD_EXEC_DONE;
}

void CirWriteCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: CIRWrite [(int gateId)][-Output (string aagFile)]" << endl;
}

void CirWriteCmd::help() const {
    cout << setw(15) << left << "CIRWrite: "
         << "write the netlist to an ASCII AIG file (.aag)\n";
}