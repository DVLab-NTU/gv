#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"
#include <vector>
#include <map>

#include "kernel/yosys.h"
#include "gvModMgr.h"
#include "gvAbcMgr.h"
#include "gvV3Mgr.h"

USING_YOSYS_NAMESPACE

bool GVinitNtkCmd() {
    return (
            gvCmdMgr->regCmd("SEt Engine",         2, 1, new GVSetEngineCmd   )  &&
            gvCmdMgr->regCmd("REad Design",         2, 1, new GVReadDesignCmd   )  &&
            gvCmdMgr->regCmd("PRint Info",          2, 1, new GVPrintInfoCmd    )  &&
            gvCmdMgr->regCmd("FILE2 Aig",           4, 1, new GVFile2AigCmd  )
    );
}

//----------------------------------------------------------------------
// SEt Engine <(string engineName)>
//----------------------------------------------------------------------

GVCmdExecStatus
GVSetEngineCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVSetEngineCmd" << endl;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    bool engABC = false, engYOSYS = false, engV3 = false;
    // try to match engine names
    if (n == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string engineName)>");
    else {
        for (size_t i = 0; i < n; ++i) {
            const string& token = options[i];
            if (myStrNCmp("yosys", token, 1) == 0) {
                if (engABC | engYOSYS | engV3)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                engYOSYS = true;
                continue;
            }
            else if (myStrNCmp("abc", token, 1) == 0) {
                if (engABC | engYOSYS | engV3)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                engABC = true; 
                continue;
            }
            else if (myStrNCmp("v3", token, 1) == 0) {
                if (engABC | engYOSYS | engV3)  
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                engV3 = true;
                continue;
            }
            else {
                if ( !engABC && !engYOSYS && !engV3)
                    return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
        }
    }

    // set current engine
    if(engYOSYS)  gvModMgr->setGVEngine(GV_MOD_ENGINE_YOSYS);
    else if (engABC) gvModMgr->setGVEngine(GV_MOD_ENGINE_ABC);
    else if(engV3)  gvModMgr->setGVEngine(GV_MOD_ENGINE_V3);

    // print the success message
    int engPos = gvModMgr->getGVEngine();
    string engNameList[3] = {"yosys","abc","V3"};
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

    bool fileVerilog = false, fileBlif = false, fileAig = false, fileBtor = false;
    size_t n = options.size();
    string filename = "";

    // try to match file type options
    if (n == 0)
        fileVerilog = true;
    else {
        for (size_t i = 0; i < n; ++i) {
            const string& token = options[i];
            if (myStrNCmp("-Verilog", token, 2) == 0) {
                if (fileVerilog | fileBlif | fileAig | fileBtor)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileVerilog = true;
                continue;
            }
            else if (myStrNCmp("-Blif", token, 3) == 0) {
                if (fileVerilog | fileBlif | fileAig | fileBtor)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileBlif = true; 
                continue;
            }
            else if (myStrNCmp("-Aig", token, 2) == 0) {
                if (fileVerilog | fileBlif | fileAig | fileBtor)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileAig = true;
                continue;
            }
            else if (myStrNCmp("-Btor", token, 3) == 0) {
                if (fileVerilog | fileBlif | fileAig | fileBtor)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                fileBtor = true;
                continue;
            }
            else {
                if ( !fileVerilog && !fileBlif && !fileAig && !fileBtor)
                    return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);

                if (filename == "") filename = token;
                else return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                continue;
            }
        }
    }
    // check filename
    if (filename == "") return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string filename)>");

    // check file extension 
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
        // set the aig file name
        gvModMgr->setAigFileName(filename);
    }
    else if(fileBtor){
        string fileExt =  filename.substr(filename.size()-5,5);
        if(fileExt != ".btor")
            return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, filename);
    }

    // print the input file name
    cout << "\nfile name: " << filename << "\n";

    // set input file name to Mode manager
    gvModMgr->setInputFileName(filename);
    gvModMgr->setInputFileExist(true);
    GVModEngine currEng = gvModMgr -> getGVEngine();

    // read design
    if(currEng == GV_MOD_ENGINE_YOSYS){  
        if(fileAig | fileBtor){
            gvMsg(GV_MSG_IFO) << "[ERROR]: Engine yosys doesn't support aig file and btor file !!" << endl;
            return GV_CMD_EXEC_NOP;
        }
        string yosCommand = "";
        if(fileVerilog) yosCommand += "read_verilog ";
        else if(fileBlif) yosCommand += "read_blif ";
        run_pass(yosCommand + filename);
    }   
    else if (currEng == GV_MOD_ENGINE_ABC){
        abcMgr -> abcReadDesign(filename);
    }
    else if(currEng == GV_MOD_ENGINE_V3){
        if(fileBlif){
            gvMsg(GV_MSG_IFO) << "[ERROR]: Engine V3 doesn't support blif file !!" << endl;
            return GV_CMD_EXEC_NOP;
        }
        v3Mgr->init();
        char execCmd[128], inname[128]; string cmd = "" ,inname_str = "";
        if(fileVerilog) inname_str = "rtl";
        else if(fileAig) inname_str = "aig";
        else if(fileBtor) inname_str = "btor";
        inname_str += " "+ filename;
        strcpy(inname, inname_str.c_str());
        sprintf(execCmd, "read %s", inname);
        string command = string(execCmd);
        v3_exe = v3Mgr->parseCmd(command, cmd);
        v3_exe->exec(cmd);
    }

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

    int numFF = 0, numPI = 0, numPO = 0, numPIO = 0, numConst = 0, numNet = 0;
    int numMux = 0, numAnd = 0, numAdd = 0, numSub = 0, numMul = 0, numEq = 0, numNot = 0, numLe = 0, numGe = 0;
    bool verbose = false;
    
    // check options
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    // try to match options
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

    // print info
    GVModEngine currEng = gvModMgr -> getGVEngine(); 
    if(currEng == GV_MOD_ENGINE_YOSYS){
        gvMsg(GV_MSG_IFO) << "Modules in current design: ";
        string moduleName = yosys_design->top_module()->name.str();
        cout << moduleName <<"(" << GetSize(yosys_design->top_module()->wires()) <<" wires, " << GetSize(yosys_design->top_module()->cells()) << " cells)\n";
        for(auto wire : yosys_design->top_module()->wires()){
            //string wire_name = log_id(wire->name);
            if(wire->port_input) numPI++;
            else if(wire->port_output) numPO++;
        }
        if(verbose){
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
            }
            gvMsg(GV_MSG_IFO) << "==================================================\n";
            gvMsg(GV_MSG_IFO) << "   MUX" << setw(40) << numMux << "\n";
            gvMsg(GV_MSG_IFO) << "   AND" << setw(40) << numAnd << "\n";
            gvMsg(GV_MSG_IFO) << "   ADD" << setw(40) << numAdd << "\n";
            gvMsg(GV_MSG_IFO) << "   SUB" << setw(40) << numSub << "\n";
            gvMsg(GV_MSG_IFO) << "   MUL" << setw(40) << numMul << "\n";
            gvMsg(GV_MSG_IFO) << "   EQ"  << setw(41) << numEq << "\n";
            gvMsg(GV_MSG_IFO) << "   NOT" << setw(40) << numNot << "\n";
            gvMsg(GV_MSG_IFO) << "   LT"  << setw(41) << numLe << "\n";
            gvMsg(GV_MSG_IFO) << "   GE"  << setw(41) << numGe << "\n";
            gvMsg(GV_MSG_IFO) << "--------------------------------------------------\n";
            gvMsg(GV_MSG_IFO) << "   PI"  << setw(41) << numPI << "\n";
            gvMsg(GV_MSG_IFO) << "   PO"  << setw(41) << numPO << "\n";
            gvMsg(GV_MSG_IFO) << "==================================================\n";
        }
        else
            gvMsg(GV_MSG_IFO) << "#PI = " << numPI << ", #PO = " << numPO << ", #PIO = " << numPIO << "\n";
    }
    else if(currEng == GV_MOD_ENGINE_ABC){
        return GV_CMD_EXEC_DONE;
    }
    else if(currEng == GV_MOD_ENGINE_V3){
        v3Mgr->init();
        char execCmd[128]; string cmd = "";
        if(verbose)
            sprintf(execCmd, "print ntk -verbose");
        else
            sprintf(execCmd, "print ntk");
        string command = string(execCmd);
        v3_exe = v3Mgr->parseCmd(command, cmd);
        v3_exe->exec(cmd);
    }
    return GV_CMD_EXEC_DONE;
}

