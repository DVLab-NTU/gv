#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include <iostream>
#include <string>
#include "gvNtkCmd.h"
#include "gvCmdMgr.h"
#include "gvMsg.h"
#include "util.h"
// #include "yosys.h"

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
GVCmdExecStatus
GVVerilog2AigCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVVerilog2AigCmd" << endl;
    // Parse GV command
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    if (options.size() < 2) { cerr << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl; }
    else if (options.size() > 2) { cerr << "Usage: VErilog2 Aig -input <filename> -output <filename>" << endl; }
    string infile, outfile;
    const string tok_in = options[0];
    const string tok_out = options[1];
    infile.assign(tok_in); outfile.assign(tok_out);

    // Start Program: Convert to V3 command
    string _r = "read rtl " + infile; 
    string _b = "blast ntk";
    string _w = "write aig " + outfile; 
    string _dir = "cd ../../../V3/; ./v3";
    string _q = "quit";
    string _y = "Y";
    const char* r = _r.c_str();
    const char* b = _b.c_str();
    const char* w = _w.c_str();
    const char* dir = _dir.c_str();
    const char* q = _q.c_str();
    const char* y = _y.c_str();
    system(dir);
    system(r);
    system(b);
    system(w);
    system(q); system(y);

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