/****************************************************************************
  FileName     [ main.cpp ]
  PackageName  [ main ]
  Synopsis     [ Define main() ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_MAIN_CC
#define GV_MAIN_CC

#include "util.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "gvUsage.h"

using namespace std;

string GVMsg::_allName = "";
ofstream GVMsg::_allout;
GVMsgMgr Msg;

GVUsage gvUsage;

//----------------------------------------------------------------------
//    Global cmd Manager
//----------------------------------------------------------------------
GVCmdMgr* gvCmdMgr = new GVCmdMgr("gv");

extern bool initCommonCmd();
extern bool initNtkCmd();
extern bool initSimCmd();
extern bool initVrfCmd();


static void
usage()
{
   cout << "Usage: cirTest [ -File < doFile > ]" << endl;
}

static void
myexit()
{
   usage();
   exit(-1);
}

int
main(int argc, char** argv)
{
   myUsage.reset();

   ifstream dof;

   if (argc == 3) {  // -file <doFile>
      if (myStrNCmp("-File", argv[1], 2) == 0) {
         if (!gvCmdMgr->openDofile(argv[2])) {
            cerr << "Error: cannot open file \"" << argv[2] << "\"!!\n";
            myexit();
         }
      }
      else {
         cerr << "Error: unknown argument \"" << argv[1] << "\"!!\n";
         myexit();
      }
   }
   else if (argc != 1) {
      cerr << "Error: illegal number of argument (" << argc << ")!!\n";
      myexit();
   }

   if (!(initCommonCmd()&&initNtkCmd()&&initSimCmd()&&initVrfCmd()))
      return 1;

   GVCmdExecStatus status = CMD_EXEC_DONE;
   while (status != CMD_EXEC_QUIT) {
      gvCmdMgr->setPrompt();
      status = gvCmdMgr->execOneCmd();
      cout << endl;
   }
   return 0;
}

#endif
