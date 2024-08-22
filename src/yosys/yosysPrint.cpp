
#include <iomanip>

#include "fmt/color.h"
#include "fmt/core.h"
#include "gvType.h"
#include "yosysExt.h"
#include "yosysMgr.h"

#define LEFT_WIDTH_10 std::setw(10) << std::left

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

    // loadDesign(fileTypeStr[fileType]);
    loadDesign(_fileVec.front());
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

/**
 * @brief
 *
 */
void YosysMgr::printFSM() {
    unsigned i = 0;
    fmt::println("  Transition Table (state_in, ctrl_in, state_out, ctrl_out):");
    for (auto const& fsm : _fsmList) {
        fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, "{:>5}", i);
        fmt::print("{0:>5}: {1:>5} {2:<}   -> {3:>5} {4:<}", i, fsm.stateIn, fsm.ctrlIn, fsm.stateOut, fsm.ctrlOut);
        fmt::println("");
        ++i;
    }
}
