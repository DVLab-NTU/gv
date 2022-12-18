#include "simulator.hpp"
#include "string"
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include "argparse.hpp"

int main(int argc, char *argv[]) {

    Simulator *simulator = new Simulator();
    unsigned RegNum = simulator->getRegNum();
    unsigned PiNum = simulator->getPiNum();
    unsigned PoNum = simulator->getPoNum();
	int shmid;
	key_t key;
	char *shm, *s;

    // IPC - Shared Memory
	key= 7899;
    //size_t shm_size = PiNum + RegNum + PoNum;
    size_t shm_size = 1000000;
    cout << "[INFO] Size of Shared Memory: " << shm_size << endl;
	shmid= shmget(key, shm_size, 0666);
	if(shmid == -1) {
		cout << strerror(errno) << endl;;
		perror("Error in Shared Memory get statement");
		exit(1);
	}

	shm= (char*) shmat(shmid, NULL, 0);
	if(shm == (char*) -1) {
		cout << strerror(errno) << endl;
		perror("Error in Shared Memory attachment");
		exit(1);
	}

    // Argument Parser
    argparse::ArgumentParser program("verilator_api");
    
    // reset
    program.add_argument("-R", "--reset")
        .default_value(false)
        .help("whether simulator reset design or not")
        .implicit_value(true);
     
    // sequence
    program.add_argument("-S", "--sequence")
        .default_value(false)
        .help("get pattern sequence of design")
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    }
    catch (const runtime_error& err) {
        cerr << err.what() << endl;
        cerr << program;
        exit(1);
    }

    if (program["--reset"] == true) {
        cout << "[INFO] Verilator - Reset" << endl;
        simulator->resetDUV();
        vector<unsigned> pattern;
        simulator->getPiPattern(pattern);
        simulator->getRegPattern(pattern);
        simulator->getPoPattern(pattern);

        //simulator->printPiPattern();
        //simulator->printRegPattern();
        //simulator->printPoPattern();
        cout << "[INFO] Verilator - Write data => Shared memory (id=" << shmid << ")" << endl;
        string pattern_str = simulator->pattern2str(pattern) + " ";
        cout << pattern_str << endl;
        memcpy(shm, pattern_str.c_str(), pattern_str.size());
    }

    if (program["--sequence"] == true) {
        cout << "[INFO] Verilator - Write data => Shared memory (id=" << shmid << ")" << endl;
        string sequence = simulator->getSequence() + " ";
        cout << sequence << endl;
        memcpy(shm, sequence.c_str(), sequence.size());
    }

    /*
    //cout << "PiNum: " << PiNum << endl;

    
    string data(shm);
    cout << "[Verilator] Shared Memory (id=" << shmid << ") => Read data " << data << endl;
    cout << "[Verilator] ACK" << endl;
    
	*shm= '*';
    shmctl(shmid, IPC_RMID, 0);
    for (unsigned i=0 ; i < PiNum ; i++){
        simulator->setPI(i, (unsigned)data[i]);
    }
    simulator->printPO(1);
    simulator->evalOneClock();
    simulator->printPO(1);*/
    return 0;
}
