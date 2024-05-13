#include "vrltMgr.h"

#include "fstream"
#include "yosysMgr.h"

using namespace std;
VRLTMgr* vrltMgr;

static string signalWidth2Type(const int& width) {
    if (width <= 8) return "C";
    else if (width <= 16) return "S";
    else if (width <= 32) return "I";
    else if (width <= 64) return "Q";
    else return "W";
}
static string genItfCode(const string type, YosysSignal* signal) {
    string line = "    " + type + ".push_back(new Signal" + signalWidth2Type(signal->getWidth()) + "(\"" + signal->getName() +
                  "\", &rootp->" + signal->getName() + ", " + to_string(signal->getWidth()) + "));";
    return line;
}

/**
 * @brief Construct a new VRLTMgr::VRLTMgr object
 *
 */
VRLTMgr::VRLTMgr() {
    _itfFileName = "interface.hpp";
    _itfPath     = string(GV_TEMPLATE_PATH) + _itfFileName;
    _dirPath     = string(GV_VERILATOR_PATH);
};

/**
 * @brief
 *
 */
bool VRLTMgr::preVrltSim() {
    if (!genVrltItf()) return false;
    if (!genVrltMakefile()) return false;
    return true;
}

/**
 * @brief
 *
 */
bool VRLTMgr::genVrltItf() {
    string outFileName = _dirPath + _itfFileName;
    ifstream infile(_itfPath, ios::in);
    ofstream outfile(outFileName, ios::out);
    if (!infile) {
        cout << "Cannot open design \"" << outFileName << "\"!!" << endl;
        return false;
    }
    while (infile) {
        string tmpLine;
        getline(infile, tmpLine);
        outfile << tmpLine << endl;
        if (tmpLine.find("CLK") != string::npos) {
            for (int i = 0; i < yosysMgr->getClkNum(); ++i)
                outfile << genItfCode("clk", yosysMgr->getClk(0)) << endl;
        } else if (tmpLine.find("RST") != string::npos) {
            for (int i = 0; i < yosysMgr->getRstNum(); ++i)
                outfile << genItfCode("rst", yosysMgr->getRst(i)) << endl;
        } else if (tmpLine.find("PI") != string::npos) {
            for (int i = 0; i < yosysMgr->getPiNum(); ++i)
                outfile << genItfCode("pi", yosysMgr->getPi(i)) << endl;
        } else if (tmpLine.find("PO") != string::npos) {
            for (int i = 0; i < yosysMgr->getPoNum(); ++i)
                outfile << genItfCode("po", yosysMgr->getPo(i)) << endl;
        }
    }
    infile.close();
    outfile.close();
    return true;
}

bool VRLTMgr::genVrltMakefile() {
    // TODO: Compile the design with Verilator command
    string command = "cd " + string(GV_VERILATOR_PATH) + "; make";
    string execute = "cd " + string(GV_VERILATOR_PATH) + "; ./build/obj_dir/Vdesign_under_test";
    system(command.c_str());
    system(execute.c_str());
}
