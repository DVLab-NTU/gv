#include "vrltMgr.h"

#include <experimental/filesystem>
#include <string>

#include "cirMgr.h"
#include "fstream"
#include "simMgr.h"
#include "yosysMgr.h"

#define GREEN_TEXT "\033[32m"  // ANSI escape code for green color
#define RESET_COLOR "\033[0m"  // ANSI escape code to reset color

using namespace std;
VRLTMgr* vrltMgr;

static string signalWidth2Type(const int& width) {
    if (width <= 8) return "C";
    else if (width <= 16) return "S";
    else if (width <= 32) return "I";
    else if (width <= 64) return "Q";
    else return "W";
}
static string genItfCode(const string type, YosysSignal* signal) {
    string signalName = signal->getName();
    if (type == "reg") signalName = yosysMgr->getTopModuleName() + "__DOT__" + signalName;
    string line = "\t    " + type + ".push_back(new Signal" + signalWidth2Type(signal->getWidth()) +
                  "(\"" + signalName + "\", &rootp->" + signalName + ", " +
                  to_string(signal->getWidth()) + "));";
    return line;
}

static string genMacro(string macroName, int val) {
    return " " + macroName + "=" + to_string(val);
}

static string genMacro(string macroName, string val) {
    return " " + macroName + "=" + val;
}

/**
 * @brief Construct a new VRLTMgr::VRLTMgr object
 *
 */
VRLTMgr::VRLTMgr() : SimMgr(0) {
    _itfFileName = "interface.hpp";
    _itfPath     = string(GV_TEMPLATE_PATH) + _itfFileName;
    _dirPath     = string(GV_VERILATOR_PATH);
    _make        = "make";
    _macro       = genMacro("DESIGN", cirMgr->getFileName()) + genMacro("GV_PATH", GV_PATH);
    _verboseCmd  = " > /dev/null 2>&1";
};

/**
 * @brief
 *
 */
bool VRLTMgr::preVrltSim(const bool& verbose) {
    if (!preDesignInfo(verbose)) {
        cout << "ERROR: Cannot prepare design info for Verilator !!\n";
        return false;
    }
    if (!genVrltBuild(verbose)) {
        cout << "ERROR: Cannot generate the verilator/build/ directory !!\n";
        return false;
    }
    if (!genVrltItf(verbose)) {
        cout << "ERROR: Cannot generate the interface file !!\n";
        return false;
    }
    if (!genVrltMakefile(verbose)) {
        cout << "ERROR: Cannot generate the verilator Makefile !!\n";
        return false;
    }
    return true;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool VRLTMgr::preDesignInfo(const bool&) {
    _macro += genMacro("CYCLE", getSimCycle());
    return true;
}

/**
 * @brief
 *
 * @param verbose
 * @return true
 * @return false
 */
bool VRLTMgr::genVrltBuild(const bool& verbose) {
    namespace fs       = std::experimental::filesystem;
    fs::path currPath  = fs::current_path();
    fs::path buildPath = GV_VERILATOR_BUILD_PATH;
    string rmCmd       = "make create_build_dir";
    if (!verbose) rmCmd += _verboseCmd;

    fs::current_path(GV_VERILATOR_PATH);
    cout << GREEN_TEXT << " >>> SYSTEM COMMAND: " << rmCmd << RESET_COLOR << endl;
    if (system(rmCmd.c_str()) != 0) {
        cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

/**
 * @brief
 *
 */
bool VRLTMgr::genVrltItf(const bool& verbose) {
    string outFileName = _dirPath + _itfFileName, tmpLine;
    ifstream infile(_itfPath, ios::in);
    ofstream outfile(outFileName, ios::out);
    if (!infile) {
        cout << "Cannot open design \"" << outFileName << "\"!!" << endl;
        return false;
    }
    while (infile) {
        getline(infile, tmpLine);
        outfile << tmpLine << endl;
        if (tmpLine.find("CLK") != string::npos) {
            for (int i = 0; i < yosysMgr->getNumCLKs(); ++i)
                outfile << genItfCode("clk", yosysMgr->getClk(0)) << endl;
        } else if (tmpLine.find("RST") != string::npos) {
            for (int i = 0; i < yosysMgr->getNumRSTs(); ++i)
                outfile << genItfCode("rst", yosysMgr->getRst(i)) << endl;
        } else if (tmpLine.find("PI") != string::npos) {
            for (int i = 0; i < yosysMgr->getNumPIs(); ++i)
                outfile << genItfCode("pi", yosysMgr->getPi(i)) << endl;
        } else if (tmpLine.find("PO") != string::npos) {
            for (int i = 0; i < yosysMgr->getNumPOs(); ++i)
                outfile << genItfCode("po", yosysMgr->getPo(i)) << endl;
        } else if (tmpLine.find("REG") != string::npos) {
            for (int i = 0; i < yosysMgr->getNumREGs(); ++i) {
                string regName = yosysMgr->getTopModuleName() + "__DOT__";
                outfile << genItfCode("reg", yosysMgr->getReg(i)) << endl;
            }
        }
    }
    infile.close();
    outfile.close();
    return true;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool VRLTMgr::genVrltMakefile(const bool& verbose) {
    namespace fs      = std::experimental::filesystem;
    fs::path currPath = fs::current_path();
    string execCmd    = _make + " " + _macro;

    if (!verbose)
        execCmd += _verboseCmd;

    cout << GREEN_TEXT << " >>> SYSTEM COMMAND: " << execCmd << RESET_COLOR << endl;
    fs::current_path(GV_VERILATOR_PATH);
    if (system(execCmd.c_str()) != 0) {
        cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool VRLTMgr::runVrltSim(const bool& verbose) {
    namespace fs      = std::experimental::filesystem;
    fs::path currPath = fs::current_path();
    string runCmd     = " run";
    string execCmd    = _make + runCmd;

    if (!verbose)
        execCmd += _verboseCmd;

    cout << GREEN_TEXT << " >>> SYSTEM COMMAND: " << execCmd << RESET_COLOR << endl;
    fs::current_path(GV_VERILATOR_PATH);
    if (system(execCmd.c_str()) != 0) {
        cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

void VRLTMgr::enableRandomSim() {
    _macro += genMacro("MODE", 0);
}

void VRLTMgr::enableFileSim() {
    _macro += genMacro("MODE", 1);
}

/**
 * @brief
 *
 */
void VRLTMgr::fileSim(const bool& verbose) {
    enableFileSim();
    preVrltSim(verbose);
    runVrltSim(verbose);
}

/**
 * @brief
 *
 */
void VRLTMgr::randomSim(const bool& verbose) {
    enableRandomSim();
    preVrltSim(verbose);
    runVrltSim(verbose);
}