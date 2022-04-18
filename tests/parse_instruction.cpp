#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main() {
    string line = "";
    ifstream infile("I_mem_test");
    ofstream outfile;
    vector<string> strs;
    string str = "", tmp = "";
    int mem_num = 8192;
    int count = 0;
    outfile.open("I_test.txt");
    if (infile.is_open()) {
        while (getline(infile, line)) {
            for (unsigned i = 0; i < line.length(); ++i) {
                if (line[i] == '_'){
                    strs.push_back(str);
                    str = "";
                } else {
                    str = str + line[i];
                }
            }
            strs.push_back(str.substr(0, 2));        
            str = "";
            for (unsigned i = 0; i < 4; ++i) {
                for (unsigned j = 0; j < 2; ++j) {
                    count ++;
                    tmp = strs[3-i][j];
                    if (tmp == "0") {
                        outfile << "0000";
                    } else if (tmp == "1") {
                        outfile << "0001";
                    } else if (tmp == "2") {
                        outfile << "0010";
                    } else if (tmp== "3") {
                        outfile << "0011";
                    } else if (tmp == "4") {
                        outfile << "0100";
                    } else if (tmp == "5") {
                        outfile << "0101";
                    } else if (tmp == "6") {
                        outfile << "0110";
                    } else if (tmp == "7") {
                        outfile << "0111";
                    } else if (tmp == "8") {
                        outfile << "1000";
                    } else if (tmp == "9") {
                        outfile << "1001";
                    } else if (tmp == "A") {
                        outfile << "1010";
                    } else if (tmp == "B") {
                        outfile << "1011";
                    } else if (tmp == "C") {
                        outfile << "1100";
                    } else if (tmp == "D") {
                        outfile << "1101";
                    } else if (tmp == "E") {
                        outfile << "1110";
                    } else if (tmp == "F") {
                        outfile << "1111";
                    }
                }
            }
            strs.clear();
        }
        infile.close();
    } else {
        cerr << "file is not opened" << endl;
    }
    for (unsigned i = 0; i < mem_num-count*4; ++i) {
        outfile << "0";
    }
    outfile.close();
}