#include "vrltMgr.h"

#include <experimental/filesystem>

#include "cirMgr.h"
#include "fstream"
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

/**
 * @brief Construct a new VRLTMgr::VRLTMgr object
 *
 */
VRLTMgr::VRLTMgr() {
    _itfFileName = "interface.hpp";
    _itfPath     = string(GV_TEMPLATE_PATH) + _itfFileName;
    _dirPath     = string(GV_VERILATOR_PATH);
};

/**
 * @brief
 *
 */
bool VRLTMgr::preVrltSim(const bool& verbose) {
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

bool VRLTMgr::genVrltBuild(const bool& verbose) {
    namespace fs       = std::experimental::filesystem;
    fs::path currPath  = fs::current_path();
    fs::path buildPath = GV_VERILATOR_BUILD_PATH;
    string rmCmd       = "make create_build_dir > /dev/null 2>&1";

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
    string outFileName = _dirPath + _itfFileName;
    ifstream infile(_itfPath, ios::in);
    ofstream outfile(outFileName, ios::out);
    if (!infile) {
        cout << "Cannot open design \"" << outFileName << "\"!!" << endl;
        return false;
    }
    while (infile) {
        string tmpLine;
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
    string macro      = "DESIGN=" + cirMgr->getFileName() + " GV_PATH=" + GV_PATH;
    string compileCmd = "make " + macro + " > /dev/null 2>&1";
    string verboseCmd = " > /dev/null 2>&1";

    if (!verbose) compileCmd += verboseCmd;

    cout << GREEN_TEXT << " >>> SYSTEM COMMAND: " << compileCmd << RESET_COLOR << endl;
    fs::current_path(GV_VERILATOR_PATH);
    if (system(compileCmd.c_str()) != 0) {
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
    string runCmd     = "make run";
    string verboseCmd = " > /dev/null 2>&1";

    if (!verbose) runCmd += verboseCmd;

    cout << GREEN_TEXT << " >>> SYSTEM COMMAND: " << runCmd << RESET_COLOR << endl;
    fs::current_path(GV_VERILATOR_PATH);
    if (system(runCmd.c_str()) != 0) {
        cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}