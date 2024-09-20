#include "cxxMgr.h"

#include <cstddef>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "cirMgr.h"
#include "fmt/color.h"
#include "fmt/core.h"
#include "simMgr.h"
#include "util.h"

gv::sim::CXXMgr* cxxMgr = nullptr;

static string genItfFuncCode(const unsigned& bitWidth) {
    return fmt::format("{0:<4}SET_VALUE_FUNC({1},{2});\n", "", bitWidth, "unsigned");
}
static string genItfCondCode(const unsigned& bitWidth) {
    return fmt::format("{0:<8}SET_WIDTH_CONDITION({1});\n", "", bitWidth);
}
static string genItfSignalCode(const string& signaType, const string& signalName) {
    return fmt::format("{0:<8}SET_CXX_SIGNAL({1}, \"{2}\");\n", "", signaType, signalName);
}
static string genItfTopObjCode(const string& topModule) {
    return fmt::format("{0:<4}SET_CXX_TOP({1});\n", "", topModule);
}
static string genMacro(const string& macro, const string& value) {
    return fmt::format("{0}={1} ", macro, value);
}
static string genMacro(const string& macro, const size_t& value) {
    return fmt::format("{0}={1} ", macro, value);
}

namespace gv {
namespace sim {

CXXMgr::CXXMgr() : SimMgr(0) {
    _cxxDir     = PATH(std::string(GV_CXXRTL_PATH)).parent_path();
    _itfTmpFile = _cxxDir / "template" / "interface.hpp";
    _itfFile    = _cxxDir / "src" / "interface.hpp";
    _cxxSimFile = _cxxDir / "src" / ".sim.cpp";
}

bool CXXMgr::preCXXSim(const bool& verbose) {
    if (!genMakeMacro(verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the macro !!\n");
        return false;
    }
    // generate the driver file by calling the Yosys CXXRTL command
    if (!genCXXDriver(verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the CXXRTL driver file !!\n");
        return false;
    }
    // generate the interface code
    if (!genCXXItf(verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the interface file !!\n");
        return false;
    }
    // generate the simulation execution file
    if (!genCXXExe(verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the CXXRTL driver file !!\n");
        return false;
    }
    return true;
}

bool CXXMgr::genMakeMacro(const bool& verbose) {
    namespace fs  = std::experimental::filesystem;
    PATH currPath = fs::current_path();
    _macro += genMacro("GV_PATH", currPath);
    _macro += genMacro("CYCLE", getSimCycle());
    _macro += genMacro("VCD_FILE", currPath / getVcdFileName());
    return true;
}

bool CXXMgr::genCXXItf(const bool& verbose) {
    std::ifstream infile(_itfTmpFile, std::ios::in);
    std::ofstream outfile(_itfFile, std::ios::out);
    std::vector<std::string> condCode;
    std::string tmpLine;
    unordered_set<int> uniqBit;
    YosysMgr* curYsyMgr = cirMgr->getYosysMgr();

    if (!infile) {
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                   "Cannot open design {0} !!", _itfFile.string());
        return false;
    }
    while (infile) {
        getline(infile, tmpLine);
        outfile << tmpLine << std::endl;
        if (tmpLine.find("Set signal value functions") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumPIs(); ++i) {
                unsigned bitWidth = curYsyMgr->getPi(i)->getWidth();
                if (bitWidth != 1 && !uniqBit.count(bitWidth)) {
                    uniqBit.insert(bitWidth);
                    outfile << genItfFuncCode(bitWidth);
                    condCode.emplace_back(genItfCondCode(bitWidth));
                }
            }
        } else if (tmpLine.find("Set N bits value condition") != std::string::npos) {
            for (int i = 0, n = condCode.size(); i < n; ++i)
                outfile << condCode[i];
        } else if (tmpLine.find("Set top design object") != std::string::npos) {
            outfile << genItfTopObjCode(curYsyMgr->getTopModuleName());
        } else if (tmpLine.find("CLK") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumCLKs(); ++i)
                outfile << genItfSignalCode("clk", curYsyMgr->getClk(i)->getName());
        } else if (tmpLine.find("RST") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumRSTs(); ++i)
                outfile << genItfSignalCode("rst", curYsyMgr->getRst(i)->getName());
        } else if (tmpLine.find("PI") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumPIs(); ++i)
                outfile << genItfSignalCode("pi", curYsyMgr->getPi(i)->getName());
        } else if (tmpLine.find("PO") != std::string::npos) {
            for (int i = 0; i < curYsyMgr->getNumPOs(); ++i)
                outfile << genItfSignalCode("po", curYsyMgr->getPo(i)->getName());
        }
        // } else if (tmpLine.find("REG") != std::string::npos) {
        //     for (int i = 0; i < yosysMgr->getNumREGs(); ++i) {
        //         std::string regName = yosysMgr->getTopModuleName() + "__DOT__";
        //         // std::cout << genItfSignalCode("rst", yosysMgr->getRst(i)->getName());
        //         // outfile << genItfCode("reg", yosysMgr->getReg(i)) << endl;
        //     }
        // }
    }
    infile.close();
    outfile.close();
    return true;
}

bool CXXMgr::genCXXDriver(const bool& verbose) {
    YosysMgr* curYsyMgr = cirMgr->getYosysMgr();
    // Call the Yosys command to write the driver.cpp for simulation
    string writeCXXCmd = "write_cxxrtl " + _cxxSimFile.string();
    curYsyMgr->runPass(writeCXXCmd);
    return true;
}

bool CXXMgr::genCXXExe(const bool& verbose) {
    string compileCmd = fmt::format("make -C {0} {1}", _cxxDir.string(), _macro);
    // if (system(compileCmd.c_str()) != 0) {
    if (!systemCmd(compileCmd.c_str(), verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot execute the system command \"{0}\" !!\n", compileCmd);
        return false;
    }
    return true;
}

bool CXXMgr::runCXXSim(const bool& verbose) {
    string runCmd = fmt::format("make -C {0} run", _cxxDir.string());
    if (!systemCmd(runCmd.c_str(), verbose)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot execute the system command \"{0}\" !!\n", runCmd);
        return false;
    }
    return true;
}

/**
 * @brief Add the MODE=1 and PATTERN_FILE macro
 *
 */
void CXXMgr::enableFileSim() {
    _macro += genMacro("MODE", "1");
    _macro += genMacro("PATTERN_FILE", getPatternFileName());
}

/**
 * @brief Add the MODE=0
 *
 */
void CXXMgr::enableRandomSim() {
    _macro += genMacro("MODE", "0");
}

void CXXMgr::fileSim(const bool& verbose) {
    enableFileSim();
    if (preCXXSim(verbose))
        runCXXSim(verbose);
}

void CXXMgr::randomSim(const bool& verbose) {
    enableRandomSim();
    if (preCXXSim(verbose))
        runCXXSim(verbose);
}

}  // namespace sim
}  // namespace gv
