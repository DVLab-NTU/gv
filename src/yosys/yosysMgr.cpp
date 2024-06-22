#include "yosysMgr.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ios>
#include <string>

#include "gvType.h"
#include "kernel/yosys.h"
#include "yosysExt.h"

YosysMgr* yosysMgr;

#define LEFT_WIDTH_10 std::setw(10) << std::left

/**
 * @brief Construct a new Yosys Mgr:: Yosys Mgr object
 *
 */
YosysMgr::YosysMgr() : _property(-1) {
    init();
    _designInfo = DesignInfo("clk", "rst");
    _yosysSigTypeStr = {"PI", "PO", "CLK", "RST", "REG"};
}

/**
 * @brief Initializes the Yosys framework.
 *
 * This function initializes the Yosys framework by calling the yosys_setup()
 */
void YosysMgr::init() {
    Yosys::yosys_setup();
}

/**
 * @brief Resets the Yosys framework to its initial state.
 *
 * This function resets the Yosys framework by deleting the existing yosys_design pointer
 * and creating a new, empty one.
 */
void YosysMgr::reset() {
    delete Yosys::yosys_design;
    Yosys::yosys_design = new Yosys::RTLIL::Design;
}

/**
 * @brief Enables or disables logging to standard output.
 *
 * This function allows enabling or disabling logging to standard output.
 *
 * @param enable A boolean indicating whether logging should be enabled or disabled.
 *               - If true, logging to standard output will be enabled.
 *               - If false, logging to standard output will be disabled.
 */
void YosysMgr::setLogging(const bool& enable) {
    if (enable && Yosys::log_streams.empty())
        Yosys::log_streams.push_back(&std::cout);
    else if (!enable && !Yosys::log_streams.empty())
        Yosys::log_streams.pop_back();
}

/**
 * @brief Saves the current design with a specified name.
 *
 * This function instructs Yosys to save the current design with the given name.
 *
 * @param designName The name to use when saving the design.
 */
void YosysMgr::saveDesign(const string& designName) {
    string command = "design -save " + designName;
    Yosys::run_pass(command);
    saveTopModuleName();
}

/**
 * @brief Loads a design with a specified name.
 *
 * This function instructs Yosys to load a design with the given name.
 *
 * @param designName The name of the design to load.
 */
void YosysMgr::loadDesign(const string& designName) {
    string command = "design -load " + designName;
    Yosys::run_pass(command);
}

/**
 * @brief Creates a name mapping file for the design specified in a Verilog file.
 *
 * This function invokes Yosys to read the Verilog file, perform preprocessing steps,
 * and create a name mapping file for the design.
 *
 * @param fileName The name of the Verilog file containing the design.
 */
void YosysMgr::createMapping(const string& fileName) {
    string yosys = "yosys -qp ";
    string readVerilog = "read_verilog " + fileName + "; ";
    string topModule = "hierarchy -auto-top; ";
    string preProcess = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger -map .map.txt ._temp_.aig";
    string command = yosys + "\"" + readVerilog + topModule + preProcess + writeAigMapping + "\"";
    system(command.c_str());
}

/**
 * @brief Reads a BLIF file and processes it.
 *
 * This function sets the current design type to BLIF, reads the specified BLIF file,
 * and then saves the design.
 *
 * @param fileName The name of the BLIF file to read.
 */
void YosysMgr::readBlif(const string& fileName) {
    const string designName = fileTypeStr[BLIF];
    string command = "read_blif " + fileName;
    Yosys::run_pass(command);
    saveDesign(designName);
}

/**
 * @brief Reads a Verilog file and processes it.
 *
 * This function sets the current design type to Verilog, reads the specified Verilog file,
 * and then saves the design.
 *
 * @param fileName The name of the Verilog file to read.
 */
void YosysMgr::readVerilog(const string& fileName) {
    const string designName = fileTypeStr[VERILOG];
    const string command = "read_verilog -sv " + fileName;
    Yosys::run_pass(command);
    saveDesign(designName);
    assignSignal();
}

/**
 * @brief Reads an AIGER file and sets the current design type accordingly.
 *
 * This function reads an AIGER file and sets the current design type to AIGER.
 *
 * @param fileName The name of the AIGER file to read.
 */
void YosysMgr::readAiger(const string& fileName) {
    // string command = "read_aiger " + fileName;
    // Yosys::run_pass(command);
}

/**
 * @brief Writes the current design to a BLIF file.
 *
 * This function loads the current design from a Verilog file, processes it using a series
 * of Yosys passes, and then writes the resulting design to a BLIF file.
 *
 * @param fileName The name of the BLIF file to write the design to.
 */
void YosysMgr::writeBlif(const string& fileName) {
    loadDesign(fileTypeStr[VERILOG]);
    string command = "hierarchy -auto-top; hierarchy -check; ";
    command += "proc; opt; ";
    command += "opt_expr -mux_undef; opt; ";
    command += "rename -hide; opt; ";
    command += "memory_collect; flatten; memory_unpack; ";
    command += "splitnets -driver; ";
    command += "setundef -zero -undriven; ";
    command += "dffunmap; ";
    command += "opt -fast -noff; ";
    command += "write_blif " + fileName;
    Yosys::run_pass(command);
}

/**
 * @brief Writes the current design to an AIGER file.
 *
 * This function loads the current design from a Verilog file, processes it using a series
 * of Yosys passes, and then writes the resulting design to an AIGER file.
 *
 * @param fileName The name of the AIGER file to write the design to.
 */
