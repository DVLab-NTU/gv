/****************************************************************************
  FileName     [ bddCmd.cpp ]
  PackageName  [ bdd ]
  Synopsis     [ Define BDD commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "bddCmd.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "bddMgrV.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "gvMsg.h"
// #include "gvNtk.h"
#include "util.h"

using namespace std;

bool initBddCmd() {
    if (bddMgrV) delete bddMgrV;
    bddMgrV = new BddMgrV;
    return (gvCmdMgr->regCmd("BRESET", 6, new BResetCmd) &&
            gvCmdMgr->regCmd("BSETVar", 5, new BSetVarCmd) &&
            gvCmdMgr->regCmd("BINV", 4, new BInvCmd) &&
            gvCmdMgr->regCmd("BAND", 4, new BAndCmd) &&
            gvCmdMgr->regCmd("BOr", 3, new BOrCmd) &&
            gvCmdMgr->regCmd("BNAND", 5, new BNandCmd) &&
            gvCmdMgr->regCmd("BNOR", 4, new BNorCmd) &&
            gvCmdMgr->regCmd("BXOR", 4, new BXorCmd) &&
            gvCmdMgr->regCmd("BXNOR", 4, new BXnorCmd) &&
            gvCmdMgr->regCmd("BCOFactor", 4, new BCofactorCmd) &&
            gvCmdMgr->regCmd("BEXist", 3, new BExistCmd) &&
            gvCmdMgr->regCmd("BCOMpare", 4, new BCompareCmd) &&
            gvCmdMgr->regCmd("BSIMulate", 4, new BSimulateCmd) &&
            gvCmdMgr->regCmd("BREPort", 4, new BReportCmd) &&
            gvCmdMgr->regCmd("BDRAW", 5, new BDrawCmd) &&
            gvCmdMgr->regCmd("BSETOrder", 5, new BSetOrderCmd) &&
            gvCmdMgr->regCmd("BCONstruct", 4, new BConstructCmd));
    return true;
}

bool isValidBddName(const string& str) {
    int id;
    return (isValidVarName(str) || (myStr2Int(str, id) && id >= 0));
}

extern BddNodeV getBddNodeV(const string& bddName);
bool setBddOrder = false;

//----------------------------------------------------------------------
//    BRESET <(size_t nSupports)> <(size_t hashSize)> <(size_t cacheSize)>
//----------------------------------------------------------------------
GVCmdExecStatus
BResetCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 3) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 3) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[3]);
    }

    int nSupports, hashSize, cacheSize;
    if (!myStr2Int(options[0], nSupports) || (nSupports <= 0))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    if (!myStr2Int(options[1], hashSize) || (hashSize <= 0))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (!myStr2Int(options[2], cacheSize) || (cacheSize <= 0))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[2]);

    assert(bddMgrV != 0);
    bddMgrV->init(nSupports, hashSize, cacheSize);

    return GV_CMD_EXEC_DONE;
}

void BResetCmd::usage(const bool& verbose) const {
    cout
        << "Usage: BRESET <(size_t nSupports)> <(size_t hashSize)> "
        << "<(size_t cacheSize)>" << endl;
}

void BResetCmd::help() const {
    cout << setw(20) << left << "BRESET: "
         << "BDD reset" << endl;
}

//----------------------------------------------------------------------
//    BSETVar <(size_t level)> <(string varName)>
//----------------------------------------------------------------------
GVCmdExecStatus
BSetVarCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    int level;

    if (myStr2Int(options[0], level) && (level >= 1) &&
        (size_t(level) < bddMgrV->getNumSupports())) {
        BddNodeV n = bddMgrV->getSupport(level);
        if (!isValidVarName(options[1]) ||
            !bddMgrV->addBddNodeV(options[1], n()))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        return GV_CMD_EXEC_DONE;
    } else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    return GV_CMD_EXEC_DONE;
}

void BSetVarCmd::usage(const bool& verbose) const {
    cout << "Usage: BSETVar <(size_t level)> <(string varName)>"
         << endl;
}

void BSetVarCmd::help() const {
    cout << setw(20) << left << "BSETVar: "
         << "BDD set a variable name for a support" << endl;
}

//----------------------------------------------------------------------
//    BINV <(string varName)> <(string bddName)>
//----------------------------------------------------------------------
GVCmdExecStatus
BInvCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    if (!isValidBddName(options[1]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    BddNodeV b = ::getBddNodeV(options[1]);
    if (b() == 0) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    bddMgrV->forceAddBddNodeV(options[0], (~b)());

    return GV_CMD_EXEC_DONE;
}

void BInvCmd::usage(const bool& verbose) const {
    cout << "Usage: BINV <(string varName)> <(string bddName)>"
         << endl;
}

void BInvCmd::help() const {
    cout << setw(20) << left << "BINV: "
         << "BDD Inv" << endl;
}

//----------------------------------------------------------------------
//    BAND <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BAndCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_one;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret &= b;
    }
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BAndCmd::usage(const bool& verbose) const {
    cout << "Usage: BAND <(string varName)> <(string bddName)>..."
         << endl;
}

void BAndCmd::help() const {
    cout << setw(20) << left << "BAND: "
         << "BDD And" << endl;
}

//----------------------------------------------------------------------
//    BOR <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BOrCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_zero;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret |= b;
    }
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BOrCmd::usage(const bool& verbose) const {
    cout << "Usage: BOR <(string varName)> <(string bddName)>..."
         << endl;
}

void BOrCmd::help() const {
    cout << setw(20) << left << "BOR: "
         << "BDD Or" << endl;
}

//----------------------------------------------------------------------
//    BNAND <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BNandCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_one;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret &= b;
    }
    ret = ~ret;
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BNandCmd::usage(const bool& verbose) const {
    cout << "Usage: BNAND <(string varName)> <(string bddName)>..."
         << endl;
}

void BNandCmd::help() const {
    cout << setw(20) << left << "BNAND: "
         << "BDD Nand" << endl;
}

//----------------------------------------------------------------------
//    BNOR <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BNorCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_zero;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret |= b;
    }
    ret = ~ret;
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BNorCmd::usage(const bool& verbose) const {
    cout << "Usage: BNOR <(string varName)> <(string bddName)>..."
         << endl;
}

void BNorCmd::help() const {
    cout << setw(20) << left << "BNOR: "
         << "BDD Nor" << endl;
}

//----------------------------------------------------------------------
//    BXOR <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BXorCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_zero;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret ^= b;
    }
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BXorCmd::usage(const bool& verbose) const {
    cout << "Usage: BXOR <(string varName)> <(string bddName)>..."
         << endl;
}

void BXorCmd::help() const {
    cout << setw(20) << left << "BXOR: "
         << "BDD Xor" << endl;
}

//----------------------------------------------------------------------
//    BXNOR <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------
GVCmdExecStatus
BXnorCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    size_t n = options.size();
    if (n < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    if (!isValidVarName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    BddNodeV ret = BddNodeV::_zero;
    for (size_t i = 1; i < n; ++i) {
        if (!isValidBddName(options[i]))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        BddNodeV b = ::getBddNodeV(options[i]);
        if (b() == 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        ret ^= b;
    }
    ret = ~ret;
    bddMgrV->forceAddBddNodeV(options[0], ret());

    return GV_CMD_EXEC_DONE;
}

void BXnorCmd::usage(const bool& verbose) const {
    cout << "Usage: BXNOR <(string varName)> <(string bddName)>..."
         << endl;
}

void BXnorCmd::help() const {
    cout << setw(20) << left << "BXNOR: "
         << "BDD Xnor" << endl;
}

//---------------------------------------------------------------------------
//    BCOFactor <-Positive|-Negative> <(string varName)> <(string bddName)>
//---------------------------------------------------------------------------
GVCmdExecStatus
BCofactorCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if (n < 3) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (n > 3) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[3]);

    bool posCof = false;
    if (myStrNCmp("-Positive", options[0], 2) == 0)
        posCof = true;
    else if (myStrNCmp("-Negative", options[0], 2) != 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    if (!isValidVarName(options[1]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (!isValidBddName(options[2]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[2]);
    BddNodeV f = ::getBddNodeV(options[2]);
    if (f() == 0) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[2]);
    unsigned level = f.getLevel();
    bddMgrV->forceAddBddNodeV(options[1], posCof ? f.getLeftCofactor(level)()
                                                 : f.getRightCofactor(level)());
    return GV_CMD_EXEC_DONE;
}

void BCofactorCmd::usage(const bool& verbose) const {
    cout << "Usage: BCOFactor <-Positive | -Negative> <(string "
            "varName)> <(string bddName)>\n";
}

void BCofactorCmd::help() const {
    cout << setw(20) << left << "BCOFactor: "
         << "Retrieve BDD cofactor\n";
}

//----------------------------------------------------------------------
//    BEXist <(size_t level)> <(string varName)> <(string bddName)>
//----------------------------------------------------------------------
GVCmdExecStatus
BExistCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if (n < 3) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (n > 3) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[3]);

    int level;
    if (!myStr2Int(options[0], level) || (level < 1) ||
        (size_t(level) >= bddMgrV->getNumSupports()))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    if (!isValidVarName(options[1]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (!isValidBddName(options[2]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[2]);
    BddNodeV f = ::getBddNodeV(options[2]);
    if (f() == 0) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[2]);
    bddMgrV->forceAddBddNodeV(options[1], f.exist(level)());

    return GV_CMD_EXEC_DONE;
}

void BExistCmd::usage(const bool& verbose) const {
    cout << "Usage: BEXist <(size_t level)> <(string varName)> "
            "<(string bddName)>\n";
}

void BExistCmd::help() const {
    cout << setw(20) << left << "BEXist: "
         << "Perform BDD existential quantification\n";
}

//----------------------------------------------------------------------
//    BCOMpare <(string bddName)> <(string bddName)>
//----------------------------------------------------------------------
GVCmdExecStatus
BCompareCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    if (!isValidBddName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    BddNodeV b0 = ::getBddNodeV(options[0]);
    if (b0() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    if (!isValidBddName(options[1]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    BddNodeV b1 = ::getBddNodeV(options[1]);
    if (b1() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);

    if (b0 == b1)
        cout << "\"" << options[0] << "\" and \"" << options[1]
             << "\" are equivalent." << endl;
    else if (b0 == ~b1)
        cout << "\"" << options[0] << "\" and \"" << options[1]
             << "\" are inversely equivalent." << endl;
    else
        cout << "\"" << options[0] << "\" and \"" << options[1]
             << "\" are not equivalent." << endl;

    return GV_CMD_EXEC_DONE;
}

void BCompareCmd::usage(const bool& verbose) const {
    cout << "Usage: BCOMpare <(string bddName)> <(string bddName)>"
         << endl;
}

void BCompareCmd::help() const {
    cout << setw(20) << left << "BCOMpare: "
         << "BDD comparison" << endl;
}

//----------------------------------------------------------------------
//    BSIMulate <(string bddName)> <(bit_string inputPattern)>
//----------------------------------------------------------------------
// input pattern = [01]*
//
GVCmdExecStatus
BSimulateCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    if (!isValidBddName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    BddNodeV node = ::getBddNodeV(options[0]);
    if (node() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int value = bddMgrV->evalCube(node, options[1]);
    if (value == -1)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);

    cout << "BDD Simulate: " << options[1] << " = " << value << endl;

    return GV_CMD_EXEC_DONE;
}

void BSimulateCmd::usage(const bool& verbose) const {
    cout
        << "Usage: BSIMulate <(string bddName)> <(bit_string inputPattern)>"
        << endl;
}

void BSimulateCmd::help() const {
    cout << setw(20) << left << "BSIMulate: "
         << "BDD simulation" << endl;
}

//----------------------------------------------------------------------
//    BREPort <(string bddName)> [-ADDRess] [-REFcount]
//            [-File <(string fileName)>]
//----------------------------------------------------------------------
GVCmdExecStatus
BReportCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.empty()) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

    bool doFile = false, doAddr = false, doRefCount = false;
    string bddNodeVName, fileName;
    BddNodeV bnode;
    for (size_t i = 0, n = options.size(); i < n; ++i) {
        if (myStrNCmp("-File", options[i], 2) == 0) {
            if (doFile)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            if (++i == n)
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING,
                                              options[i - 1]);
            fileName = options[i];
            doFile   = true;
        } else if (myStrNCmp("-ADDRess", options[i], 5) == 0) {
            if (doAddr)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            doAddr = true;
        } else if (myStrNCmp("-REFcount", options[i], 4) == 0) {
            if (doRefCount)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
            doRefCount = true;
        } else if (bddNodeVName.size())
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        else {
            bddNodeVName = options[i];
            if (!isValidBddName(bddNodeVName))
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, bddNodeVName);
            bnode = ::getBddNodeV(bddNodeVName);
            if (bnode() == 0)
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, bddNodeVName);
        }
    }

    if (!bddNodeVName.size())
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (doAddr) BddNodeV::_debugBddAddr = true;
    if (doRefCount) BddNodeV::_debugRefCount = true;
    if (doFile) {
        ofstream ofs(fileName.c_str());
        if (!ofs)
            return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, fileName);
        ofs << bnode << endl;
    } else
        cout << bnode << endl;

    // always set to false afterwards
    BddNodeV::_debugBddAddr  = false;
    BddNodeV::_debugRefCount = false;

    return GV_CMD_EXEC_DONE;
}

void BReportCmd::usage(const bool& verbose) const {
    cout
        << "Usage: BREPort <(string bddName)> [-ADDRess] [-REFcount]\n "
        << "               [-File <(string fileName)>]" << endl;
}

void BReportCmd::help() const {
    cout << setw(20) << left << "BREPort: "
         << "BDD report node" << endl;
}

//----------------------------------------------------------------------
//    BDRAW <(string bddName)> <(string fileName)>
//----------------------------------------------------------------------
GVCmdExecStatus
BDrawCmd::exec(const string& option) {
    // check option
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    if (!isValidBddName(options[0]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    if (::getBddNodeV(options[0])() == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    if (!bddMgrV->drawBdd(options[0], options[1]))
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);

    return GV_CMD_EXEC_DONE;
}

void BDrawCmd::usage(const bool& verbose) const {
    cout << "Usage: BDRAW <(string bddName)> <(string fileName)>" << endl;
}

void BDrawCmd::help() const {
    cout << setw(20) << left << "BDRAW: "
         << "BDD graphic draw" << endl;
}

//----------------------------------------------------------------------
//    BSETOrder < -File | -RFile >
//----------------------------------------------------------------------
GVCmdExecStatus
BSetOrderCmd::exec(const string& option) {
    if (setBddOrder) {
        gvMsg(GV_MSG_WAR) << "BDD Variable Order Has Been Set !!" << endl;
        return GV_CMD_EXEC_ERROR;
    }
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    }
    string token = options[0];
    bool file    = false;
    if (myStrNCmp("-File", token, 2) == 0)
        file = true;
    else if (myStrNCmp("-RFile", token, 3) == 0)
        file = false;
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    bddMgrV->restart();

    setBddOrder = cirMgr->setBddOrder(file);
    if (!setBddOrder)
        gvMsg(GV_MSG_ERR) << "Set BDD Variable Order Failed !!" << endl;
    else
        cout << "Set BDD Variable Order Succeed !!" << endl;
    return GV_CMD_EXEC_DONE;
}
void BSetOrderCmd::usage(const bool& verbose) const {
    cout << "Usage: BSETOrder < -File | -RFile >" << endl;
}
void BSetOrderCmd::help() const {
    cout << setw(20) << left << "BSETOrder: "
         << "Set BDD variable Order From Circuit." << endl;
}

//----------------------------------------------------------------------
//    BConstruct <-Netid <netId> | -Output <outputIndex> | -All>
//----------------------------------------------------------------------
GVCmdExecStatus
BConstructCmd::exec(const string& option) {
    if (!setBddOrder) {
        gvMsg(GV_MSG_WAR) << "BDD variable order has not been set !!!" << endl;
        return GV_CMD_EXEC_ERROR;
    }

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if (options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }

    bool isGate = false, isOutput = false;
    if (myStrNCmp("-All", options[0], 2) == 0)
        cirMgr->buildNtkBdd();
    else if (myStrNCmp("-Gateid", options[0], 2) == 0)
        isGate = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0)
        isOutput = true;
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    if (isOutput || isGate) {
        if (options.size() != 2)
            return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[0]);

        int num = 0;
        CirGate* gate;
        if (!myStr2Int(options[1], num) || (num < 0))
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        if (isGate) {
            if ((unsigned)num >= cirMgr->getNumTots()) {
                gvMsg(GV_MSG_ERR) << "Gate with Id " << num << " does NOT Exist in Current Cir !!" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
            }
            gate = cirMgr->getGate(num);
        } else if (isOutput) {
            if ((unsigned)num >= cirMgr->getNumPOs()) {
                gvMsg(GV_MSG_ERR) << "Output with Index " << num << " does NOT Exist in Current Cir !!" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
            }
            gate = cirMgr->getPo(num);
        }
        cirMgr->buildBdd(gate);
    }

    return GV_CMD_EXEC_DONE;
}

void BConstructCmd::usage(const bool& verbose) const {
    cout << "Usage: BConstruct <-Gateid <gateId> | -Output <outputIndex> | -All > "
         << endl;
}

void BConstructCmd::help() const {
    cout << setw(20) << left << "BConstruct: "
         << "Build BDD From Current Design." << endl;
}
