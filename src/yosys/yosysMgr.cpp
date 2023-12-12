#include "yosysMgr.h"

#include <cstdlib>

YosysMgr* yosysMgr;

void YosysMgr::init() {
    Yosys::yosys_setup();
}

void YosysMgr::reset() {
    delete Yosys::yosys_design;
    Yosys::yosys_design = new RTLIL::Design;
}

void YosysMgr::setLogging(const bool& enable) {
    if (enable && log_streams.empty())
        log_streams.push_back(&std::cout);
    else if (!enable && !log_streams.empty())
        log_streams.pop_back();
}

void YosysMgr::createMapping(const string& fileName) {
    string yosys           = "yosys -qp ";
    string readVerilog     = "read_verilog " + fileName + "; ";
    string topModule       = "hierarchy -auto-top; ";
    string preProcess      = "flatten; proc; techmap; setundef -zero; aigmap; ";
    string writeAigMapping = "write_aiger -map .map.txt ._temp_.aig";
    string command         = yosys + "\"" + readVerilog + topModule + preProcess + writeAigMapping + "\"";
    system(command.c_str());
}
