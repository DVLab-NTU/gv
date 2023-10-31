/****************************************************************************
  FileName     [ satCmd.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "gvSatCmd.h"
#include "gvMsg.h"
#include "gvSatMgr.h"
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <cstring>
#include <iomanip>
using namespace std;

static SATMgr* satMgr = new SATMgr();

bool
GVinitItpCmd() {
    return (gvCmdMgr->regCmd("SATVerify ITP", 4, 3, new SATVerifyItpCmd) &&
            gvCmdMgr->regCmd("SATVerify BMC", 4, 3, new SATVerifyBmcCmd));
}

//----------------------------------------------------------------------
//    SATVerify ITP < -Netid <netId> | -Output <outputIndex> > >
//----------------------------------------------------------------------
GVCmdExecStatus
SATVerifyItpCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-Netid", options[0], 2) == 0) isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0) isNet = false;
    else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int     num = 0;
    GVNetId netId;
    CirGate* gate;
    if (!myStr2Int(options[1], num) || (num < 0)) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= gvNtkMgr->getNetSize()) {
            gvMsg(GV_MSG_ERR) << "Net with Id " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = GVNetId::makeNetId(num);
    } else {
        if ((unsigned)num >= cirMgr->getNumPOs()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        // netId = gvNtkMgr->getOutput(num);
        gate = cirMgr->getPo(num);
    }
    // get PO's input, since the PO is actually a redundant node and should be removed
    // GVNetId redundantNode = gvNtkMgr->getGVNetId(netId.id);
    // GVNetId monitor       = gvNtkMgr->getInputNetId(redundantNode, 0);
    // satMgr->verifyPropertyItp(gvNtkMgr->getNetNameFromId(redundantNode.id), monitor);

    CirGate* monitor = new CirAigGate(cirMgr->getNumTots(), 0); cirMgr->addTotGate(monitor);
    monitor->setIn0(gate->getIn0Gate(), gate->getIn0().isInv());
    monitor->setIn1(cirMgr->_const0, true);
    satMgr->verifyPropertyItp("monitor", monitor);

    // Ref
    // satMgr->verifyPropertyItp("monitor", gate->getIn0Gate());

    return GV_CMD_EXEC_DONE;
}

void
SATVerifyItpCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SATVerify ITP < -Netid <netId> | -Output <outputIndex> >" << endl;
}

void
SATVerifyItpCmd::help() const {
    cout << setw(20) << left << "SATVerify ITP:"
         << "check the monitor by interpolation-based technique" << endl;
}

// //----------------------------------------------------------------------
// //    SATVerify BMC < -Netid <netId> | -Output <outputIndex> > >
// //----------------------------------------------------------------------
GVCmdExecStatus
SATVerifyBmcCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-Netid", options[0], 2) == 0) isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0) isNet = false;
    else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int     num = 0;
    GVNetId netId;
    CirGate* gate;
    if (!myStr2Int(options[1], num) || (num < 0)) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= gvNtkMgr->getNetSize()) {
            gvMsg(GV_MSG_ERR) << "Net with Id " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = GVNetId::makeNetId(num);
    } else {
        if ((unsigned)num >= gvNtkMgr->getOutputSize()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        netId = gvNtkMgr->getOutput(num);
        gate = cirMgr->getPo(num);
    }
    // get PO's input, since the PO is actually a redundant node and should be removed
    GVNetId redundantNode = gvNtkMgr->getGVNetId(netId.id);
    GVNetId monitor       = gvNtkMgr->getInputNetId(redundantNode, 0);
    // satMgr->verifyPropertyBmc(gvNtkMgr->getNetNameFromId(redundantNode.id), monitor);
    satMgr->verifyPropertyBmc("monitor", gate->getIn0Gate());

    return GV_CMD_EXEC_DONE;
}

void
SATVerifyBmcCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SATVerify BMC < -Netid <netId> | -Output < outputIndex >> " << endl;
}

void
SATVerifyBmcCmd::help() const {
    cout << setw(20) << left << "SATVerify BMC:"
         << "check the monitor by bounded model checking" << endl;
}
