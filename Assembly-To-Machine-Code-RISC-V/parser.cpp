#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main() {
    string line = "";
    ifstream infile("MCode.mc");
    ofstream outfile("Random.txt");
    if (infile.is_open()) {
        while (infile >> line) {
            if (line == "-------------------------------------------------------") {
                break;
            }
            else if (line.length() == 10) {
                outfile << line.substr(2, 2) << "_" << line.substr(4, 2) << "_" << line.substr(6, 2) << "_" << line.substr(8, 2) << endl;
            }
        }
    } else {
        cerr << "file is not opened" << endl;
    }
    infile.close();
    outfile.close();
}

