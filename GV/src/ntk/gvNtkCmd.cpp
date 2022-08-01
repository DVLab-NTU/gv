#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"


// #include "v3Usage.h"
// #include "v3CmdMgr.h"
// #include "v3StrUtil.h"
// #include "v3NtkHandler.h"

// Abc_Frame_t * pAbc = 0;


bool initNtkCmd() {
    return (
            gvCmdMgr->regCmd("REad Design",         2, 1, new GVReadDesignCmd   )  &&
            gvCmdMgr->regCmd("PRint Info",          2, 1, new GVPrintInfoCmd    )  &&
            gvCmdMgr->regCmd("VErilog2 Aig",        2, 1, new GVVerilog2AigCmd  )
    );
}

// string V3Msg::_allName = "";
// ofstream V3Msg::_allout;
// V3MsgMgr Msg;
// V3Usage v3Usage;
// V3CmdMgr* v3CmdMgr = new V3CmdMgr("v3");
// V3Handler v3Handler;


//----------------------------------------------------------------------
// REad Design <(string fileName)> 
//----------------------------------------------------------------------

GVCmdExecStatus
GVReadDesignCmd ::exec(const string& option) {
    // V3CmdExecStatus status = CMD_EXEC_DONE;
    // while (status != CMD_EXEC_QUIT) {
    //     v3CmdMgr->setPrompt();
    //     status = v3CmdMgr->execOneCmd();
    //     cout << endl;
    // }
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
    
    // initV3();

    // get file name
    // vector<string> options;
    // GVCmdExec::lexOptions(option, options);

	// size_t n = options.size();
    // char inname[128], outname[128];
    // bool hasinfile = false, hasoutfile = false;

    // cout << "#################" << endl;
    // cout << "# input command #" << endl;
    // cout << "#################" << endl;
    // for (size_t i = 0; i < n; ++i) {
    //     const string& token = options[i];
    //     cout << "option[" << i << "]: " << options[i] << endl;
    //     if (myStrNCmp("-input", token, 6) == 0) {
    //         // if no specify input file
    //         if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
    //         // if input file not an AIG 
    //         else if (strncmp(options[i+1].substr(options[i+1].length()-2, options[i+1].length()).c_str(), ".v", 2)) 
    //         {
    //             cerr << "ERROR: Please input an \"Verilog\" file (<filename>.v) !" << endl;
    //             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    //         }
    //         else { strcpy(inname, options[i+1].c_str()); hasinfile = true; }
    //     }
    //     else if (myStrNCmp("-output", token, 7) == 0) {
    //         // if no specify input file
    //         if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
    //         // if input file not an AIG 
    //         else if (strncmp(options[i+1].substr(options[i+1].length()-4, options[i+1].length()).c_str(), ".aig", 4)) 
    //         {
    //             cerr << "ERROR: Please input an \"AIG\" file (<filename>.aig) !" << endl;
    //             return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
    //         }
    //         else { strcpy(outname, options[i+1].c_str()); hasoutfile = true; }
    //     }
    // }
    // // need both input and output file
    // if ((!hasinfile) || (!hasoutfile))
    // {
    //     cerr << "ERROR: Both input and output file are required !" << endl;
    //     const string& missing = ((!hasinfile) && (!hasoutfile)) ? "Input / Output file" : ((!hasinfile)) ? "Input file" : (!hasoutfile) ? "Output file" : "";
    //     return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, missing);
    // }

    // // refer to v3CmdMgr.cpp & main.cpp, execute the virtual layer of GVCmdExec*
    // char execCmd[128];
    // string cmd1 = "", cmd2 = "", cmd3 = "";

    // sprintf(execCmd, "read rtl %s", inname);
    // string command = string(execCmd);
    // e = parseV3Cmd(command, cmd1);
    // e->exec(cmd1);

    // sprintf(execCmd, "blast ntk");
    // command = string(execCmd);
    // e = parseV3Cmd(command, cmd2);
    // e->exec(cmd2);

    // sprintf(execCmd, "write aig %s", outname);
    // command = string(execCmd);
    // e = parseV3Cmd(command, cmd3);
    // e->exec(cmd3);

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