#include "argparse.hpp"
#include "simulator.hpp"
#include "string"
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

int
main(int argc, char* argv[]) {

    Simulator* simulator = new Simulator();
    unsigned   RegNum    = simulator->getRegNum();
    unsigned   PiNum     = simulator->getPiNum();
    unsigned   PoNum     = simulator->getPoNum();
    int        shmid;
    key_t      key;
    char *     shm, *s;

    // IPC - Shared Memory
    key = 7899;
    // size_t shm_size = PiNum + RegNum + PoNum;
    size_t shm_size = 1000000;
    cout << "[INFO] Size of Shared Memory: " << shm_size << endl;
    shmid = shmget(key, shm_size, 0666);
    if (shmid == -1) {
        cout << strerror(errno) << endl;
        ;
        perror("Error in Shared Memory get statement");
        exit(1);
    }

    shm = (char*)shmat(shmid, NULL, 0);
    if (shm == (char*)-1) {
        cout << strerror(errno) << endl;
        perror("Error in Shared Memory attachment");
        exit(1);
    }

    // Argument Parser
    argparse::ArgumentParser program("verilator_api");

    // reset
    program.add_argument("-R", "--reset").default_value(false).help("whether simulator reset design or not").implicit_value(true);

    // sequence
    program.add_argument("-S", "--sequence").default_value(false).help("return pattern sequence of design").implicit_value(true);

    // update
    program.add_argument("-U", "--update").default_value(false).help("update current state to verilator").implicit_value(true);

    // eval
    program.add_argument("-E", "--eval").default_value(0).help("eval n cycle").scan<'i', int>();

    // printState
    program.add_argument("-P", "--print").default_value(false).help("print simulator info").implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const runtime_error& err) {
        cerr << err.what() << endl;
        cerr << program;
        exit(1);
    }

    if (program["--reset"] == true) {
        cout << "[INFO] Verilator - Reset" << endl;
        simulator->resetDUV();
        vector<unsigned> pattern;
        simulator->getAllPattern(pattern);
        string pattern_str = simulator->pattern2str(pattern) + " ";
        cout << "[INFO] Verilator - Write data (size=" << pattern_str.size() - 1 << ") => Shared memory (id=" << shmid << ")" << endl;
        //cout << pattern_str << endl;
        memcpy(shm, pattern_str.c_str(), pattern_str.size());
    }

    if (program["--sequence"] == true) {
        string sequence = simulator->getSequence() + " ";
        cout << "[INFO] Verilator - Write data (size=" << sequence.size() - 1 << ") => Shared memory (id=" << shmid << ")" << endl;
        //cout << sequence << endl;
        memcpy(shm, sequence.c_str(), sequence.size());
    }

    if (program["--update"] == true) {
        string pattern_str(shm);
        string token;
        size_t pos = pattern_str.find(" ");
        token      = pattern_str.substr(0, pos);
        cout << "[INFO] Verilator - Shared memory (id=" << shmid << ") => Read data (size=" << token.size() << ")" << endl;
        //cout << token << endl;
        vector<unsigned> pattern = simulator->str2pattern(token);
        simulator->setAllPattern(pattern);
    }

    auto eval_cycle = program.get<int>("eval");
    if (eval_cycle != 0) {
        string pattern_str(shm);
        string token;
        size_t pos = pattern_str.find(" ");
        token      = pattern_str.substr(0, pos);
        cout << "[INFO] Verilator - Shared memory (id=" << shmid << ") => Read data (size=" << token.size() << ")" << endl;
        //cout << token << endl;
        vector<unsigned> pattern = simulator->str2pattern(token);
        simulator->setAllPattern(pattern);
        cout << "[INFO] Verilator - eval " << eval_cycle << " cycle" << endl;
        for (unsigned i = 0; i < eval_cycle; i++) {
            simulator->evalOneClock();
        }
        vector<unsigned> AllPattern;
        simulator->getAllPattern(AllPattern);
        pattern_str = simulator->pattern2str(AllPattern) + " ";
        cout << "[INFO] Verilator - Write data (size=" << pattern_str.size() - 1 << ") => Shared memory (id=" << shmid << ")" << endl;
        //cout << pattern_str << endl;
        memcpy(shm, pattern_str.c_str(), pattern_str.size());
    }

    if (program["--print"] == true) {
        string pattern_str(shm);
        string token;
        size_t pos = pattern_str.find(" ");
        token      = pattern_str.substr(0, pos);
        cout << "[INFO] Verilator - Shared memory (id=" << shmid << ") => Read data (size=" << token.size() << ")" << endl;
        //cout << token << endl;
        vector<unsigned> pattern = simulator->str2pattern(token);
        simulator->setAllPattern(pattern);
        simulator->printPiPattern();
        simulator->printRegPattern();
        // simulator->printPoPattern();
    }

    return 0;
}
