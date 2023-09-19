/****************************************************************************
  FileName     [ cirCmd.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define basic cir package commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <iostream>
#include <iomanip>
#include "cirMgr.h"
#include "cirGate.h"
#include "cirCmd.h"
#include "util.h"
#include "gvMsg.h"

using namespace std;

extern CirMgr* cirMgr;
extern int effLimit;

bool
GVinitCirCmd()
{
   return (gvCmdMgr->regCmd("CIRRead", 4, new CirReadCmd) &&
         gvCmdMgr->regCmd("CIRPrint", 4, new CirPrintCmd) &&
         gvCmdMgr->regCmd("CIRGate", 4, new CirGateCmd));
         // gvCmdMgr->regCmd("CIRSWeep", 5, new CirSweepCmd) &&
         // gvCmdMgr->regCmd("CIROPTimize", 6, new CirOptCmd) &&
         // gvCmdMgr->regCmd("CIRSTRash", 6, new CirStrashCmd) &&
         // gvCmdMgr->regCmd("CIRSIMulate", 6, new CirSimCmd) &&
         // gvCmdMgr->regCmd("CIRFraig", 4, new CirFraigCmd) &&
         // gvCmdMgr->regCmd("CIRWrite", 4, new CirWriteCmd) &&
         // gvCmdMgr->regCmd("CIRMiter", 4, new CirMiterCmd) &&
         // gvCmdMgr->regCmd("CIREFFort", 6, new CirEffortCmd));
}

enum CirCmdState
{
   // Order matters! Do not change the order!!
   CIRINIT,
   CIRREAD,
   CIROPT,
   CIRSTRASH,
   CIRSIMULATE,
   CIRFRAIG,
   // dummy end
   CIRCMDTOT
};

static CirCmdState curCmd = CIRINIT;

//----------------------------------------------------------------------
//    CIRRead <(string fileName)> [-Replace]
//----------------------------------------------------------------------
GVCmdExecStatus
CirReadCmd::exec(const string& option)
{
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
         if (doReplace) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA,options[i]);
         doReplace = true;
      }
      else if (myStrNCmp("-Aiger", options[i], 1) == 0) {
         fileType = AIGER;
      }
      else {
         if (fileName.size())
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
         fileName = options[i];
      }
   }

   if (cirMgr != 0) {
      if (doReplace) {
         cerr << "Note: original circuit is replaced..." << endl;
         curCmd = CIRINIT;
         delete cirMgr; cirMgr = 0;
      }
      else {
         cerr << "Error: circuit already exists!!" << endl;
         return GV_CMD_EXEC_ERROR;
      }
   }
   cirMgr = new CirMgr;
   cirMgr->readCirFromAbc(fileName, fileType);

   // curCmd = CIRREAD;

   return GV_CMD_EXEC_DONE;
}

void
CirReadCmd::usage(const bool& verbose) const
{
   gvMsg(GV_MSG_IFO) << "Usage: CIRRead <(string fileName)> [-Replace]" << endl;
}

void
CirReadCmd::help() const
{
   cout << setw(15) << left << "CIRRead: "
        << "read in a circuit and construct the netlist" << endl;
}

//----------------------------------------------------------------------
//    CIRPrint [-Summary | -Netlist | -PI | -PO | -FLoating | -FECpairs]
//----------------------------------------------------------------------
GVCmdExecStatus
CirPrintCmd::exec(const string& option)
{
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

void
CirPrintCmd::usage(const bool& verbose) const
{  
   gvMsg(GV_MSG_IFO) << "Usage: CIRPrint [-Summary | -Netlist | -PI | -PO | -FLoating "
      << "| -FECpairs]" << endl;
}

void
CirPrintCmd::help() const
{  
   cout << setw(15) << left << "CIRPrint: " << "print circuit\n";
}

//----------------------------------------------------------------------
//    CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>
//----------------------------------------------------------------------
GVCmdExecStatus
CirGateCmd::exec(const string& option)
{
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
         doFanin = true;
         checkLevel = true;
      }
      else if (myStrNCmp("-FANOut", options[i], 5) == 0) {
         if (doFanin || doFanout)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
         doFanout = true;
         checkLevel = true;
      }
      else if (!thisGate) {
         if (!myStr2Int(options[i], gateId) || gateId < 0)
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
         thisGate = cirMgr->getGate(gateId);
         if (!thisGate) {
            cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
         }
      }
      else if (thisGate)
         return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
      else
         return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
      if (checkLevel) {
         if (++i == n)
            return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i-1]);
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

void
CirGateCmd::usage(const bool& verbose) const
{
   gvMsg(GV_MSG_IFO) << "Usage: CIRGate <<(int gateId)> [<-FANIn | -FANOut><(int level)>]>"
      << endl;
}

void
CirGateCmd::help() const
{
   cout << setw(15) << left << "CIRGate: " << "report a gate\n";
}

// //----------------------------------------------------------------------
// //    CIRSWeep
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirSweepCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (!options.empty())
//       return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[0]);

//    assert(curCmd != CIRINIT);
//    cirMgr->sweep();

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirSweepCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRSWeep" << endl;
// }

// void
// CirSweepCmd::help() const
// {
//    cout << setw(15) << left << "CIRSWeep: "
//         << "remove unused gates\n";
// }

// //----------------------------------------------------------------------
// //    CIROPTimize
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirOptCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (!options.empty())
//       return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[0]);

//    assert(curCmd != CIRINIT);
//    if (curCmd == CIRSIMULATE) {
//       cerr << "Error: circuit has been simulated!! Do \"CIRFraig\" first!!"
//            << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    cirMgr->optimize();
//    curCmd = CIROPT;

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirOptCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIROPTimize" << endl;
// }

// void
// CirOptCmd::help() const
// {
//    cout << setw(15) << left << "CIROPTimize: "
//         << "perform trivial optimizations\n";
// }

// //----------------------------------------------------------------------
// //    CIRSTRash
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirStrashCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (!options.empty())
//       return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[0]);

//    assert(curCmd != CIRINIT);
//    if (curCmd == CIRSTRASH) {
//       cerr << "Error: circuit has been strashed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    else if (curCmd == CIRSIMULATE) {
//       cerr << "Error: circuit has been simulated!! Do \"CIRFraig\" first!!"
//            << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    cirMgr->strash();
//    curCmd = CIRSTRASH;

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirStrashCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRSTRash" << endl;
// }

// void
// CirStrashCmd::help() const
// {
//    cout << setw(15) << left << "CIRSTRash: "
//         << "perform structural hash on the circuit netlist\n";
// }

// //----------------------------------------------------------------------
// //    CIRSIMulate <-Random | -File <string patternFile>>
// //                [-Output (string logFile)]
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirSimCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    ifstream patternFile;
//    ofstream logFile;
//    bool doRandom = false, doFile = false, doLog = false;
//    for (size_t i = 0, n = options.size(); i < n; ++i) {
//       if (myStrNCmp("-Random", options[i], 2) == 0) {
//          if (doRandom || doFile)
//             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//          doRandom = true;
//       }
//       else if (myStrNCmp("-File", options[i], 2) == 0) {
//          if (doRandom || doFile)
//             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//          if (++i == n)
//             return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i-1]);
//          patternFile.open(options[i].c_str(), ios::in);
//          if (!patternFile)
//             return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
//          doFile = true;
//       }
//       else if (myStrNCmp("-Output", options[i], 2) == 0) {
//          if (doLog)
//             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//          if (++i == n)
//             return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i-1]);
//          logFile.open(options[i].c_str(), ios::out);
//          if (!logFile)
//             return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[i]);
//          doLog = true;
//       }
//       else
//          return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//    }

//    if (!doRandom && !doFile)
//       return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");

//    assert (curCmd != CIRINIT);
//    if (doLog)
//       cirMgr->setSimLog(&logFile);
//    else cirMgr->setSimLog(0);

//    if (doRandom)
//       cirMgr->randomSim();
//    else
//       cirMgr->fileSim(patternFile);
//    cirMgr->setSimLog(0);
//    curCmd = CIRSIMULATE;
   
//    return GV_CMD_EXEC_DONE;
// }

// void
// CirSimCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRSIMulate <-Random | -File <string patternFile>>\n"
//       << "                   [-Output (string logFile)]" << endl;
// }

// void
// CirSimCmd::help() const
// {
//    cout << setw(15) << left << "CIRSIMulate: "
//         << "perform Boolean logic simulation on the circuit\n";
// }

// //----------------------------------------------------------------------
// //    CIRFraig
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirFraigCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (!options.empty())
//       return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[0]);

//    if (curCmd != CIRSIMULATE) {
//       cerr << "Error: circuit is not yet simulated!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    cirMgr->fraig();
//    curCmd = CIRFRAIG;

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirFraigCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRFraig" << endl;
// }

// void
// CirFraigCmd::help() const
// {
//    cout << setw(15) << left << "CIRFraig: "
//         << "perform Boolean logic simulation on the circuit\n";
// }

// //----------------------------------------------------------------------
// //    CIRWrite [(int gateId)][-Output (string aagFile)]
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirWriteCmd::exec(const string& option)
// {
//    if (!cirMgr) {
//       cerr << "Error: circuit is not yet constructed!!" << endl;
//       return GV_CMD_EXEC_ERROR;
//    }
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (options.empty()) {
//       cirMgr->writeAag(cout);
//       return GV_CMD_EXEC_DONE;
//    }
//    bool hasFile = false;
//    int gateId;
//    CirGate *thisGate = NULL;
//    ofstream outfile;
//    for (size_t i = 0, n = options.size(); i < n; ++i) {
//       if (myStrNCmp("-Output", options[i], 2) == 0) {
//          if (hasFile) 
//             return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
//          if (++i == n)
//             return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, options[i-1]);
//          outfile.open(options[i].c_str(), ios::out);
//          if (!outfile)
//             return GVCmdExec::errorOption(GV_CMD_OPT_FOPEN_FAIL, options[1]);
//          hasFile = true;
//       }
//       else if (myStr2Int(options[i], gateId) && gateId >= 0) {
//          if (thisGate != NULL)
//             return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[i]);
//          thisGate = cirMgr->getGate(gateId);
//          if (!thisGate) {
//             cerr << "Error: Gate(" << gateId << ") not found!!" << endl;
//             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//          }
//          if (!thisGate->isAig()) {
//              cerr << "Error: Gate(" << gateId << ") is NOT an AIG!!" << endl;
//             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//          }
//       }
//       else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[i]);
//    }

//    if (!thisGate) {
//       assert (hasFile);
//       cirMgr->writeAag(outfile);
//    }
//    else if (hasFile) cirMgr->writeGate(outfile, thisGate);
//    else cirMgr->writeGate(cout, thisGate);

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirWriteCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRWrite [(int gateId)][-Output (string aagFile)]" << endl;
// }

// void
// CirWriteCmd::help() const
// {
//    cout << setw(15) << left << "CIRWrite: "
//         << "write the netlist to an ASCII AIG file (.aag)\n";
// }

// //----------------------------------------------------------------------
// //    CIRMiter <(string inFile1)> <(string inFile2)>
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirMiterCmd::exec(const string& option)
// {
//    // check option
//    vector<string> options;
//    GVCmdExec::lexOptions(option, options);

//    if (cirMgr != 0) {
//       cerr << "Note: original circuit is replaced..." << endl;
//       curCmd = CIRINIT;
//       delete cirMgr; cirMgr = 0;
//    }
//    CirMgr *cm[2];
//    for (int i = 0; i < 2; ++i) {
//       cm[i] = new CirMgr;
//       cirMgr = cm[i];
//       if (!cirMgr->readCircuit(options[i])) {
//          curCmd = CIRINIT;
//          for (int j = 0; j <= i; ++j) delete cm[j];
//          cirMgr = 0;
//          return GV_CMD_EXEC_ERROR;
//       }
//    }

//    if (cm[0]->getNumPIs() != cm[1]->getNumPIs()) {
//       cerr << "Error: numbers of PIs are different!!" << endl;
//       delete cm[0]; delete cm[1]; cirMgr = 0;
//       return GV_CMD_EXEC_ERROR;
//    } 
//    if (cm[0]->getNumPOs() != cm[1]->getNumPOs()) {
//       cerr << "Error: numbers of POs are different!!" << endl;
//       delete cm[0]; delete cm[1]; cirMgr = 0;
//       return GV_CMD_EXEC_ERROR;
//    } 

//    cirMgr = new CirMgr;
//    if (!cirMgr->createMiter(cm[0], cm[1])) {
//       delete cirMgr;
//       cirMgr = 0;
//       return GV_CMD_EXEC_ERROR;
//    }
//    delete cm[0]; delete cm[1];

//    curCmd = CIRREAD;

//    return GV_CMD_EXEC_DONE;
// }

// void
// CirMiterCmd::usage(const bool& verbose) const
// {
//    gvMsg(GV_MSG_IFO) << "Usage: CIRMiter <(string inFile1)> <(string inFile2)> " << endl;
// }

// void
// CirMiterCmd::help() const
// {
//    cout << setw(15) << left << "CIRMiter: "
//         << "create a miter circuit.\n";
// }

// //----------------------------------------------------------------------
// //    CIREFFort <(int effortLimit)>
// //----------------------------------------------------------------------
// GVCmdExecStatus
// CirEffortCmd::exec(const string& option)
// {
//    // check option
//    string token;
//    if (!GVCmdExec::lexSingleOption(option, token))
//       return GV_CMD_EXEC_ERROR;

//    int eff = -1;
//    if (token.empty())
//       return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
//    if (!myStr2Int(token, eff) || eff < 0)
//       return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);

//    effLimit = eff;
//    return GV_CMD_EXEC_DONE;
// }

// void
// CirEffortCmd::usage(const bool& verbose) const
// {  
//    gvMsg(GV_MSG_IFO) << "Usage: CIREFFort <(int effortLimit)>" << endl;
// }

// void
// CirEffortCmd::help() const
// {  
//    cout << setw(15) << left << "CIREFFort: " << "set SAT proof effort\n";
// }
