#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"

#include "kernel/yosys.h"
#include "gvModMgr.h"
#include "gvAbcMgr.h"
USING_YOSYS_NAMESPACE

bool initNtkCmd() {
    return (
            gvCmdMgr->regCmd("SEt Engine",         2, 1, new GVSetEngineCmd   )  &&
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
// SEt Engine <(string engineName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVSetEngineCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVSetEngineCmd" << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    // Missing Option
    if(n == 0){
        cerr << "ERROR: Please input an \"Engine Name\"<(string engineName)> !" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string engineName)>");
    }
    // Extra option(s)
    if(n > 1){
        string extraOption = "";
        for(int i = 1; i < n ; ++i){extraOption += options[i];}
        return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, extraOption);
    }
    vector<string> engNameList = {"yosys","abc","v3"};
    string& token =options[0];
    size_t engPos = 0;
    for (;engPos < engNameList.size(); ++engPos){
        // convert the engine name to lower case
        for_each(token.begin(), token.end(), [](char & c){c = ::tolower(c);});
        //cout << "token: " << token << endl;
        if(token == engNameList[engPos]){
            break;
        }
        else if (token.size() < engNameList[engPos].size()){
            if(token == engNameList[engPos].substr(0,token.size()))
                break;
        }
        
    }
    switch(engPos){
        case GV_MOD_ENGINE_YOSYS:{
            gvModMgr->setGVEngine(GV_MOD_ENGINE_YOSYS);
            break;
        }
        case GV_MOD_ENGINE_ABC:{
            gvModMgr->setGVEngine(GV_MOD_ENGINE_ABC);
            break;
        }
        case GV_MOD_ENGINE_V3:{
            gvModMgr->setGVEngine(GV_MOD_ENGINE_V3);
            break;
        }
        default:{
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
        }
    };
    cout << "Set Engine \""<< engNameList[engPos] <<"\" Success !!" <<endl;
    return GV_CMD_EXEC_DONE;
}

void
GVSetEngineCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt Engine <(string engineName)> " << endl;
    gvMsg(GV_MSG_IFO) << "Param: <(string engineName)>  :  Name of the engine. <(yosys) | (abc) | (v3)>" << endl;
}

void
GVSetEngineCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "SEt Engine: " << "Set the specific engine to parse the design." << endl;
}

//----------------------------------------------------------------------
// REad Design <-Verilog | -Blif  | -Aig> <(string fileName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVReadDesignCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVReadDesignCmd" << endl;

    // check option 
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    bool fileVerilog = false, fileBlif = false, fileAig = false;
    size_t n = options.size();
    string filename = "";

    if (n == 0)
        fileVerilog = true;
    else {
        for (size_t i = 0; i < n; ++i) {
            const string& token = options[i];
            if (myStrNCmp("-Verilog", token, 2) == 0) {
                if (fileVerilog | fileBlif | fileAig)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileVerilog = true;
                continue;
            }
            else if (myStrNCmp("-Blif", token, 2) == 0) {
                if (fileVerilog | fileBlif | fileAig)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileBlif = true; 
                continue;
            }
            else if (myStrNCmp("-Aig", token, 2) == 0) {
                if (fileVerilog | fileBlif | fileAig)
                return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileAig = true;
                continue;
            }
            else {
                if ( !fileVerilog && !fileBlif && !fileAig )
                    return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);

                if (filename == "") filename = token;
                else return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                continue;
            }
            
        }
    }
    if (filename == "") return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string filename)>");
    if(fileVerilog){
        string fileExt =  filename.substr(filename.size()-2,2);
        if(fileExt != ".v")
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, filename);
    }
    else if(fileBlif){
        string fileExt =  filename.substr(filename.size()-5,5);
        if(fileExt != ".blif")
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, filename);
    }
    else if(fileAig){
        string fileExt =  filename.substr(filename.size()-4,4);
        if(fileExt != ".aig")
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, filename);
    }

    cout << "\nfile name: " << filename << "\n";
    GVModEngine currEng = gvModMgr -> getGVEngine();
    if(currEng == GV_MOD_ENGINE_YOSYS){
        log_streams.push_back(&std::cout);            
        string yosCommand = "";
        yosys_setup();
        if(fileVerilog) yosCommand += "read_verilog ";
        else if(fileBlif) yosCommand += "read_blif ";
        //cout << yosCommand + filename <<"\n";
        run_pass(yosCommand + filename);
        //run_pass("help -all");
    }   
    else if (currEng == GV_MOD_ENGINE_ABC){
        /*char execCmd[128];
        cout << "Start abc read rtl\n";
        string outname = "test.aig";
        sprintf(execCmd, "read rtl %s", inname.c_str());
        sprintf(execCmd, "blast ntk");
        sprintf(execCmd, "write aig %s", outname.c_str());*/
        abcMgr -> abcReadDesign(filename);
        abcMgr -> abcPrintDesign();
        return GV_CMD_EXEC_DONE;    
    }
    else if(currEng == GV_MOD_ENGINE_V3){
        return GV_CMD_EXEC_DONE;
    }
    gvModMgr->setInputFileExist(true);
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
// PRint Info [-Verbose]
//----------------------------------------------------------------------

