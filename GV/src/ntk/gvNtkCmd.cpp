#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"


// #include "yosys.h"

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
// VErilog2 Aig
//----------------------------------------------------------------------

GVCmdExecStatus
GVVerilog2AigCmd ::exec(const string& option) {
    Msg(MSG_IFO) << "I am GVVerilog2AigCmd" << endl;
}

void
GVVerilog2AigCmd ::usage(const bool& verbose) const {
    Msg(MSG_IFO) << "Usage: VErilog2 Aig " << endl;
}

void
GVVerilog2AigCmd ::help() const {
    Msg(MSG_IFO) << setw(20) << left << "VErilog2 Aig: " << "Convert verilog file into AIG. " << endl;
}



#endif