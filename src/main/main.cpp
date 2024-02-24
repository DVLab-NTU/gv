/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define main() ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_MAIN_CC
#define GV_MAIN_CC

#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "gvUsage.h"
#include "kernel/yosys.h"
#include "util.h"
using namespace std;

string GVMsg::_allName = "";
ofstream GVMsg::_allout;
GVMsgMgr gvMsg;

GVUsage gvUsage;

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
GVCmdMgr* gvCmdMgr = new GVCmdMgr("gv");

extern bool initCommonCmd();
extern bool initSimCmd();
extern bool initVrfCmd();
extern bool initAbcCmd();
extern bool initModCmd();
extern bool initBddCmd();
extern bool initProveCmd();
extern bool initProveCmd();
extern bool initItpCmd();
extern bool initCirCmd();
extern bool initYosysCmd();

static void
usage() {
    cout << "Usage: ./gv [ -File < doFile > ]" << endl;
}

static void
myexit() {
    usage();
    exit(-1);
}

int main(int argc, char** argv) {
    myUsage.reset();

    ifstream dof;

    if (argc == 3) {  // -file <doFile>
        if (myStrNCmp("-File", argv[1], 2) == 0) {
            if (!gvCmdMgr->openDofile(argv[2])) {
                cout << "Error: cannot open file \"" << argv[2] << "\"!!\n";
                myexit();
            }
        } else {
            cout << "Error: unknown argument \"" << argv[1] << "\"!!\n";
            myexit();
        }
    } else if (argc != 1) {
        cout << "Error: illegal number of argument (" << argc << ")!!\n";
        myexit();
    }

    // cout << "GV v0.3.0 - Copyright © 2023-present, DVLab NTUEE.\n";
    if (!(initCommonCmd() && initSimCmd() && initVrfCmd() && initAbcCmd() &&
          initModCmd() && initBddCmd() && initProveCmd() && initItpCmd() && initCirCmd() && initYosysCmd()))
        return 1;

    GVCmdExecStatus status = GV_CMD_EXEC_DONE;
    while (status != GV_CMD_EXEC_QUIT) {
        gvCmdMgr->setPrompt();
        status = gvCmdMgr->execOneCmd();
        cout << endl;
    }
    return 0;
}

#endif
