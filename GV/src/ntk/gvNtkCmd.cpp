#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include <iostream>
#include <string>
#include <string.h>
#include "gvNtkCmd.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "util.h"
// #include "yosys.h"

// for "VErilog2 Aig"
#include "../../../V3/src/cmd/v3CmdMgr.h"

using namespace std;

bool initNtkCmd() {
    return (
            gvCmdMgr->regCmd("REad Design",         2, 1, new GVReadDesignCmd   )  &&
            gvCmdMgr->regCmd("PRint Info",          2, 1, new GVPrintInfoCmd    )  &&
            gvCmdMgr->regCmd("VErilog2 Aig",        2, 1, new GVVerilog2AigCmd  )
    );
}

//----------------------------------------------------------------------
// REad Design <(string fileName)> 
//----------------------------------------------------------------------

GVCmdExecStatus
GVReadDesignCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVReadDesignCmd" << endl;
}

void
GVReadDesignCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: REAd Rtl <(string fileName)> " << endl;
}

void
GVReadDesignCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "REad Design: " << "Read RTL (Verilog) Designs." << endl;
}

//----------------------------------------------------------------------
// PRint Info
//----------------------------------------------------------------------

GVCmdExecStatus
GVPrintInfoCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVPrintInfoCmd" << endl;
}

void
GVPrintInfoCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: PRint Info " << endl;
}

void
GVPrintInfoCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "PRint Info: " << "Print circuit information extracted by our parser." << endl;
}

//----------------------------------------------------------------------
// VErilog2 Aig -input <filename> -output <filename>
//----------------------------------------------------------------------
// Global Variable for V3CmdMgr
V3CmdMgr* v3CmdMgr = new V3CmdMgr("v3");

GVCmdExecStatus
GVVerilog2AigCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVVerilog2AigCmd" << endl;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    // options[0] = "-input"
    string label_in, label_out;
    label_in.assign(options[0]); label_out.assign(options[2]);
    if (options.size() < 4) { cerr << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl; }
    else if (options.size() > 4) { cerr << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl; }
    else if ((strcmp(label_in.c_str(), "-input") != 0) || (strcmp(label_out.c_str(), "-output") != 0))
    {
        cerr << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl;
    }
    const string tok_in = options[0];
    char* infile = const_cast <char *>(tok_in.c_str());
    const string tok_out = options[1];
    char* outfile = const_cast <char *>(tok_out.c_str());

    // Convert to V3 command
    char command[256];
    sprintf(command, "read rtl %s\nblast ntk\nwrite aig %s", infile, outfile);
    string v3cmd(command);

    // Start Program
    v3CmdMgr->GVsetPrompt(v3cmd);
    // bool status = v3CmdMgr->execOneCmd();
    return GV_EXEC_DONE;
}

void
GVVerilog2AigCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl;
    if (verbose) {
      Msg(MSG_IFO) << "Param: -input  : Specify the input Verilog filename with relative path." << endl;
      Msg(MSG_IFO) << "       -output : Specify the output AIG filename with relative path." << endl;
   }
}

void
GVVerilog2AigCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "VErilog2 Aig: " << "Convert verilog file into AIG. " << endl;
}



#endif