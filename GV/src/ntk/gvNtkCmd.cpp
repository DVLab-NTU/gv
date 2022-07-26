#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"


#include "v3Usage.h"
#include "v3CmdMgr.h"
#include "v3StrUtil.h"
#include "v3NtkHandler.h"

extern "C"
{
    void   Abc_Start();
    void   Abc_Stop();
    typedef struct Abc_Frame_t_ Abc_Frame_t;
    Abc_Frame_t * Abc_FrameGetGlobalFrame();
    int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );
}

Abc_Frame_t * pAbc = 0;


bool initNtkCmd() {
    return (
            gvCmdMgr->regCmd("REad Design",         2, 1, new GVReadDesignCmd   )  &&
            gvCmdMgr->regCmd("PRint Info",          2, 1, new GVPrintInfoCmd    )  &&
            gvCmdMgr->regCmd("VErilog2 Aig",        2, 1, new GVVerilog2AigCmd  )
    );
}

string V3Msg::_allName = "";
ofstream V3Msg::_allout;
V3MsgMgr Msg;
V3Usage v3Usage;
V3CmdMgr* v3CmdMgr = new V3CmdMgr("v3");
V3Handler v3Handler;


//----------------------------------------------------------------------
// REad Design <(string fileName)> 
//----------------------------------------------------------------------

GVCmdExecStatus
GVReadDesignCmd ::exec(const string& option) {
    V3CmdExecStatus status = CMD_EXEC_DONE;
    while (status != CMD_EXEC_QUIT) {
        v3CmdMgr->setPrompt();
        status = v3CmdMgr->execOneCmd();
        cout << endl;
    }
    gvMsg(GV_MSG_IFO) << "I am GVReadDesignCmd" << endl;
    return GV_CMD_EXEC_DONE;

}

void
GVReadDesignCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: REAd Rtl <(string fileName)> " << endl;
}

void
GVReadDesignCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "REad Design: " << "Read RTL (Verilog) Designs." << endl;
}

//----------------------------------------------------------------------
// PRint Info
//----------------------------------------------------------------------

GVCmdExecStatus
GVPrintInfoCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVPrintInfoCmd" << endl;
    Abc_Start();
	pAbc = Abc_FrameGetGlobalFrame();
    Abc_Stop();
    return GV_CMD_EXEC_DONE;
}

void
GVPrintInfoCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: PRint Info " << endl;
}

void
GVPrintInfoCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "PRint Info: " << "Print circuit information extracted by our parser." << endl;
}

//----------------------------------------------------------------------
// VErilog2 Aig
//----------------------------------------------------------------------

GVCmdExecStatus
GVVerilog2AigCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVVerilog2AigCmd" << endl;
    return GV_CMD_EXEC_DONE;
}

void
GVVerilog2AigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: VErilog2 Aig " << endl;
}

void
GVVerilog2AigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "VErilog2 Aig: " << "Convert verilog file into AIG. " << endl;
}



#endif