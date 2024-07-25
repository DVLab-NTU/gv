#ifndef interface_hpp
#define interface_hpp

#include "crpg.hpp"

class Interface {
public:
    Interface(Vdesign_under_test___024root *rootp) {
        // CLK Init
	    clk.push_back(new SignalC("clk", &rootp->clk, 1));

        // RST Init
	    rst.push_back(new SignalC("rst", &rootp->rst, 1));

        // PI Init List
	    pi.push_back(new SignalC("b", &rootp->b, 4));
	    pi.push_back(new SignalC("a", &rootp->a, 4));

        // PO Init List
	    po.push_back(new SignalC("sum", &rootp->sum, 5));

        // REG Init List

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

