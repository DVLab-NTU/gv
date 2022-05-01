#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "Error: Missing number or output file" << endl;
        exit(-1);
    }
    srand(time(NULL));
    int num = stoi(argv[1]);
    ofstream outfile(argv[2]);
    vector<string> ins = {"add", "addi", "sub", "subi", "ld", "sd", "beq", "bge", "ble", "jal"};
    vector<string> regs = {"x0", "x5", "x6", "x7", "x8", "x9", "x18", "x19", "x20", "x21", "x22", "x23", "x24", 
                           "x25", "x26", "x27", "x28", "x29", "x30", "x31"};
    vector<string> funcs = {"a", "b", "c", "d", "e"};
    vector<int> num_funcs = {1, 1, 1, 1, 1};
    int in = 0, reg = 0, n = 0, func = 0, num_func = 0;
    func = 4.0 * rand() / (RAND_MAX + 1.0) + 1;
    // cout << func << endl;

    for (unsigned k = 0; k < func; ++k) {
        num_func = (double)num * rand() / (RAND_MAX + 1.0);
        num_funcs[k] += num_func;
        num -= num_func;
        outfile << funcs[k] << ": ";
        for (unsigned i = 0; i < num_funcs[k]-1; ++i) {
            in = ins.size() * rand() / (RAND_MAX + 1.0);
            outfile << ins[in];
            if (in == 0 || in == 2)    // add or sub
            {  
                for (unsigned j = 0; j < 3; ++j) {
                    reg = regs.size() * rand() / (RAND_MAX + 1.0);
                    outfile << " " << regs[reg];
                }
            }
            else if (in == 1 || in == 3)   // addi or subi
            {
                for (unsigned j = 0; j < 2; ++j) {
                    reg = regs.size() * rand() / (RAND_MAX + 1.0);
                    outfile << " " << regs[reg];
                }
                n = 10.0 * rand() / (RAND_MAX + 1.0);
                outfile << " " << n;
            }
            else if (in == 4 || in == 5)    // sd or ld 
            {
                reg = regs.size() * rand() / (RAND_MAX + 1.0);
                outfile << " " << regs[reg];
                reg = regs.size() * rand() / (RAND_MAX + 1.0);
                n = 3.0 * rand() / (RAND_MAX + 1.0);
                outfile << " " << n*4 << "(" << regs[reg] << ")";
            }
            else if (in == 6 || in == 7 || in == 8)    // beq or bge or ble
            {
                for (unsigned j = 0; j < 2; ++j) {
                    reg = regs.size() * rand() / (RAND_MAX + 1.0);
                    outfile << " " << regs[reg];
                }
                n = func * rand() / (RAND_MAX + 1.0);
                outfile << " " << funcs[n];
            }
            else if (in == 9)   // jal
            {
                n = func * rand() / (RAND_MAX + 1.0);
                outfile << "jal x0 " << funcs[n] << endl;
            }
            outfile << endl;
            num --;
        }
        n = 2.0 * rand() / (RAND_MAX + 1.0);
        if (n == 0) {
            outfile << "jalr x0 0(x1)" << endl; 
        } else {
            n = func * rand() / (RAND_MAX + 1.0);
            outfile << "jal x0 " << funcs[n] << endl;
        }
        outfile << endl;
    }
    outfile << "exit:";
    outfile.close();
}