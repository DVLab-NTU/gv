#ifndef GV_ABC_CMD_C
#define GV_ABC_CMD_C

#include "gvMsg.h"
#include "gvAbcCmd.h"
#include "gvAbcMgr.h"
#include "util.h"
#include <string.h>
#include <vector>


bool GVinitAbcCmd() {
    if (abcMgr) delete abcMgr;
    abcMgr = new AbcMgr;
    return (
         gvCmdMgr->regCmd("ABCRead",           4, new GVABCReadCmd   ) ,
         gvCmdMgr->regCmd("ABCPrint",          4, new GVABCPrintCmd   ) ,
         gvCmdMgr->regCmd("ABCNtk2Aig",        4, new GVABCNtk2AigCmd   ) ,
         gvCmdMgr->regCmd("ABCSweep",          4, new GVABCSweepCmd   ) ,
         gvCmdMgr->regCmd("AIGPrint",          4, new GVAIGPrintCmd   ) ,
         gvCmdMgr->regCmd("AIGFraig",          4, new GVAIGFraigCmd   ) ,
         gvCmdMgr->regCmd("AIGRAndomSim",      5, new GVAIGRAndomSimCmd   ),
         gvCmdMgr->regCmd("ABCCMD",            6, new GVABCOriginalCmd ) ,
         gvCmdMgr->regCmd("CUT ENUmerate" , 3, 3, new GVCutEnumerate ) ,
         gvCmdMgr->regCmd("SIMilarity COMpute" , 3, 3, new GVSimilarityCompute )
    );
}

//----------------------------------------------------------------------
// ABCRead <(string filename)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCReadCmd ::exec(const string& option) {
    vector<string> options;
    string fileName = ""; 
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
      const string& token = options[i];
      if (fileName == "") fileName = options[i];
      else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    }
    if (fileName == "") return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string fileName)>");
    abcMgr -> abcReadDesign(fileName);
    return GV_CMD_EXEC_DONE;
}

void
GVABCReadCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCRead " << endl;
}

void
GVABCReadCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCRead: " << "Read netlist by ABC." << endl;
}

//----------------------------------------------------------------------
// ABCPrint [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCPrintCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_abcNtkMgr()) -> printSummary();
    return GV_CMD_EXEC_DONE;
}

void
GVABCPrintCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCPrint " << endl;
}

void
GVABCPrintCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCPrint: " << "Print netlist information." << endl;
}

//----------------------------------------------------------------------
// ABCNtk2Aig
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCNtk2AigCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();
    abcMgr -> abcNtk2Aig();
    return GV_CMD_EXEC_DONE;
}

void
GVABCNtk2AigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCNtk2Aig " << endl;
}

void
GVABCNtk2AigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCNtk2Aig: " << "Transform netlist into AIG." << endl;
}

//----------------------------------------------------------------------
// ABCSweep [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCSweepCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_abcNtkMgr()) -> sweep(verbose);
    return GV_CMD_EXEC_DONE;
}

void
GVABCSweepCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: GVABCSweep " << endl;
}

void
GVABCSweepCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "GVABCSweep: " << "Do sweep operation on current netlist." << endl;
}

//----------------------------------------------------------------------
// AIGPrint [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGPrintCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> printSummary(verbose);
    return GV_CMD_EXEC_DONE;
}

void
GVAIGPrintCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGPrint " << endl;
}

void
GVAIGPrintCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGPrint: " << "Print AIG information." << endl;
}

//----------------------------------------------------------------------
// AIGFraig 
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGFraigCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> fraig();
    return GV_CMD_EXEC_DONE;
}

void
GVAIGFraigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGFraig " << endl;
}

void
GVAIGFraigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGFraig: " << "Do FRAIG on current netlist." << endl;
}

//----------------------------------------------------------------------
// AIGRAndomSim 
//----------------------------------------------------------------------

GVCmdExecStatus
GVAIGRAndomSimCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    bool basic = false, verbose = false;
    size_t n = options.size();

    if (options.size() > 1) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[1]);
    else if(options.size()) {
        if (myStrNCmp("-Verbose", options[0], 2) == 0) verbose = true;
        else return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);
    }
    (abcMgr -> get_aigNtkMgr()) -> randomSim(verbose);
    return GV_CMD_EXEC_DONE;
}

void
GVAIGRAndomSimCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: AIGRAndomSim [-Verbose]" << endl;
}

void
GVAIGRAndomSimCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "AIGRAndomSim: " << "Random Simulation." << endl;
}
//----------------------------------------------------------------------
// ABCCMD <command in ABC>
//----------------------------------------------------------------------

