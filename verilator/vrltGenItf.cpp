#include "fstream"
#include "yosysMgr.h"

using namespace std;

static string signalWidth2Type(const int& width) {
    if (width <= 8) return "C";
    else if (width <= 16) return "S";
    else if (width <= 32) return "I";
    else if (width <= 64) return "Q";
    else return "W";
}
static string genItfFile(const string type, YosysSignal* signal) {
    string line = "    " + type + ".push_back(new Signal" + signalWidth2Type(signal->getWidth()) + "(\"" + signal->getName() +
                  "\", &rootp->" + signal->getName() + ", " + to_string(signal->getWidth()) + "));";
    return line;
}
static void genVrltItf() {
    // string path = "./template/interface.cpp";
    ifstream infile("interface.hpp", ios::in);
    ofstream outfile("interface_mod.hpp", ios::out);
    if (!infile) cout << "ERROR !! \n";
    while (infile) {
        string tmpLine;
        getline(infile, tmpLine);
        outfile << tmpLine << endl;
        if (tmpLine.find("CLK") != string::npos) {
            for (int i = 0; i < yosysMgr->getClkNum(); ++i)
                outfile << genItfFile("clk", yosysMgr->getClk(0)) << endl;
        } else if (tmpLine.find("RST") != string::npos) {
            for (int i = 0; i < yosysMgr->getRstNum(); ++i)
                outfile << genItfFile("rst", yosysMgr->getRst(i)) << endl;
        } else if (tmpLine.find("PI") != string::npos) {
            for (int i = 0; i < yosysMgr->getPiNum(); ++i)
                outfile << genItfFile("pi", yosysMgr->getPi(i)) << endl;
        } else if (tmpLine.find("PO") != string::npos) {
            for (int i = 0; i < yosysMgr->getPoNum(); ++i)
                outfile << genItfFile("po", yosysMgr->getPo(i)) << endl;
        }
    }
}
