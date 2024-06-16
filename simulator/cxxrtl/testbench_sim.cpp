#include <backends/cxxrtl/cxxrtl_vcd.h>

#include <iostream>

#include "simulator.hpp"

int main() {
    Simulator* simulator = new Simulator(5);
    simulator->loadRandomPattern();
    simulator->startSim(true);
    simulator->finish();
    std::cout << "Finish CXXRTL !!! \n";
}