void
GVPrintInfoCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: PRint Info [-Verbose]" << endl;
}

void
GVPrintInfoCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "PRint Info: " << "Print circuit information extracted by our parser." << endl;
}

//-------------------------------------------------------------------------------------------------------
// FILE2 Aig < [-Verilog | -Blif] > -Input <infilename> -TOP <top_module_name> -Output <outfilename>.aig
//-------------------------------------------------------------------------------------------------------

GVCmdExecStatus
GVFile2AigCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVFile2AigCmd" << endl;
    
    string inname, topname, outname;
    bool fileVerilog = false, fileBlif = false;
    bool hasInfile = false, hasTop = false, hasOutfile = false;

    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();

    for (size_t i = 0; i < n; ++i) 
    {
        const string& token = options[i];
        if (myStrNCmp("-Verilog", token, 2) == 0) 
        {
            if (fileVerilog | fileBlif) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            fileVerilog = true;
            continue;
        }
        else if (myStrNCmp("-Blif", token, 3) == 0) 
        {
            if (fileVerilog | fileBlif) { return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token); }
            fileBlif = true; 
            continue;
        }
        else if (myStrNCmp("-Input", token, 2) == 0) 
        {
            // if no specify filename
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            else { inname = options[i+1]; ++i; }
            hasInfile = true;
            continue;
        }
        else if (myStrNCmp("-TOP", token, 4) == 0) 
        {
             // if no specify top module
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            else { topname = options[i+1]; ++i; }
            hasTop = true;
            continue;
        }
        else if (myStrNCmp("-Output", token, 2) == 0) 
        {
            // if no specify filename
            if ((i+1) >= n) { return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token); }
            else if (strncmp(options[i+1].substr(options[i+1].length()-4, options[i+1].length()).c_str(), ".aig", 4))
            {
                cout << "[ERROR]: Please output an \"AIG\" file (<filename>.aig) !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
            else { outname = options[i+1]; ++i; }
            hasOutfile = true;
            continue;
        }
    }

    // command 
    string readCmd, topCmd, outCmd;

    if (!fileVerilog && !fileBlif) 
    { 
        cout << "[ERROR]: Please specify input file format (-Verilog | -Blif) !" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "FILE2 Aig"); 
    }
    else if (!hasInfile || !hasOutfile)
    {
        cout << "[ERROR]: Please specify the file options !" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "FILE2 Aig");
    }
    else if (fileVerilog) 
    { 
        if (!hasTop)
        {
            cout << "[ERROR]: Please specify the top module name options !" << endl;
            return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "FILE2 Aig");
        }   
        readCmd = "read_verilog " + inname; 
    }
    else if (fileBlif) { readCmd = "read_blif " + inname; }
    topCmd = "synth -flatten -top " + topname;
    outCmd = "write_aiger " + outname;

    run_pass(readCmd);
    run_pass(topCmd);
    run_pass("dffunmap");
    run_pass("abc -g AND");
    run_pass(outCmd);

    return GV_CMD_EXEC_DONE;
}

void
GVFile2AigCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: FILE2 Aig <[-Verilog|-Blif]> -Input <infilename> -TOP <top_module_name> -Output <outfilename>.aig " << endl;
}

void
GVFile2AigCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "File2 Aig: " << "Convert verilog file into AIG. " << endl;
}



#endif
