/****************************************************************************
  FileName     [ satCmd.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/

#include "gvSatCmd.h"

#include <cstring>
#include <iomanip>

#include "cirGate.h"
#include "cirMgr.h"
#include "gvMsg.h"
#include "gvSatMgr.h"
#include "util.h"
using namespace std;

static SATMgr* satMgr = new SATMgr();

bool initItpCmd() {
    return (gvCmdMgr->regCmd("SATVerify ITP", 4, 3, new SATVerifyItpCmd) &&
            gvCmdMgr->regCmd("SATVerify BMC", 4, 3, new SATVerifyBmcCmd));
}

//----------------------------------------------------------------------
//    SATVerify ITP < -GateId <gateId> | -Output <outputIndex> > >
//----------------------------------------------------------------------
GVCmdExecStatus
SATVerifyItpCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-GateId", options[0], 2) == 0)
        isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0)
        isNet = false;
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int num = 0;
    CirGate* gate;
    string monitorName = "";
    if (!myStr2Int(options[1], num) || (num < 0)) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= cirMgr->getNumTots()) {
            gvMsg(GV_MSG_ERR) << "Net with Id " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        gate = cirMgr->getGate(num);
    } else {
        if ((unsigned)num >= cirMgr->getNumPOs()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num << " does NOT Exist in Current Ntk !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        gate        = cirMgr->getPo(num);
        monitorName = gate->getName();
    }
    // get PO's input, since the PO is actually a redundant node and should be removed
    CirGate* monitor = new CirAigGate(cirMgr->getNumTots(), 0);
    cirMgr->addTotGate(monitor);
    monitor->setIn0(gate->getIn0Gate(), gate->getIn0().isInv());
    monitor->setIn1(cirMgr->_const1, false);
    satMgr->verifyPropertyItp(monitorName, monitor);

    return GV_CMD_EXEC_DONE;
}

void SATVerifyItpCmd::usage(const bool& verbose) const {
    cout << "Usage: SATVerify ITP < -GateId <gateId> | -Output <outputIndex> >" << endl;
}

void SATVerifyItpCmd::help() const {
    cout << setw(20) << left << "SATVerify ITP:"
         << "check the monitor by interpolation-based technique" << endl;
}

// //----------------------------------------------------------------------
// //    SATVerify BMC < -GateId <gateId> | -Output <outputIndex> > >
// //----------------------------------------------------------------------
GVCmdExecStatus
SATVerifyBmcCmd::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    bool isNet = false;

    if (myStrNCmp("-GateId", options[0], 2) == 0)
        isNet = true;
    else if (myStrNCmp("-Output", options[0], 2) == 0)
        isNet = false;
    else
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    int num = 0;
    CirGate* gate;
    string monitorName = "";
    if (!myStr2Int(options[1], num) || (num < 0)) return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    if (isNet) {
        if ((unsigned)num >= cirMgr->getNumTots()) {
            gvMsg(GV_MSG_ERR) << "Gate with Id " << num << " does NOT Exist in Current Cir !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        gate = cirMgr->getGate(num);
    } else {
        if ((unsigned)num >= cirMgr->getNumPOs()) {
            gvMsg(GV_MSG_ERR) << "Output with Index " << num << " does NOT Exist in Current Cir !!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
        }
        monitorName = cirMgr->getPo(num)->getName();
        gate        = cirMgr->getPo(num)->getIn0Gate();
    }
    // get PO's input, since the PO is actually a redundant node and should be removed
    satMgr->verifyPropertyBmc(monitorName, gate);

    return GV_CMD_EXEC_DONE;
}

void SATVerifyBmcCmd::usage(const bool& verbose) const {
    cout << "Usage: SATVerify BMC < -GateId <gateId> | -Output < outputIndex >> " << endl;
}

void SATVerifyBmcCmd::help() const {
    cout << setw(20) << left << "SATVerify BMC:"
         << "check the monitor by bounded model checking" << endl;
}
