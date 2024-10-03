#include "simulator.hpp"
#include "utility.hpp"

int main() {
    Simulator* simulator = new Simulator();
    simulator->enableVCD();
    simulator->setCycle(CYCLE);

    switch (MODE) {
        case PURE_RANDOM:
            simulator->loadRandomPattern();
            break;
        case STIMULUS:
            simulator->loadInputPattern();
            break;
    }
    simulator->startSim(true);
    simulator->finish();
}
