#ifndef GV_NTK_CMD_C
#define GV_NTK_CMD_C

#include "gvNtkCmd.h"
#include "gvMsg.h"
#include <string>
#include "util.h"
#include <vector>
#include <map>

// #include "kernel/yosys.h"
#include "gvNtk.h"
#include "gvModMgr.h"
#include "gvAbcMgr.h"
#include "gvNtk.h"
#include "gvWordLevel.h"

USING_YOSYS_NAMESPACE

bool GVinitNtkCmd() {
    return (
            gvCmdMgr->regCmd("SEt Engine",         2, 1, new GVSetEngineCmd   ) &&
            gvCmdMgr->regCmd("REad Design",        2, 1, new GVReadDesignCmd  ) &&
            gvCmdMgr->regCmd("PRint Info",         2, 1, new GVPrintInfoCmd   ) &&
            gvCmdMgr->regCmd("FILE2 Aig",          4, 1, new GVFile2AigCmd    ) && 
            gvCmdMgr->regCmd("YOSYSCMD",           8, new GVYosysOriginalCmd  )&&
            gvCmdMgr->regCmd("FILE2 Btor",          4, 1, new GVFile2BtorCmd )
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

    bool engABC = false, engYOSYS = false;
    // try to match engine names
    if (n == 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "<(string engineName)>");
    else {
        for (size_t i = 0; i < n; ++i) {
            const string& token = options[i];
            if (myStrNCmp("yosys", token, 1) == 0) {
                if (engABC | engYOSYS)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                engYOSYS = true;
                continue;
            }
            else if (myStrNCmp("abc", token, 1) == 0) {
                if (engABC | engYOSYS)
                    return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, token);
                engABC = true; 
                continue;
            }
            else {
                if ( !engABC && !engYOSYS )
                    return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            }
        }
    }

    // set current engine
    if(engYOSYS)  gvModMgr->setGVEngine(GV_MOD_ENGINE_YOSYS);
    else if (engABC) gvModMgr->setGVEngine(GV_MOD_ENGINE_ABC);

    // print the success message
    int engPos = gvModMgr->getGVEngine();
    string engNameList[2] = {"yosys","abc"};
    cout << "Set Engine \""<< engNameList[engPos] <<"\" Success !!" <<endl;
    return GV_CMD_EXEC_DONE;
}

void
GVSetEngineCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SEt Engine <(string engineName)> " << endl;
    gvMsg(GV_MSG_IFO) << "Param: <(string engineName)>  :  Name of the engine. <(yosys) | (abc)>" << endl;
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
        string moduleName = gvRTLDesign->getDesign()->top_module()->name.str();
        cout << moduleName <<"(" << GetSize(gvRTLDesign->getDesign()->top_module()->wires()) <<" wires, " << GetSize(gvRTLDesign->getDesign()->top_module()->cells()) << " cells)\n";
        for(auto wire : gvRTLDesign->getDesign()->top_module()->wires()){
            //string wire_name = log_id(wire->name);
            if(wire->port_input) numPI++;
            else if(wire->port_output) numPO++;
        }
        if(verbose){
            for(auto cell : gvRTLDesign->getDesign()->top_module()->cells()){
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

//----------------------------------------------------------------------
// YOSYSCMD <command in ABC>
//----------------------------------------------------------------------

GVCmdExecStatus
GVYosysOriginalCmd ::exec(const string& option) {
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    string command;
    for (size_t i = 0; i < n; ++i) {
      command += options[i];
      if (i < n-1) { command += " "; }
    }

    // calling Yosys's command
    if ((yosys_design->top_module() == NULL) && (myStrNCmp("read", command, 4))) 
    {
        cout << "Error: Please read in a design first !" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, command);
    }
    run_pass(command);
}

void
GVYosysOriginalCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: YOSYSCMD <command in Yosys> " << endl;
}

void
GVYosysOriginalCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "YosysCMD: " << "Directly call Yosys's command." << endl;
}



GVCmdExecStatus
GVFile2BtorCmd::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "Create Btor from RTL " << endl;
  
    // gvMsg(GV_MSG_IFO) << "I am GVFile2AigCmd" << endl;
    vector<string> options;
    GVCmdExec::lexOptions(option, options);
    size_t n = options.size();
    if(n!=2)cout<<"wrong arg number\n";
    string top=options[0];
    string outputf=options[1];
    cout<<"top = "<<top<<" outputf = "<<outputf<<endl;
    string commend;
    commend="hierarchy -top ";
    commend+=top;
    run_pass(commend);
    commend="hierarchy -check";
    run_pass(commend);
    commend="proc";
    run_pass(commend);
    commend="opt";
    run_pass(commend);
    commend="opt_expr -mux_undef";
    run_pass(commend);
    commend="opt";
    run_pass(commend);
    commend="rename -hide";
    run_pass(commend);
    commend="opt";
    run_pass(commend);
    commend="memory_collect";
    run_pass(commend);
    commend="flatten";
    run_pass(commend);
    commend="memory_unpack";
    run_pass(commend);
    commend="splitnets -driver";
    run_pass(commend);
    commend="setundef -zero -undriven";
    run_pass(commend);
    commend="dffunmap";
    run_pass(commend);
    commend="opt -fast -noff";
    run_pass(commend);
    commend="write_btor ";
    commend+=outputf;
    std::ofstream *f = new std::ofstream;
    f->open(outputf.c_str(),std::ofstream::trunc);
    RTLIL::Module *topmod = yosys_design->top_module();
    *f << stringf("; BTOR description generated by %s for module %s.\n",
				yosys_version_str, log_id(topmod));
    BtorWorker2(*f, topmod, false, false, false, false, "btorinfo.log");
    *f << stringf("; end of yosys output\n");
    f->close();
    delete f;
    boolectorTest();
    // set the aig file name
    //gvModMgr->setAigFileName(outputname);

    return GV_CMD_EXEC_DONE;
}

void
GVFile2BtorCmd ::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: File2 Btor <-Verilog> <input_filename> <output_filename.btor> " << endl;
}

void
GVFile2BtorCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "File2 Btor: " << "Convert verilog file into btor. " << endl;
}


#endif