GVCmdExecStatus
GVPrintInfoCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVPrintInfoCmd" << endl;
    /*Abc_Start();
	pAbc = Abc_FrameGetGlobalFrame();
    Abc_Stop();*/
    int numFF = 0, numPI = 0, numPO = 0, numPIO = 0, numConst = 0, numNet = 0;
    int numMux = 0, numAnd = 0, numAdd = 0, numSub = 0, numMul = 0, numEq = 0, numNot = 0, numLe = 0, numGe = 0;
    bool verbose = false;
    
    // check option 
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        if (myStrNCmp("-Verbose", token, 2) == 0) {
            verbose = true;
            continue;
        }
        else{
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
        }
    }

    GVModEngine currEng = gvModMgr -> getGVEngine(); 
    if(currEng == GV_MOD_ENGINE_YOSYS){
        gvMsg(GV_MSG_IFO) << "Modules in current design: ";
        gvMsg(GV_MSG_IFO) << log_id(yosys_design->top_module()->name) <<"(" << GetSize(yosys_design->top_module()->wires()) <<" wires, " << GetSize(yosys_design->top_module()->cells()) << " cells)\n";

        for(auto wire : yosys_design->top_module()->wires()){
            //string wire_name = log_id(wire->name);
            if(wire->port_input){
                numPI++;
                //gvMsg(GV_MSG_IFO) << "  PI: " + wire_name  << setw(25  - (wire->name.size()))<< "->      " << wire->width << " bits width\n";
            }
            if(wire->port_output){
                numPO++;
                //gvMsg(GV_MSG_IFO) << "  PO: " + wire_name  << setw(25  - (wire->name.size()))<< "->      " << wire->width << " bits width\n";
            }
        }
        if(verbose){
            /**/
            for(auto cell : yosys_design->top_module()->cells()){
                if (cell->type.in(ID($mux))) numMux++;
                else if (cell->type.in(ID($logic_and))) numAnd++;
                else if(cell->type.in(ID($add))) numAdd++;
                else if (cell->type.in(ID($sub))) numSub++;
                else if (cell->type.in(ID($mul))) numMul++;
                else if (cell->type.in(ID($eq))) numEq++;
                else if (cell->type.in(ID($logic_not))) numNot++;
                else if (cell->type.in(ID($lt))) numLe++;
                else if (cell->type.in(ID($ge))) numGe++;

                //log("  This is cell: %s  %s\n", log_id(cell->name), log_id(cell->type));
            }
            gvMsg(GV_MSG_IFO) << "=========================\n";
            gvMsg(GV_MSG_IFO) << "   MUX" << setw(15) << numMux << "\n";
            gvMsg(GV_MSG_IFO) << "   AND" << setw(15) << numAnd << "\n";
            gvMsg(GV_MSG_IFO) << "   ADD" << setw(15) << numAdd << "\n";
            gvMsg(GV_MSG_IFO) << "   SUB" << setw(15) << numSub << "\n";
            gvMsg(GV_MSG_IFO) << "   MUL" << setw(15) << numMul << "\n";
            gvMsg(GV_MSG_IFO) << "   EQ" << setw(16) << numEq << "\n";
            gvMsg(GV_MSG_IFO) << "   NOT" << setw(15) << numNot << "\n";
            gvMsg(GV_MSG_IFO) << "   LT" << setw(16) << numLe << "\n";
            gvMsg(GV_MSG_IFO) << "   GE" << setw(16) << numGe << "\n";
            gvMsg(GV_MSG_IFO) << "=========================\n";
            gvMsg(GV_MSG_IFO) << "   PI" << setw(16) << numPI << "\n";
            gvMsg(GV_MSG_IFO) << "   PO" << setw(16) << numPI << "\n";
            gvMsg(GV_MSG_IFO) << "=========================\n";
            //gvMsg(GV_MSG_IFO) << "   PI" << setw(16) << numPI << "\n";
        }
        else
            gvMsg(GV_MSG_IFO) << "#PI = " << numPI << ", #PO = " << numPO << ", #PIO = " << numPIO << "\n";
    }
    else if(currEng == GV_MOD_ENGINE_ABC){
        return GV_CMD_EXEC_DONE;
    }
    else if(currEng == GV_MOD_ENGINE_V3){
        return GV_CMD_EXEC_DONE;
    }
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