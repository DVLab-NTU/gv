//
//  interface.hpp
//  Constrained-Random-Pattern-Generation
//  dhgir.abien@gmail.com
//

#ifndef interface_hpp
#define interface_hpp

#include "crpg.hpp"

class Interface {
public:
    Interface(Vdesign_under_test___024root *rootp) {
        // CLK Init

        // RST Init

        // PI Init List

        // PO Init List

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