void YosysMgr::writeAiger(const string& fileName) {
    loadDesign(fileTypeStr[VERILOG]);
    string command = "hierarchy -auto-top; ";
    command += "flatten; proc; techmap; setundef -zero; aigmap; ";
    command += "write_aiger " + fileName;
    Yosys::run_pass(command);
}

/**
 * @brief Prints information about the current design.
 *
 * This function prints various information about the current design,
 * such as the number of modules, wires, cells, and different types of cells.
 *
 * @param verbose Flag indicating whether to print detailed information about cell types.
 *                If set to true, detailed information about each cell type will be printed.
 *                If set to false, only the counts of primary inputs (PIs) and primary outputs (POs) will be printed.
 */
void YosysMgr::printDesignInfo(const bool& verbose) {
    int numFF = 0, numPI = 0, numPO = 0, numPIO = 0, numConst = 0, numNet = 0;
    int numMux = 0, numAnd = 0, numAdd = 0, numSub = 0, numMul = 0, numEq = 0,
        numNot = 0, numLe = 0, numGe = 0;
    // Check design
    FileType fileType = getFileType();
    if (fileType == AIGER) {
        cout << "[ERROR]: Please read the word-level design first !!\n";
        cout << "[ERROR]: Use \"cirprint\" to print the aig info.\n";
        return;
    }

    loadDesign(fileTypeStr[fileType]);
    Yosys::RTLIL::Module* topModule = Yosys::yosys_design->top_module();
    // print info
    cout << "Modules in current design: ";
    string moduleName = topModule->name.str();
    cout << moduleName << "(" << Yosys::GetSize(topModule->wires()) << " wires, "
         << Yosys::GetSize(topModule->cells()) << " cells)\n";
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

/**
 * @brief Displays the schematic of the top module in the Yosys design.
 *
 * This function runs a series of Yosys passes to generate and display
 * the schematic of the top module in the current Yosys design.
 *
 */
void YosysMgr::showSchematic() {
    if (Yosys::yosys_design->top_module() == nullptr) return;
    Yosys::run_pass("hierarchy -auto-top");
    Yosys::run_pass("proc");
    Yosys::run_pass("opt");
    Yosys::run_pass("show");
}

/**
 * @brief Loads the simulation plugin.
 *
 * This function loads the simulation plugin by running a Yosys command.
 *
 * @note The simulation plugin file path is assumed to be defined in the GV_SIMSO_PATH macro.
 */
void YosysMgr::loadSimPlugin() {
    string command = "plugin -i ";
    string libsimPath = GV_SIMSO_PATH;
    string libsimName = "libsim.so";
    command = command + libsimPath + libsimName;
    runPass(command);
}

/**
 * @brief Runs a Yosys pass with the specified command.
 *
 * This function runs a Yosys pass with the given command.
 *
 * @param command The command string specifying the Yosys pass to run.
 */
void YosysMgr::runPass(const string& command) {
    // TODO: Check if the Yosys::run_pass is executed succefully
    Yosys::run_pass(command);
}

/**
 * @brief Retrieves the name of the top module.
 *
 * This function retrieves the name of the top module in the current Yosys design.
 *
 * @return
 */
void YosysMgr::saveTopModuleName() {
    if (!Yosys::yosys_design) {
        cout << "[ERROR]: Please read the word-level design first !!\n";
        return;
    }
    Yosys::RTLIL::Module* top = Yosys::yosys_design->top_module();
    string topModule = top->name.substr(1, strlen(Yosys::yosys_design->top_module()->name.c_str()) - 1);
    setTopModuleName(topModule);
}

/**
 * @brief
 *
 * @param currSignal
 * @param signalVec
 * @return true
 * @return false
 */
static bool findSignal(const string& currSignal, const vector<string>& signalVec) {
    for (int i = 0; i < signalVec.size(); ++i)
        if (currSignal.find(signalVec[i]) != string::npos) return true;
    return false;
}

/**
 * @brief
 *
 */
void YosysMgr::assignSignal() {
    _topModule = Yosys::yosys_design->top_module();
    for (auto wire : _topModule->wires()) {
        YosysSignal* newSig = new YosysSignal(wire);
        bool isDesignSig = (wire->name.str()[0] == '\\') ? true : false;
        bool isClk = findSignal(newSig->getName(), _designInfo.clkName);
        bool isReset = findSignal(newSig->getName(), _designInfo.rstName);
        if (isDesignSig) {
            if (wire->port_input) {
                if (isClk) _clkList.push_back(newSig);
                else if (isReset) _rstList.push_back(newSig);
                else _piList.push_back(newSig);
            } else if (wire->port_output) _poList.push_back(newSig);
            else _regList.push_back(newSig);
        }
    }
}

/**
 * @brief
 *
 */
void YosysMgr::printSignal(const YosysSigType& sigType) {
    SignalVec tmpVec = _piList;
    if (sigType == YosysSigType::PO) tmpVec = _poList;
    else if (sigType == YosysSigType::CLK) tmpVec = _clkList;
    else if (sigType == YosysSigType::RST) tmpVec = _rstList;
    else if (sigType == YosysSigType::REG) tmpVec = _regList;

    cout << " /////////////" << _yosysSigTypeStr[sigType] << "/////////////\n";
    for (int i = 0; i < tmpVec.size(); ++i) {
        cout << LEFT_WIDTH_10 << " " + _yosysSigTypeStr[sigType] << ": " << LEFT_WIDTH_10 << tmpVec[i]->getName()
             << LEFT_WIDTH_10 << "ID: " << LEFT_WIDTH_10 << tmpVec[i]->getId()
             << LEFT_WIDTH_10 << "Width: " << LEFT_WIDTH_10 << tmpVec[i]->getWidth() << endl;
    }
}
