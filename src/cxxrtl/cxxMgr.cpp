#include "cxxMgr.h"

#include <cstddef>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "fmt/color.h"
#include "fmt/core.h"
#include "simMgr.h"
#include "yosysMgr.h"

CXXMgr* cxxMgr = nullptr;

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

CXXMgr::CXXMgr() : SimMgr(0) {
    _cxxDir = PATH(std::string(GV_CXXRTL_PATH)).parent_path();
    _itfTmpFile = _cxxDir / "template/interface.hpp";
    _itfFile = _cxxDir / "interface.hpp";
    _cxxSimFile = _cxxDir / ".sim.cpp";
}

bool CXXMgr::preCXXSim(const bool& verbose) {
    if (!genMakeMacro(true)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the macro !!\n");
        return false;
    }
    // generate the driver file by calling the Yosys CXXRTL command
    if (!genCXXDriver(true)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the CXXRTL driver file !!\n");
        return false;
    }
    // generate the interface code
    if (!genCXXItf(true)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the interface file !!\n");
        return false;
    }
    // generate the simulation execution file
    if (!genCXXExe(true)) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot generate the CXXRTL driver file !!\n");
        return false;
    }
    return true;
}

bool CXXMgr::genMakeMacro(const bool& verbose) {
    namespace fs = std::experimental::filesystem;
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

    if (!infile) {
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                   "Cannot open design {0} !!", _itfFile.string());
        return false;
    }
    while (infile) {
        getline(infile, tmpLine);
        outfile << tmpLine << std::endl;
        if (tmpLine.find("Set signal value functions") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumPIs(); ++i) {
                unsigned bitWidth = yosysMgr->getPi(i)->getWidth();
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
            outfile << genItfTopObjCode(yosysMgr->getTopModuleName());
        } else if (tmpLine.find("CLK") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumCLKs(); ++i)
                outfile << genItfSignalCode("clk", yosysMgr->getClk(i)->getName());
        } else if (tmpLine.find("RST") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumRSTs(); ++i)
                outfile << genItfSignalCode("rst", yosysMgr->getRst(i)->getName());
        } else if (tmpLine.find("PI") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumPIs(); ++i)
                outfile << genItfSignalCode("pi", yosysMgr->getPi(i)->getName());
        } else if (tmpLine.find("PO") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumPOs(); ++i)
                outfile << genItfSignalCode("po", yosysMgr->getPo(i)->getName());
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
    // Call the Yosys command to write the driver.cpp for simulation
    string writeCXXCmd = "write_cxxrtl " + _cxxSimFile.string();
    yosysMgr->runPass(writeCXXCmd);
    return true;
}

bool CXXMgr::genCXXExe(const bool& verbose) {
    string compileCmd = fmt::format("make -C {0} {1}", _cxxDir.string(), _macro);
    if (system(compileCmd.c_str()) != 0) {
        fmt::print(fmt::fg(fmt::color::red),
                   "ERROR: Cannot execute the system command \"{0}\" !!\n", compileCmd);
        return false;
    }
    return true;
}

bool CXXMgr::runCXXSim(const bool& verbose) {
    string runCmd = fmt::format("make -C {0} run", _cxxDir.string());
    if (system(runCmd.c_str()) != 0) {
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

void CXXMgr::fileSim(const bool& verbose) {
    enableFileSim();
    if (preCXXSim(verbose))
        runCXXSim(verbose);
}

void CXXMgr::randomSim(const bool& verbose) {
    if (preCXXSim(verbose))
        runCXXSim(verbose);
}