GVCmdExecStatus
GVABCOriginalCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    string command;
    for (size_t i = 0; i < n; ++i) {
      command += options[i];
      if (i < n-1) { command += " "; }
    }

    // calling abc's command
    char Command[1024], abcCmd[128];
    strcpy(abcCmd, command.c_str());
    sprintf( Command, "%s", abcCmd ); Cmd_CommandExecute( abcMgr->get_Abc_Frame_t(), Command );
}

void
GVABCOriginalCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: ABCCMD <command in ABC> " << endl;
}

void
GVABCOriginalCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "ABCCMD: " << "Directly call ABC's command." << endl;
}

//----------------------------------------------------------------------
// Cut Sat
//----------------------------------------------------------------------
GVCmdExecStatus
GVCutEnumerate::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVCutEnumerate " << endl;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    int cut_size = 6, n_cuts_max = 16;
    bool verbose = false;
    for (size_t i = 0; i < n; ++i) {
      const string& token = options[i];
        if (myStrNCmp("-N", token, 1) == 0) {
            ++i;
            n_cuts_max = stoi(options[i]);
            
            continue;
        }
        if (myStrNCmp("-M", token, 1) == 0) {
            ++i;
            cut_size = stoi(options[i]);
            continue;
        }
        if (myStrNCmp("-v", token, 1) == 0) {
            verbose = true;
            continue;
        }
    }
    if(n_cuts_max <= 0)
    {
        cout << "the number of cuts at a node nust be a positive number!" << endl;
        return GV_CMD_EXEC_ERROR;
    }
    if(cut_size <= 2 or cut_size > 16)
    {
        cout << "the size of the cut must between 2 and 16!" << endl;
        return GV_CMD_EXEC_ERROR;
    }

    (abcMgr -> get_aigNtkMgr()) -> Aig_EnumerateCuts( abcMgr->get_aigNtkMgr()->getAig(), n_cuts_max, cut_size, 0, verbose);
    

    return GV_CMD_EXEC_DONE;
}

void
GVCutEnumerate::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: CUT ENUmerate [-NM num] [-v]" << endl;
    gvMsg(GV_MSG_IFO) << "  -N num : the maximum number of cuts at a node [default = 16]" << endl;
    gvMsg(GV_MSG_IFO) << "  -M num : the maximum size of each cut [default = 6]" << endl;
    gvMsg(GV_MSG_IFO) << "  -v : verbose print cut size for each cut" << endl;
}

void
GVCutEnumerate::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "CUT ENUmerate: " << "Conduct Cut Enumeration." << endl;
}

GVCmdExecStatus
GVSimilarityCompute::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVSimilarityCompute " << endl;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    char* golden_file_name;
    char* revised_file_name;
    bool verbose = false,golden_file_set = false, revised_file_set=false;
    for (size_t i = 0; i < n; ++i) {
      const string& token = options[i];
        if (myStrNCmp("-v", token, 1) == 0) {
            verbose = true;
            continue;
        }
        if (myStrNCmp("-golden", token, 4) == 0) {
            ++i;
            golden_file_name = new char[options[i].length() + 1];
            strcpy(golden_file_name, options[i].c_str());
            golden_file_set = true;
            cout << "golden set" << endl;
            continue;
        }
        if (myStrNCmp("-revised", token, 4) == 0) {
            ++i;
            cout << "reised set" << endl;
            revised_file_name = new char[options[i].length() + 1];
            strcpy(revised_file_name, options[i].c_str());
            revised_file_set = true;
            continue;
        }
    }
    // cout << "1" << endl;
    if(!golden_file_set ) 
    {
        golden_file_name = new char[gvModMgr->getInputFileName().length() + 1];
        strcpy(golden_file_name, gvModMgr->getInputFileName().c_str());
        golden_file_set = true;
    }
    // cout << "2" << endl;
    if(!revised_file_set || !revised_file_set)
    {
        gvMsg(GV_MSG_ERR) << "Either golden design or revised design is not given" << endl;
        return GV_CMD_EXEC_ERROR;
    }

    (abcMgr -> get_aigNtkMgr()) -> simlirarity(revised_file_name);
    

    return GV_CMD_EXEC_DONE;
}

void
GVSimilarityCompute::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: CUT ENUmerate <-golden file> [-revised file] [-v]" << endl;
    gvMsg(GV_MSG_IFO) << "  -N num : the maximum number of cuts at a node [default = 16]" << endl;
    gvMsg(GV_MSG_IFO) << "  -M num : the maximum size of each cut [default = 6]" << endl;
    gvMsg(GV_MSG_IFO) << "  -v : verbose print cut size for each cut" << endl;
}

void
GVSimilarityCompute::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SIMilarity COMpute: " << "Compute the similarity between cuts from two deigns." << endl;
}

#endif