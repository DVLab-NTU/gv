//
//  main.cpp
//  Constrained Random Pattern Generation
//  dhgir.abien@gmail.com
//

#include <string>

#include "fstream"
#include "simulator.hpp"
#include "string"
#include "utility.hpp"

#define PURE_RANDOM 0
#define STIMULUS 1
// #define CYCLE 10

using namespace std;

int main(int argc, char **argv, char **env) {
    // TODO: Assertion Handler
    timer.start();
    Simulator *simulator = new Simulator();
    simulator->setCycle(CYCLE);
    simulator->openVcdFile();
    simulator->contextp->commandArgs(argc, argv);

    // double random_time = timer.getTime();
    if (MODE == PURE_RANDOM) {
        simulator->loadRandomPattern();
        simulator->startSim(true);
        return 0;
    } else if (MODE == STIMULUS) {
        if (!simulator->loadInputPattern()) return 1;
        simulator->startSim(true);
    }
    // simulator->closeVcdFile();

    return 0;
}