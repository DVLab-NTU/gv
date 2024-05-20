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
    // TODO: ...
    cout << "CYCLE Macro: " << CYCLE << endl;
    timer.start();
    Simulator *simulator = new Simulator();
    double random_time   = timer.getTime();
    if (PURE_RANDOM) {
        cout << "******************************" << endl;
        cout << "*" << setw(21) << "Pure Random" << setw(8) << "*" << endl;
        cout << "******************************" << endl;

        if (simulator->getRstNum() > 0)
            simulator->resetNegDUV();

        unsigned i;
        for (i = 0; i < CYCLE; i++) {
            std::vector<unsigned> piPatternR = simulator->genPiRandomPattern();
            simulator->setPiPattern(piPatternR);
            simulator->evalOneClock();
            cout << GREEN_TEXT << " ########### " << i << " ###########\n"
                 << RESET_COLOR;
            for (int j = 0; j < simulator->getPiNum(); ++j)
                simulator->printPI(j);
            for (int j = 0; j < simulator->getPoNum(); ++j)
                simulator->printPO(j);
            for (int j = 0; j < simulator->getRegNum(); ++j)
                simulator->printREG(j);
            cout << endl;
        }
        return 0;
    } else if (STIMULUS) {
        cout << "******************************" << endl;
        cout << "*" << setw(20) << "Stimulus " << setw(9) << "*" << endl;
        cout << "******************************" << endl;

        simulator->loadInputPattern();
        simulator->startSim(true);
    }
    return 0;
}