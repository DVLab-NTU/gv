#include "vrltMgr.h"

#include <cstddef>
#include <experimental/filesystem>
#include <string>

#include "cirMgr.h"
#include "fmt/color.h"
#include "fstream"
#include "simMgr.h"

VRLTMgr* vrltMgr = nullptr;

static std::string signalWidth2Type(const int& width) {
    if (width <= 8) return "C";
    else if (width <= 16) return "S";
    else if (width <= 32) return "I";
    else if (width <= 64) return "Q";
    else return "W";
}
static std::string genItfCode(const std::string type, YosysSignal* signal) {
    std::string signalName = signal->getName();
    if (type == "reg") signalName = cirMgr->getYosysMgr()->getTopModuleName() + "__DOT__" + signalName;
    std::string line = "\t    " + type + ".push_back(new Signal" + signalWidth2Type(signal->getWidth()) +
                       "(\"" + signalName + "\", &rootp->" + signalName + ", " + to_string(signal->getWidth()) + "));";
    return line;
}
static std::string genMacro(std::string macroName, int val) {
    return " " + macroName + "=" + std::to_string(val);
}

static std::string genMacro(std::string macroName, std::string val) {
    return " " + macroName + "=" + val;
}

/**
 * @brief Construct a new VRLTMgr::VRLTMgr object
 *
 */
VRLTMgr::VRLTMgr() : SimMgr(0) {
    _itfFileName = "interface.hpp";
    _itfPath     = std::string(GV_TEMPLATE_PATH) + _itfFileName;
    _dirPath     = std::string(GV_VERILATOR_PATH);
    _make        = "make";
    _macro       = "";
    _verboseCmd  = " > /dev/null 2>&1";
};

/**
 * @brief Prepare for simulation with Verilator
 *
 */
bool VRLTMgr::preVrltSim(const bool& verbose) {
    if (!preDesignInfo(verbose)) {
        std::cout << "ERROR: Cannot prepare design info for Verilator !!\n";
        return false;
    }
    if (!genVrltBuild(verbose)) {
        std::cout << "ERROR: Cannot generate the verilator/build/ directory !!\n";
        return false;
    }
    if (!genVrltItf(verbose)) {
        std::cout << "ERROR: Cannot generate the interface file !!\n";
        return false;
    }
    if (!genVrltMakefile(verbose)) {
        // std::cout << "ERROR: Cannot generate the verilator Makefile !!\n";
        std::cout << "ERROR: Cannot generate the verilator Makefile !!\n";
        fmt::println("ERROR: Cannot generate the verilator Makefile !!");
        return false;
    }
    return true;
}

/**
 * @brief Define the design information macro
 *
 * @return true
 * @return false
 */
bool VRLTMgr::preDesignInfo(const bool&) {
    namespace fs      = std::experimental::filesystem;
    fs::path currPath = fs::current_path();
    _macro += genMacro("DESIGN", cirMgr->getFileName());
    _macro += genMacro("GV_PATH", fs::current_path().string());
    _macro += genMacro("CYCLE", getSimCycle());
    _macro += genMacro("VCD_FILE", getVcdFileName());
    return true;
}

/**
 * @brief Generate the build/ folder to save the obj files generated by verilator
 *
 * @param verbose
 * @return true
 * @return false
 */
bool VRLTMgr::genVrltBuild(const bool& verbose) {
    namespace fs       = std::experimental::filesystem;
    fs::path currPath  = fs::current_path();
    fs::path buildPath = GV_VERILATOR_BUILD_PATH;
    std::string rmCmd  = "make create_build_dir";
    if (!verbose) rmCmd += _verboseCmd;

    fs::current_path(GV_VERILATOR_PATH);
    if (system(rmCmd.c_str()) != 0) {
        std::cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

/**
 * @brief Generate the verilator wrapper code to the interface.hpp to access the internal signals in the DUV
 *
 * @param verbose
 * @return true
 * @return false
 */
bool VRLTMgr::genVrltItf(const bool& verbose) {
    std::string outFileName = _dirPath + _itfFileName, tmpLine;
    std::ifstream infile(_itfPath, std::ios::in);
    std::ofstream outfile(outFileName, std::ios::out);
    YosysMgr* curYsyMgr = cirMgr->getYosysMgr();

    if (!infile) {
        std::cout << "Cannot open design \"" << outFileName << "\"!!" << endl;
        return false;
    }
    while (infile) {
        getline(infile, tmpLine);
        outfile << tmpLine << endl;
        if (tmpLine.find("CLK") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumCLKs(); ++i)
                outfile << genItfCode("clk", curYsyMgr->getClk(0)) << endl;
        } else if (tmpLine.find("RST") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumRSTs(); ++i)
                outfile << genItfCode("rst", curYsyMgr->getRst(i)) << endl;
        } else if (tmpLine.find("PI") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumPIs(); ++i)
                outfile << genItfCode("pi", curYsyMgr->getPi(i)) << endl;
        } else if (tmpLine.find("PO") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumPOs(); ++i)
                outfile << genItfCode("po", curYsyMgr->getPo(i)) << endl;
        } else if (tmpLine.find("REG") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumREGs(); ++i) {
                std::string regName = curYsyMgr->getTopModuleName() + "__DOT__";
                outfile << genItfCode("reg", curYsyMgr->getReg(i)) << endl;
            }
        }
    }
    infile.close();
    outfile.close();
    return true;
}

/**
 * @brief Generate the Verilator Makefile by calling "make" with the predefined macro
 *
 * @param verbose
 * @return true
 * @return false
 */
bool VRLTMgr::genVrltMakefile(const bool& verbose) {
    namespace fs        = std::experimental::filesystem;
    fs::path currPath   = fs::current_path();
    std::string execCmd = _make + " " + _macro;

    if (!verbose)
        execCmd += _verboseCmd;

    fs::current_path(GV_VERILATOR_PATH);
    if (system(execCmd.c_str()) != 0) {
        std::cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

/**
 * @brief Run the execution file generate by verilator by calling "make run"
 *
 * @param verbose
 * @return true
 * @return false
 */
bool VRLTMgr::runVrltSim(const bool& verbose) {
    namespace fs        = std::experimental::filesystem;
    fs::path currPath   = fs::current_path();
    std::string runCmd  = " run";
    std::string execCmd = _make + runCmd;

    if (!verbose)
        execCmd += _verboseCmd;

    fs::current_path(GV_VERILATOR_PATH);
    if (system(execCmd.c_str()) != 0) {
        std::cout << "ERROR: Cannot execute the system command !!\n";
        return false;
    }
    fs::current_path(currPath);

    return true;
}

/**
 * @brief Add the MODE=0 macro
 *
 */
void VRLTMgr::enableRandomSim() {
    _macro += genMacro("MODE", 0);
}

/**
 * @brief Add the MODE=1 and PATTERN_FILE macro
 *
 */
void VRLTMgr::enableFileSim() {
    _macro += genMacro("MODE", 1);
    _macro += genMacro("PATTERN_FILE", getPatternFileName());
}

/**
 * @brief Run the simulation with the input pattern file
 *
 * @param verbose
 */
void VRLTMgr::fileSim(const bool& verbose) {
    enableFileSim();
    if (preVrltSim(verbose))
        runVrltSim(verbose);
}

/**
 * @brief Run the random simulation
 *
 * @param verbose
 */
void VRLTMgr::randomSim(const bool& verbose) {
    enableRandomSim();
    if (preVrltSim(verbose))
        runVrltSim(verbose);
}