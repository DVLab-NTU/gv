// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2017 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
//======================================================================

// For std::unique_ptr
#include <memory>

// Include common routines
#include "verilated_vcd_c.h"
#include <verilated.h>

#include "Vtop.h"
// Legacy function required only so linking works on Cygwin and MSVC++
double sc_time_stamp() { return 0; }

int main(int argc, char** argv) {
    // This is a more complicated example, please also see the simpler examples/make_hello_c.

    // Create logs/ directory in case we have traces to put under it
    Verilated::traceEverOn(true);
    Verilated::mkdir("logs");
    VerilatedVcdC* tfp = new VerilatedVcdC;

    // Construct a VerilatedContext to hold simulation time, etc.
    // Multiple modules (made later below with Vtop) may share the same
    // context to share time, or modules may have different contexts if
    // they should be independent from each other.

    // Using unique_ptr is similar to
    // "VerilatedContext* contextp = new VerilatedContext" then deleting at end.
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    // Do not instead make Vtop as a file-scope static variable, as the
    // "C++ static initialization order fiasco" may cause a crash

    // Set debug level, 0 is off, 9 is highest presently used
    // May be overridden by commandArgs argument parsing
    contextp->debug(0);
    // Randomization reset policy
    // May be overridden by commandArgs argument parsing
    contextp->randReset(2);
    // Verilator must compute traced signals
    contextp->traceEverOn(true);
    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    // This needs to be called before you create any model
    contextp->commandArgs(argc, argv);

    // Construct the Verilated model, from Vtop.h generated from Verilating "top.v".
    // Using unique_ptr is similar to "Vtop* top = new Vtop" then deleting at end.
    // "TOP" will be the hierarchical name of the module.
    const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "vendingMachine"}};
    top->trace(tfp, 2);
    tfp->open("./sim_vending.vcd");
    int timestamp = 0;

    // ### Vending Machine I/O ###
    // input          clk;
    // input          reset;
    // // Input Ports
    // input  [5:0]   coinInA;          // input number of COIN_A
    // input  [5:0]   coinInB;          // input number of COIN_B
    // input  [5:0]   coinInC;          // input number of COIN_C
    // input  [5:0]   coinInD;          // input number of COIN_D
    // input  [1:0]   itemTypeIn;       // type of an item
    // input  [2:0]   itemNumberIn;     // number of the item
    // input          forceIn;          // service max as possible
    // ### end ###

    // Set Vtop's input signals
    top->clk = 0;
    top->reset = 0;
    top->coinInA = 0;
    top->coinInB = 2;
    top->coinInC = 0;
    top->coinInD = 0;
    top->itemTypeIn = 0;
    top->itemNumberIn = 1;
    top->forceIn = 0;

    // negative reset
    top->eval();
    tfp->dump(timestamp++);
    top->clk = 1;
    top->reset = 0;
    top->eval();
    tfp->dump(timestamp++);
    top->reset = 1;
    for (int i = 0; i < 20; ++i) {
        top->clk = !top->clk;
        top->eval();
        VL_PRINTF(" ===== Cycle %d =====\n", i);
        VL_PRINTF(" clk=%x rst=%x itemTypeIn=%x \n", top->clk, top->reset, top->itemTypeIn);
        VL_PRINTF(" coinInA = %" PRId8 " coinInB = %" PRId8 " coinInC = %" PRId8
                  " coinInD = %" PRId8 "\n",
                  top->coinInA, top->coinInB, top->coinInC, top->coinInD);
        VL_PRINTF(" coinOutA = %" PRId8 " coinOutB = %" PRId8 " coinOutC = %" PRId8
                  " coinOutD = %" PRId8 "\n",
                  top->coinOutA, top->coinOutB, top->coinOutC, top->coinOutD);
        VL_PRINTF(" serviceTypeOut=%" PRId8 "\n\n ", top->serviceTypeOut);
        tfp->dump(timestamp++);
        if (top->serviceTypeOut == 0) { break; }
    }
    tfp->dump(timestamp++);

    // Final model cleanup
    top->final();
    tfp->close();

    // Coverage analysis (calling write only after the test is known to pass)
#if VM_COVERAGE
    Verilated::mkdir("logs");
    contextp->coveragep()->write("logs/coverage.dat");
#endif

    // Final simulation summary
    contextp->statsPrintSummary();
    // Return good completion status
    // Don't use exit() or destructor won't get called
    return 0;
}
