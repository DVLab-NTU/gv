//
//  main.cpp
//  Constrained Random Pattern Generation
//  dhgir.abien@gmail.com
//

#include "fstream"
#include "simulator.hpp"
#include "string"
#include "utility.hpp"

#define PURE_RANDOM false
#define STIMULUS true
// #define CYCLE 10

using namespace std;

#define GREEN_TEXT "\033[32m"  // ANSI escape code for green color
#define RESET_COLOR "\033[0m"  // ANSI escape code to reset color

int main(int argc, char **argv, char **env) {
    // TODO: Assertion Handler

    timer.start();
    Simulator *simulator = new Simulator();
    simulator->setCycle(CYCLE);
    double random_time = timer.getTime();
    if (PURE_RANDOM) {
        if (simulator->getRstNum() > 0)
            simulator->resetNegDUV();

        unsigned i;
        for (i = 0; i < CYCLE; i++) {
            std::vector<unsigned> piPatternR = simulator->genPiRandomPattern();
            simulator->setPiPattern(piPatternR);
            simulator->evalOneClock();
        }
        return 0;
    } else if (STIMULUS) {
        simulator->loadInputPattern();
        simulator->startSim(true);
    }
    return 0;
}