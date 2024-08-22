#include "yosysMgr.h"

#include <cstdlib>
#include <string>

#include "fmt/core.h"
#include "gvType.h"
#include "kernel/yosys.h"
#include "passes/fsm/fsmdata.h"

YosysMgr* yosysMgr;

/**
 * @brief Construct a new Yosys Mgr:: Yosys Mgr object
 *
 */
YosysMgr::YosysMgr() : _property(-1) {
    init();
    _designInfo      = DesignInfo("clk", "rst");
    _yosysSigTypeStr = {"PI", "PO", "CLK", "RST", "REG"};
}

YosysMgr::~YosysMgr() {
    resetDesign();
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
    _fileVec.emplace_back(designName);
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
 * @brief Delete a design with a specified name.
 *
 * This function delete the specified design pointer in the Yosys
 *
 * @param designName The name of the design to delete.
 */
void YosysMgr::deleteDesign(const string& designName) {
    string command = "design -delete " + designName;
    Yosys::run_pass(command);
}

/**
 * @brief Reset the Yosys design
 *
 * This function reset the design in the Yosys
 *
 */
void YosysMgr::resetDesign() {
    string command = "design -reset";
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
    // loadDesign(fileName);
    string yosys           = "yosys -qp ";
    string readVerilog     = "read_verilog " + fileName + "; ";
    string topModule       = "hierarchy -auto-top; ";
    string preProcess      = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger -map .map.txt ._temp_.aig";
    string command         = yosys + "\"" + readVerilog + topModule + preProcess + writeAigMapping + "\"";
    system(command.c_str());
    // runPass(command);
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
    string command          = "read_blif " + fileName;
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
    // const string designName = fileName;
    // const string command = "read_verilog -sv " + fileName;
    const string command = fmt::format("read_verilog -sv {0}", fileName);
    Yosys::run_pass(command);
    // saveDesign(designName);
    saveDesign(fileName);
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
    string command = "read_aiger " + fileName;
    Yosys::run_pass(command);
    saveDesign(fileTypeStr[AIGER]);
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
    // loadDesign(fileTypeStr[VERILOG]);
    loadDesign(fileTypeStr[VERILOG]);
    string command = "hierarchy -auto-top; ";
    command += "flatten; proc; techmap; setundef -zero; aigmap; ";
    command += "write_aiger " + fileName;
    Yosys::run_pass(command);
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
    string command    = "plugin -i ";
    string libsimPath = GV_SIMSO_PATH;
    string libsimName = "libsim.so";
    command           = command + libsimPath + libsimName;
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
    string topModule          = top->name.substr(1, strlen(Yosys::yosys_design->top_module()->name.c_str()) - 1);
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
    _design    = Yosys::yosys_design;
    _topModule = Yosys::yosys_design->top_module();
    for (auto wire : _topModule->wires()) {
        YosysSignal* newSig = new YosysSignal(wire);
        bool isDesignSig    = (wire->name.str()[0] == '\\') ? true : false;
        bool isClk          = findSignal(newSig->getName(), _designInfo.clkName);
        bool isReset        = findSignal(newSig->getName(), _designInfo.rstName);
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

void YosysMgr::extractFSM() {
    runPass("hierarchy -top cle");
    runPass("prep -top cle");
    runPass("proc_clean");
    runPass("proc_init");
    runPass("proc_mux");
    runPass("proc_dff");
    runPass("proc_clean");
    runPass("fsm_detect");
    runPass("fsm_extract");
    fetchAllFSMInfo();
    printFSM();
}

void YosysMgr::fetchAllFSMInfo() {
    for (auto mod : _design->selected_modules())
        for (auto cell : mod->selected_cells())
            if (cell->type == ID($fsm)) fetchFSMInfo(cell);
}

void YosysMgr::fetchFSMInfo(Yosys::RTLIL::Cell* cell) {
    Yosys::FsmData fsm_data;
    fsm_data.copy_from_cell(cell);
    fsm_data.log_info(cell);
    for (int i = 0; i < GetSize(fsm_data.transition_table); i++) {
        Yosys::FsmData::transition_t& tr = fsm_data.transition_table[i];
        // YosysFSM fsm                     = {tr.state_in, tr.state_out, Yosys::log_signal(tr.ctrl_in), Yosys::log_signal(tr.ctrl_out)};
        _fsmList.push_back({tr.state_in, tr.state_out,
                            Yosys::log_signal(tr.ctrl_in), Yosys::log_signal(tr.ctrl_out)});
        // fmt::print("  {0}: {1} {2}   -> {3} {4}\n", i, tr.state_in, Yosys::log_signal(tr.ctrl_in), tr.state_out, Yosys::log_signal(tr.ctrl_out));
        // fmt::print(" {0}: {1} {2}   -> {3} {4}\n", i, fsm.stateIn, fsm.ctrlIn, fsm.stateOut, fsm.ctrlOut);
    }
}

/*
        void log_info(RTLIL::Cell *cell)
        {
                log("-------------------------------------\n");
                log("\n");
                log("  Information on FSM %s (%s):\n", cell->name.c_str(), cell->parameters[ID::NAME].decode_string().c_str());
                log("\n");
                log("  Number of input signals:  %3d\n", num_inputs);
                log("  Number of output signals: %3d\n", num_outputs);
                log("  Number of state bits:     %3d\n", state_bits);

                log("\n");
                log("  Input signals:\n");
                RTLIL::SigSpec sig_in = cell->getPort(ID::CTRL_IN);
                for (int i = 0; i < GetSize(sig_in); i++)
                        log("  %3d: %s\n", i, log_signal(sig_in[i]));

                log("\n");
                log("  Output signals:\n");
                RTLIL::SigSpec sig_out = cell->getPort(ID::CTRL_OUT);
                for (int i = 0; i < GetSize(sig_out); i++)
                        log("  %3d: %s\n", i, log_signal(sig_out[i]));

                log("\n");
                log("  State encoding:\n");
                for (int i = 0; i < GetSize(state_table); i++)
                        log("  %3d: %10s%s\n", i, log_signal(state_table[i], false),
                                        int(i) == reset_state ? "  <RESET STATE>" : "");

                log("\n");
                log("  Transition Table (state_in, ctrl_in, state_out, ctrl_out):\n");
                for (int i = 0; i < GetSize(transition_table); i++) {
                        transition_t &tr = transition_table[i];
                        log("  %5d: %5d %s   -> %5d %s\n", i, tr.state_in, log_signal(tr.ctrl_in), tr.state_out, log_signal(tr.ctrl_out));
                }

                log("\n");
                log("-------------------------------------\n");
        }
*/
