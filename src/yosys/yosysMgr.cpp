#include "yosysMgr.h"

#include <cstdlib>
#include <iomanip>

#include "fileType.h"

YosysMgr* yosysMgr;

void YosysMgr::init() {
    Yosys::yosys_setup();
}

void YosysMgr::reset() {
    delete Yosys::yosys_design;
    Yosys::yosys_design = new RTLIL::Design;

    RTLIL::Design* gvYsysDesign = new RTLIL::Design;
}

void YosysMgr::setLogging(const bool& enable) {
    if (enable && log_streams.empty())
        log_streams.push_back(&std::cout);
    else if (!enable && !log_streams.empty())
        log_streams.pop_back();
}

void YosysMgr::saveDesign(const string& designName) {
    string command = "design -save " + designName;
    run_pass(command);
}

void YosysMgr::loadDesign(const string& designName) {
    string command = "design -load " + designName;
    run_pass(command);
}

void YosysMgr::createMapping(const string& fileName) {
    string yosys           = "yosys -qp ";
    string readVerilog     = "read_verilog " + fileName + "; ";
    string topModule       = "hierarchy -auto-top; ";
    string preProcess      = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger -map .map.txt ._temp_.aig";
    string command         = yosys + "\"" + readVerilog + topModule + preProcess + writeAigMapping + "\"";
    system(command.c_str());
}

void YosysMgr::readBlif(const string& fileName) {
    _fileType               = BLIF;
    const string designName = fileTypeStr[BLIF];
    string command          = "read_blif " + fileName;
    run_pass(command);
    saveDesign(designName);
}

void YosysMgr::readVerilog(const string& fileName) {
    _fileType               = VERILOG;
    const string designName = fileTypeStr[VERILOG];
    const string command    = "read_verilog -sv " + fileName;
    run_pass(command);
    saveDesign(designName);
}

void YosysMgr::readAiger(const string& fileName) {
    // string command = "read_aiger " + fileName;
    // run_pass(command);
    _fileType = AIGER;
}

void YosysMgr::writeBlif(const string& fileName) {
    loadDesign(fileTypeStr[VERILOG]);
    string command;
    run_pass(command);
    command = "hierarchy -auto-top ";
    run_pass(command);
    command = "hierarchy -check";
    run_pass(command);
    command = "proc";
    run_pass(command);
    command = "opt";
    run_pass(command);
    command = "opt_expr -mux_undef";
    run_pass(command);
    command = "opt";
    run_pass(command);
    command = "rename -hide";
    run_pass(command);
    command = "opt";
    run_pass(command);
    command = "memory_collect";
    run_pass(command);
    command = "flatten";
    run_pass(command);
    command = "memory_unpack";
    run_pass(command);
    command = "splitnets -driver";
    run_pass(command);
    command = "setundef -zero -undriven";
    run_pass(command);
    command = "dffunmap";
    run_pass(command);
    command = "opt -fast -noff";
    run_pass(command);
    command = "write_blif ";
    command += fileName;
    run_pass(command);
}

void YosysMgr::writeAiger(const string& fileName) {
    loadDesign(fileTypeStr[VERILOG]);
    string topModule       = "hierarchy -auto-top; ";
    string preProcess      = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger " + fileName;
    string command         = topModule + preProcess + writeAigMapping;
    run_pass(command);
}

void YosysMgr::printDesignInfo(const bool& verbose) {
    int numFF = 0, numPI = 0, numPO = 0, numPIO = 0, numConst = 0, numNet = 0;
    int numMux = 0, numAnd = 0, numAdd = 0, numSub = 0, numMul = 0, numEq = 0,
        numNot = 0, numLe = 0, numGe = 0;
    // Check design
    if (_fileType == AIGER) {
        cout << "[ERROR]: Please read the word-level design first !!\n";
        cout << "[ERROR]: Use \"cirprint\" to print the aig info.\n";
        return;
    }

    loadDesign(fileTypeStr[_fileType]);
    RTLIL::Module* topModule = yosys_design->top_module();
    // print info
    cout << "Modules in current design: ";
    string moduleName = topModule->name.str();
    cout << moduleName << "(" << GetSize(topModule->wires()) << " wires, "
         << GetSize(topModule->cells()) << " cells)\n";
    for (auto wire : topModule->wires()) {
        // string wire_name = log_id(wire->name);
        if (wire->port_input) numPI++;
        else if (wire->port_output) numPO++;
    }
    if (verbose) {
        for (auto cell : topModule->cells()) {
            if (cell->type.in(ID($mux))) numMux++;
            else if (cell->type.in(ID($logic_and))) numAnd++;
            else if (cell->type.in(ID($add))) numAdd++;
            else if (cell->type.in(ID($sub))) numSub++;
            else if (cell->type.in(ID($mul))) numMul++;
            else if (cell->type.in(ID($eq))) numEq++;
            else if (cell->type.in(ID($logic_not))) numNot++;
            else if (cell->type.in(ID($lt))) numLe++;
            else if (cell->type.in(ID($ge))) numGe++;
        }
        cout
            << "==================================================\n";
        cout << "   MUX" << setw(40) << numMux << "\n";
        cout << "   AND" << setw(40) << numAnd << "\n";
        cout << "   ADD" << setw(40) << numAdd << "\n";
        cout << "   SUB" << setw(40) << numSub << "\n";
        cout << "   MUL" << setw(40) << numMul << "\n";
        cout << "   EQ" << setw(41) << numEq << "\n";
        cout << "   NOT" << setw(40) << numNot << "\n";
        cout << "   LT" << setw(41) << numLe << "\n";
        cout << "   GE" << setw(41) << numGe << "\n";
        cout
            << "--------------------------------------------------\n";
        cout << "   PI" << setw(41) << numPI << "\n";
        cout << "   PO" << setw(41) << numPO << "\n";
        cout
            << "==================================================\n";
    } else
        cout << "#PI = " << numPI << ", #PO = " << numPO
             << ", #PIO = " << numPIO << "\n";
}

void YosysMgr::showSchematic() {
    if (yosys_design->top_module() == nullptr) return;
    run_pass("hierarchy -auto-top");
    run_pass("proc");
    run_pass("opt");
    run_pass("show");
}