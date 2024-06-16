#include "cxxMgr.h"

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "fmt/color.h"
#include "fmt/core.h"
#include "yosysMgr.h"

static string genItfFuncCode(const unsigned& bitWidth) {
    return fmt::format("{0:<4}SET_VALUE_FUNC({1},{2});\n", "", bitWidth, "unsigned");
}

static string genItfCondCode(const unsigned& bitWidth) {
    return fmt::format("{0:<8}SET_WIDTH_CONDITION({1});\n", "", bitWidth);
}

static string genItfSignalCode(const string& signaType, const string& signalName) {
    return fmt::format("{0:<8}SET_CXX_SIGNAL({1}, \"{2}\");\n", "", signaType, signalName);
}

CXXMgr::CXXMgr() {
    _itfTmpFile = PATH(std::string(GV_CXXRTL_PATH)) / "template/interface.hpp";
    _itfFile = _itfTmpFile.parent_path().parent_path() / "interface.hpp";
}

bool CXXMgr::preCXXSim(const bool& verbose) {
    // generate the interface code
    genCXXItf(true);
}

bool CXXMgr::genCXXItf(const bool& verbose) {
    std::ifstream infile(_itfTmpFile, std::ios::in);
    std::vector<std::string> condCode;
    unordered_set<int> uniqBit;
    std::ofstream outfile(_itfFile, std::ios::out);
    std::string tmpLine;
    if (!infile) {
        fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Cannot open design {0} !!",
                   _itfFile.string());
        return false;
    }
    while (infile) {
        getline(infile, tmpLine);
        // outfile << tmpLine;
        // std::cout << tmpLine << std::endl;
        outfile << tmpLine << std::endl;
        if (tmpLine.find("Set signal value functions") != std::string::npos) {
            // #define SET_CXX_VALUE(bitwidth, type)
            for (int i = 0; i < yosysMgr->getNumPIs(); ++i) {
                unsigned bitWidth = yosysMgr->getPi(i)->getWidth();
                if (!uniqBit.count(bitWidth)) {
                    uniqBit.insert(bitWidth);
                    // std::cout << genItfFuncCode(bitWidth);
                    outfile << genItfFuncCode(bitWidth);
                    condCode.emplace_back(genItfCondCode(bitWidth));
                }
            }
        } else if (tmpLine.find("Set N bits value condition") != std::string::npos) {
            for (int i = 0, n = condCode.size(); i < n; ++i)
                outfile << condCode[i];
            // std::cout << condCode[i];
        } else if (tmpLine.find("CLK") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumCLKs(); ++i) {
                outfile << genItfSignalCode("clk", yosysMgr->getClk(i)->getName());
                // std::cout << genItfSignalCode("clk", yosysMgr->getClk(i)->getName());
            }
        } else if (tmpLine.find("RST") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumRSTs(); ++i)
                outfile << genItfSignalCode("rst", yosysMgr->getRst(i)->getName());
            // std::cout << genItfSignalCode("rst", yosysMgr->getRst(i)->getName());
        } else if (tmpLine.find("PI") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumPIs(); ++i)
                outfile << genItfSignalCode("pi", yosysMgr->getPi(i)->getName());
            // std::cout << genItfSignalCode("pi", yosysMgr->getPi(i)->getName());
        } else if (tmpLine.find("PO") != std::string::npos) {
            for (int i = 0; i < yosysMgr->getNumPOs(); ++i)
                outfile << genItfSignalCode("po", yosysMgr->getPo(i)->getName());
            // std::cout << genItfSignalCode("po", yosysMgr->getPo(i)->getName());
        }
        // } else if (tmpLine.find("REG") != std::string::npos) {
        //     for (int i = 0; i < yosysMgr->getNumREGs(); ++i) {
        //         std::string regName = yosysMgr->getTopModuleName() + "__DOT__";
        //         std::cout << genItfSignalCode("rst", yosysMgr->getRst(i)->getName());
        //         outfile << genItfCode("reg", yosysMgr->getReg(i)) << endl;
        //     }
        // }
    }
    infile.close();
    // outfile.close();
    return true;
}

void CXXMgr::fileSim(const bool&) {}
void CXXMgr::randomSim(const bool&) {
    preCXXSim(true);
}