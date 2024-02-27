#include "yosysMgr.h"

#include <cstdlib>
#include <iomanip>

#include "fileType.h"

YosysMgr* yosysMgr;

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
    Yosys::yosys_design = new RTLIL::Design;
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
    if (enable && log_streams.empty())
        log_streams.push_back(&std::cout);
    else if (!enable && !log_streams.empty())
        log_streams.pop_back();
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
    run_pass(command);
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
    run_pass(command);
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
    string yosys           = "yosys -qp ";
    string readVerilog     = "read_verilog " + fileName + "; ";
    string topModule       = "hierarchy -auto-top; ";
    string preProcess      = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger -map .map.txt ._temp_.aig";
    string command         = yosys + "\"" + readVerilog + topModule + preProcess + writeAigMapping + "\"";
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
    _fileType               = BLIF;
    const string designName = fileTypeStr[BLIF];
    string command          = "read_blif " + fileName;
    run_pass(command);
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
    _fileType               = VERILOG;
    const string designName = fileTypeStr[VERILOG];
    const string command    = "read_verilog -sv " + fileName;
    run_pass(command);
    saveDesign(designName);
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
    // run_pass(command);
    _fileType = AIGER;
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
    run_pass(command);
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
    run_pass(command);
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

/**
 * @brief Displays the schematic of the top module in the Yosys design.
 *
 * This function runs a series of Yosys passes to generate and display
 * the schematic of the top module in the current Yosys design.
 *
 */
void YosysMgr::showSchematic() {
    if (yosys_design->top_module() == nullptr) return;
    run_pass("hierarchy -auto-top");
    run_pass("proc");
    run_pass("opt");
    run_pass("show");
}

/**
 * @brief Retrieves the name of the top module.
 *
 * This function retrieves the name of the top module in the current Yosys design.
 *
 * @return A string containing the name of the top module.
 *         If the design has not been loaded, an empty string is returned.
 */
string YosysMgr::getTopModuleName() const {
    if (!yosys_design) {
        cout << "[ERROR]: Please read the word-level design first !!\n";
        return "";
    }
    RTLIL::Module* top   = yosys_design->top_module();
    string topModuleName = top->name.substr(1, strlen(yosys_design->top_module()->name.c_str()) - 1);
    return topModuleName;
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
    run_pass(command + GV_SIMSO_PATH + "sim.so");
}

/**
 * @brief Runs a Yosys pass with the specified command.
 *
 * This function runs a Yosys pass with the given command.
 *
 * @param command The command string specifying the Yosys pass to run.
 */
void YosysMgr::runPass(const string& command) {
    run_pass(command);
}