#ifndef interface_hpp
#define interface_hpp

#include "crpg.hpp"

class Interface {
public:
    Interface(Vdesign_under_test___024root *rootp) {
        // CLK Init
	    clk.push_back(new SignalC("i_clk", &rootp->i_clk, 1));

        // RST Init
	    rst.push_back(new SignalC("i_rst_n", &rootp->i_rst_n, 1));

        // PI Init List
	    pi.push_back(new SignalC("i_inst", &rootp->i_inst, 3));
	    pi.push_back(new SignalC("i_data_b", &rootp->i_data_b, 8));
	    pi.push_back(new SignalC("i_data_a", &rootp->i_data_a, 8));
	    pi.push_back(new SignalC("i_valid", &rootp->i_valid, 1));

        // PO Init List
	    po.push_back(new SignalC("o_data", &rootp->o_data, 8));
	    po.push_back(new SignalC("o_valid", &rootp->o_valid, 1));

        // REG Init List
	    reg.push_back(new SignalC("alu__DOT__mod_reg", &rootp->alu__DOT__mod_reg, 8));
	    reg.push_back(new SignalI("alu__DOT__temp_multiply", &rootp->alu__DOT__temp_multiply, 17));
	    reg.push_back(new SignalC("alu__DOT__o_valid_r", &rootp->alu__DOT__o_valid_r, 1));
	    reg.push_back(new SignalC("alu__DOT__o_valid_w", &rootp->alu__DOT__o_valid_w, 1));
	    reg.push_back(new SignalS("alu__DOT__o_data_r", &rootp->alu__DOT__o_data_r, 9));
	    reg.push_back(new SignalS("alu__DOT__o_data_w", &rootp->alu__DOT__o_data_w, 9));

        // MEM Init List
    }

    std::vector<Signal *> clk;
    std::vector<Signal *> rst;
    std::vector<Signal *> pi;
    std::vector<Signal *> po;
    std::vector<Signal *> reg;
    std::vector<Signal *> mem;
};

#endif /* interface_hpp */

