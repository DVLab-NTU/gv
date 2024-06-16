#include <backends/cxxrtl/cxxrtl_vcd.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <vector>

// #include ".sim.cpp"
#include "backends/cxxrtl/cxxrtl_capi.h"
#include "simulator.hpp"
#include "utility.hpp"

int main() {
    srand(time(NULL));
    unsigned random_value = 0;
    unsigned upper_bound = 0;
    char buffer[100];
    unsigned count = 0;
    // Simulator* simulator = new Simulator();
    // simulator->init();

    cxxrtl_design::p_adder top;
    cxxrtl::debug_items all_debug_items;
    cxxrtl::vcd_writer vcd;
    top.debug_info(all_debug_items);
    vcd.timescale(1, "ps");
    vcd.add_without_memories(all_debug_items);
    vcd.add(all_debug_items);
    std::string hier = "adder";
    std::ofstream waves("cxxrtl.vcd");
    Simulator* simulator = new Simulator();

    // simulator->reset();
    vcd.sample(0);
    top.step();
    // RESET
    top.p_clk.set<bool>(false);  // clk -> 0
    top.p_rst.set<bool>(false);  // rst -> 0
    top.step();
    vcd.sample(count++);
    top.p_clk.set<bool>(true);   // clk -> 1
    top.p_rst.set<bool>(false);  // rst -> 0
    top.step();
    vcd.sample(count++);
    top.p_clk.set<bool>(false);  // clk -> 0
    top.p_rst.set<bool>(false);  // rst -> 0
    top.step();
    vcd.sample(count++);
    top.p_clk.set<bool>(true);   // clk -> 1
    top.p_rst.set<bool>(false);  // rst -> 0
    top.step();
    vcd.sample(count++);
    top.p_clk.set<bool>(false);  // clk -> 0
    top.p_rst.set<bool>(true);   // rst -> 1
    top.step();
    vcd.sample(count++);

    for (int cycle = 0; cycle < 5; ++cycle) {
        top.p_clk.set<bool>(true);
        // simulator->setCLK(true);
        if (cycle == 0)
            top.p_rst.set<bool>(false);
        else
            top.p_rst.set<bool>(true);
        if (cycle > 0) {
            size_t idx = 0;
            // simulator->setRandVal();
            top.p_b.set<unsigned>(rGen.getRandNum(0, 16));
            top.p_a.set<unsigned>(rGen.getRandNum(0, 16));

            std::cout << "A Value: " << top.p_a.get<unsigned>() << std::endl;
            std::cout << "B Value: " << top.p_b.get<unsigned>() << std::endl;
        }
        // simulator->eval();
        top.step();
        vcd.sample(count++);

        top.p_clk.set<bool>(false);
        // simulator->setCLK(false);
        // simulator->eval();
        top.step();
        vcd.sample(count++);

        // simulator->outVCD();
        // simulator->printValue();
        std::cout << vcd.buffer << "\n";
        waves << vcd.buffer;
        vcd.buffer.clear();
        uint32_t sum = top.p_sum.get<uint32_t>();
        std::cout << "==========================================\n";
        std::cout << "= cycle " << cycle + 1 << "\n";
        std::cout << "==========================================\n";
        std::cout << "sum= " << sum << "\n";
        std::cout << std::endl;
    }
    std::cout << "Finish CXXRTL !!! \n";
}
