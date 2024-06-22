//
//  simulator.hpp
//  crv_01
//  dhgir.abien@gmail.com
//

#ifndef simulator_h
#define simulator_h

#include <cstddef>
#include <fstream>
#include <iostream>

#include "backends/cxxrtl/cxxrtl.h"
#include "backends/cxxrtl/cxxrtl_vcd.h"
#include "interface.hpp"
#include "utility.hpp"

#define GREEN_TEXT "\033[32m"  // ANSI escape code for green color
#define RESET_COLOR "\033[0m"  // ANSI escape code to reset color

#define STR(s) STR2(s)
#define STR2(s) #s

class Simulator {
    typedef std::vector<std::vector<unsigned>> Pattern;  // Input pattern

public:
    Simulator(int c) : cycle(c), sampleCount(0), isVCD(true) { init(); }

    bool init() {
        itf = new Interface(&all_debug_items);

        vcd.timescale(1, "ps");
        vcd.add_without_memories(all_debug_items);
        vcd.add(all_debug_items);

        patternFile = STR(PATTERN_FILE);
        vcdFile = STR(VCD_FILE);
        return true;
    }

    bool enableVCD() {
        waves.open(vcdFile);
        if (!waves.is_open()) {
            return false;
            std::cout << " Open the CXXRTL file failed !!\n";
        }
        return true;
    }

    bool finish() {
        waves.close();
        return true;
    }

    void resetNeg() {
        vcd.sample(sampleCount++);
        itf->step();

        //          __    __
        // clk:  __|  |__|  |__
        //                   __
        // rst:  ___________|  |

        // RESET
        setCLK(false);
        setRST(false);
        eval();

        setCLK(true);
        setRST(false);
        eval();

        setCLK(false);
        setRST(false);
        eval();

        setCLK(true);
        setRST(false);
        eval();

        setCLK(false);
        setRST(true);
        eval();
    }

    //! setter functions
    void setCLK(const bool &value) { itf->clk[0].setValue(value); }
    void setRST(const bool &value) { itf->rst[0].setValue(value); }
    void setPiPattern(std::vector<unsigned> &piPattern) {
        for (unsigned i = 0; i < piPattern.size(); i++) {
            itf->pi[i].setValue(piPattern[i]);
        };
    }

    //! getter functions
    unsigned getPiNum() { return itf->pi.size(); }
    unsigned getPoNum() { return itf->po.size(); }
    unsigned getPiUpper(const unsigned &idx) { return itf->pi[idx].getUpper(); }

    void eval() {
        itf->step();
        if (isVCD) {
            vcd.sample(sampleCount++);
            outVCD();
        }
    }

    void evalOneClock(void) {
        //        __
        // clk : |  |___
        setCLK(1);
        eval();
        setCLK(0);
        eval();
    }

    void outVCD() {
        waves << vcd.buffer;
        vcd.buffer.clear();
    }

    void printValue() {
        uint32_t sum = itf->po[0].getValue();
        std::cout << "==========================================\n";
        std::cout << "= cycle " << cycle + 1 << "\n";
        std::cout << "==========================================\n";
        std::cout << "sum= " << sum << "\n";
        std::cout << "clk = " << itf->clk[0].getValue() << "\n";
        std::cout << "rst= " << itf->rst[0].getValue() << "\n";
        std::cout << itf->pi[0].getName() << "= " << itf->pi[0].getValue() << "\n";
        std::cout << itf->pi[1].getName() << "= " << itf->pi[1].getValue() << "\n";
        std::cout << std::endl;
    }

    std::vector<unsigned> genPiRandomPattern() {
        std::vector<unsigned> pattern;
        pattern.clear();
        for (unsigned i = 0; i < getPiNum(); i++) {
            pattern.emplace(pattern.end(),
                            rGen.getRandNum(0, getPiUpper(i)));
        }
        return pattern;
    }

    std::string replaceXValue(const std::string &pattern) {
        std::string newPattern = pattern;
        size_t xPos = newPattern.find('X');
        while (xPos != std::string::npos) {
            newPattern[xPos] = '0';
            xPos = newPattern.find('X');
        }
        return newPattern;
    }

    bool loadRandomPattern() {
        for (int i = 0; i < cycle; ++i) {
            std::vector<unsigned> onePattern = genPiRandomPattern();
            patternVec.emplace_back(onePattern);
        }
        return true;
    }

    bool loadInputPattern() {
        std::ifstream infile(patternFile);
        std::string buffer = "", inputStr = "";
        unsigned width = 0, inputVal = 0;

        if (!infile) {
            std::cout << "ERROR: Cannot open file " << patternFile << " !!\n";
            return false;
        }

        while (infile >> buffer) {
            std::vector<unsigned> onePattern;
            for (int i = 0, n = getPiNum(); i < n; ++i) {
                width = itf->pi[i].getWidth();
                inputStr = replaceXValue(buffer.substr(0, width));
                if (inputStr.size() < width) {
                    std::cout << "ERROR: Illegal input pattern file !!\n";
                    return false;
                }
                inputVal = stoi(inputStr, nullptr, 2);
                onePattern.emplace_back(inputVal);
                buffer = buffer.substr(width);
            }
            if (buffer.size() != 0) {
                std::cout << "ERROR: Illegal input pattern file !!\n";
                return false;
            }
            patternVec.push_back(onePattern);
        }
        infile.close();
        return true;
    }

    void startSim(const bool &verbose) {
        int simCycle = patternVec.size();
        if (itf->rst.size() > 0)
            resetNeg();
        for (int i = 0; i < simCycle; ++i) {
            setPiPattern(patternVec[i]);
            evalOneClock();
            if (verbose) printValue();
        }
    }

    bool isVCD;
    size_t cycle;
    size_t sampleCount;
    cxxrtl::debug_items all_debug_items;
    cxxrtl::vcd_writer vcd;
    std::string patternFile;
    std::string vcdFile;
    std::ofstream waves;
    Pattern patternVec;
    Interface *itf;
};

#endif /* simulator_h */
