#ifndef GV_BDD_CMD_C
#define GV_BDD_CMD_C

#include "gvMsg.h"
#include "gvBddCmd.h"
#include "gvBddMgr.h"
#include "util.h"

using namespace std;

bool GVinitBddCmd () {
    if (bddMgr) delete bddMgr;
    bddMgr = new BddMgr;
    return (
        gvCmdMgr->regCmd("BddInit",      4, new GVBddInitCmd ),
        gvCmdMgr->regCmd("BddReset",     4, new GVBddResetCmd ),
        gvCmdMgr->regCmd("BddAddVar",    4, new GVBddAddVarCmd ),
        gvCmdMgr->regCmd("BddSetVar",    4, new GVBddSetVarCmd ),
        gvCmdMgr->regCmd("BINV",         4, new GVBddInvCmd ),
        gvCmdMgr->regCmd("BAND",         4, new GVBddAndCmd ),
        gvCmdMgr->regCmd("BOR",          3, new GVBddOrCmd ),
        gvCmdMgr->regCmd("BXOR",         4, new GVBddXorCmd ),
        gvCmdMgr->regCmd("BNAND",        5, new GVBddNandCmd ),
        gvCmdMgr->regCmd("BNOR",         4, new GVBddNorCmd ),
        gvCmdMgr->regCmd("BXNOR",        4, new GVBddXnorCmd ),
        gvCmdMgr->regCmd("BREPort",      4, new GVBddReportCmd ),
        gvCmdMgr->regCmd("BDRAW",        5, new GVBddDrawCmd )
    );
}

//----------------------------------------------------------------------
// BddInit [(int nVars)]
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddInitCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    int num;
    if (options.size() == 0) bddMgr->init();
    else if (options.size() == 1) {
        if (myStr2Int(options[0], num) && num >= 0) bddMgr->init(num);
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    else {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddInitCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddInit <(size_t num)> " << endl;
}

void
GVBddInitCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddInit: " << "BDD initialization." << endl;
}

//----------------------------------------------------------------------
// BddReset 
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddResetCmd ::exec(const string& option) {
    bddMgr->reset();
    return GV_CMD_EXEC_DONE;
}

void
GVBddResetCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddReset " << endl;
}

void
GVBddResetCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddReset: " << "BDD reset." << endl;
}

//----------------------------------------------------------------------
// BddAddVar <(size_t num)> 
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddAddVarCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if(options.size() < 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if(options.size() > 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    }
    int num;
    if (myStr2Int(options[0], num) && num >= 0) {
        bddMgr->addNewVars((size_t(num)));
    } else {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddAddVarCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddAddVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddAddVarCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddAddVar: " << "Add new Bdd vars." << endl;
}

//----------------------------------------------------------------------
// BddSetVar <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddSetVarCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if(options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if(options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }
    int level;
    if (myStr2Int(options[0], level) && (level >= 1)) {
        DdNode* n = Cudd_bddNewVarAtLevel(bddMgr->getDdMgr(), level);
        BddNode newNode(n);
        if(!bddMgr->addBddNode(options[1], newNode)) {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
    }
    else {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddSetVarCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddSetVarCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BddInvCmd <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddInvCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if(options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if(options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddInvCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddInvCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BAND <(string varName)> <(string bddName)>...
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddAndCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode ret = BddNode::_one;
    BddNode tmp, node;
    for (size_t i=1; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret&node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }

    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddAndCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddAndCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BOR <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddOrCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode tmp, node;
    BddNode ret = bddMgr->getBddNode(options[1]);
    for (size_t i=2; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret|node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }

    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddOrCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddOrCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BXOR <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddXorCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode tmp, node;
    BddNode ret = bddMgr->getBddNode(options[1]);
    for (size_t i=2; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret^node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }

    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddXorCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddXorCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BNAND <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddNandCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode ret = BddNode::_one;
    BddNode tmp, node;
    for (size_t i=1; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret&node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }

    tmp = ~ret;
    ret = tmp;
    Cudd_Ref(tmp.getNode());
    Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());

    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddNandCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddNandCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BNOR <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddNorCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode tmp, node;
    BddNode ret = bddMgr->getBddNode(options[1]);
    for (size_t i=2; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret|node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }
    tmp = ~ret;
    ret = tmp;
    Cudd_Ref(tmp.getNode());
    Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddNorCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddNorCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BXNOR <(size_t level)> <(string varName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddXnorCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    }
    BddNode tmp, node;
    BddNode ret = bddMgr->getBddNode(options[1]);
    for (size_t i=2; i<n; ++i) {
        if (bddMgr->isBddNode(options[i])) {
            node = bddMgr->getBddNode(options[i]);
        }
        else {
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
        }
        tmp = ret^node;
        Cudd_Ref(tmp.getNode());
        Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());
        ret = tmp;
    }
    
    tmp = ~ret;
    ret = tmp;
    Cudd_Ref(tmp.getNode());
    Cudd_RecursiveDeref(bddMgr->getDdMgr(), ret.getNode());

    if(!bddMgr->addBddNode(options[0], ret)) {
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddXnorCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddSetVar <(size_t level)> <(string varName)> " << endl;
}

void
GVBddXnorCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddSetVar: " << "BDD set a variable name for a support." << endl;
}

//----------------------------------------------------------------------
// BREPort <(string bddName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddReportCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if(options.size() < 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if(options.size() > 1) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }
    BddNode ddNode = bddMgr->getBddNode(options[0]);
    if (ddNode() == 0) {
        ddNode = bddMgr->findNodeByName(options[0]);
        if (ddNode() != 0) bddMgr->report(ddNode);
    }
    else {
        bddMgr->report(ddNode);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVBddReportCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddREPort <(string bddName)>" << endl;
}

void
GVBddReportCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddREPort: " << "Report BDD information." << endl;
}

//----------------------------------------------------------------------
// BDRAW <(string bddName)> <(string fileName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVBddDrawCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if(options.size() < 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    } else if(options.size() > 2) {
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);
    }
    bddMgr->draw(options[0], options[1]);
    return GV_CMD_EXEC_DONE;
}

void
GVBddDrawCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: BddREPort <(string bddName)>" << endl;
}

void
GVBddDrawCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "BddREPort: " << "Report BDD information." << endl;
}

#endif
