//
//  simulator.hpp
//  crv_01
//  dhgir.abien@gmail.com
//

#ifndef simulator_h
#define simulator_h

#include <backends/cxxrtl/cxxrtl_vcd.h>

#include <cstddef>
#include <fstream>
#include <iostream>

#include ".sim.cpp"
#include "backends/cxxrtl/cxxrtl.h"
#include "interface.hpp"
#include "utility.hpp"

#define GREEN_TEXT "\033[32m"  // ANSI escape code for green color
#define RESET_COLOR "\033[0m"  // ANSI escape code to reset color

class Simulator {
    typedef std::vector<std::vector<unsigned>> Pattern;  // Input pattern

public:
    Simulator(int c) : cycle(c), sampleCount(0), isVCD(true) { init(); }

    bool init() {
        top.debug_info(all_debug_items);
        vcd.timescale(1, "ps");
        vcd.add_without_memories(all_debug_items);
        vcd.add(all_debug_items);
        waves.open("cxxrtl.vcd");
        if (!waves.is_open()) {
            return false;
            std::cout << " Open the CXXRTL file failed !!\n";
        }
        itf = new Interface(&all_debug_items);
        return true;
    }

    bool finish() {
        waves.close();
        return true;
    }

    void resetNeg() {
        vcd.sample(sampleCount++);
        top.step();
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
    void setCLK(const bool &val) { itf->clk[0].setValue(val); }
    void setRST(const bool &val) { itf->rst[0].setValue(val); }
    void setPiPattern(std::vector<unsigned> &piPattern) {
        for (unsigned i = 0; i < piPattern.size(); i++) {
            itf->pi[i].setValue(piPattern[i]);
            std::cout << " PI NAME: " << itf->pi[i].name << " Value: " << piPattern[i] << "\n";
        };
    }

    //! getter functions
    unsigned getPiNum() { return itf->pi.size(); }
    unsigned getPoNum() { return itf->po.size(); }
    unsigned getPiUpper(const unsigned &idx) { return itf->pi[idx].getUpper(); }
    void setRandVal() {
        top.p_b.set<unsigned>(rGen.getRandNum(0, 16));
        top.p_a.set<unsigned>(rGen.getRandNum(0, 16));
    }
    void eval() {
        top.step();
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

    bool loadRandomPattern() {
        for (int i = 0; i < cycle; ++i) {
            std::vector<unsigned> onePattern = genPiRandomPattern();
            patternVec.push_back(onePattern);
        }
        return true;
    }

    // void dump_all_items() {
    //     std::cout << "All items: \n";
    //     for (auto &it : all_debug_items.table) {
    //         for (auto &part : it.second) {
    //             if (it.first == "a" || it.first == "b") {
    //                 set4BitValue(part);
    //             }
    //             std::cout << it.first
    //                       << ": type = " << part.type
    //                       << ", width = " << part.width
    //                       << ", depth = " << part.depth
    //                       << ", lsb_at = " << part.lsb_at
    //                       << ", value = " << *(part.next)
    //                       << ", zero_at = " << part.zero_at << std::endl;
    //         }
    //     }
    //     std::cout << std::endl;
    // }

    void startSim(const bool &verbose) {
        int simCycle = patternVec.size();
        std::cout << "LOAD RANDOM PATTERN: " << simCycle << "\n";
        if (itf->rst.size() > 0)
            resetNeg();
        for (int i = 0; i < simCycle; ++i) {
            setPiPattern(patternVec[i]);
            evalOneClock();
            if (verbose) printValue();
        }
    }

    size_t cycle;
    size_t sampleCount;
    cxxrtl_design::p_adder top;
    cxxrtl::debug_items all_debug_items;
    cxxrtl::vcd_writer vcd;
    std::ofstream waves;
    Pattern patternVec;
    Interface *itf;
    bool isVCD;
};

#endif /* simulator_h */
