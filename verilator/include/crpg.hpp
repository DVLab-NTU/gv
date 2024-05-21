//
//  crpg.hpp
//  Constrained Random Pattern Generation
//

#ifndef crpg_h
#define crpg_h

#include <verilated.h>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "Vdesign_under_test.h"
#include "Vdesign_under_test___024root.h"
#include "svdpi.h"
#include "verilated_vcd_c.h"
// #include "Vdesign_under_test__Dpi.h"

#include "NumCpp.hpp"

// Time Factor
#define UCB_C 0.0001
#define EXPANSION_FACTOR 10
#define VALID_SAMPLE_FACTOR 10
#define ATTENUATION_COEFFICIENT 0.9
#define STATE_TRANSFER_MAX_TRY 10
#define SAMPLE_TIMEOUT 0.5
#define CART_DEPTH 3
#define CART_TIMEOUT 5

// Space Factor
#define MAXV 100
#define HASH_TABLE_SIZE 10000

// Other
#define INV_MATRIX_DELTA 0.00001
#define VALUE_NORMALIZATION_FACTOR 10000
#define FINAL_STATE_INDEX 4294967295

// Global Count
unsigned transitionNum    = 0;
unsigned hashNum          = 0;
unsigned hashCollisionNum = 0;
unsigned hashHitNum       = 0;
unsigned mctsNodeNum      = 0;
unsigned mctsBranchNum    = 0;

vluint64_t main_time = 1000;
double sc_time_stamp() {
    return main_time;
}

class Signal {
public:
    Signal(
        std::string newName,
        void* newValue,
        unsigned newWidth) : name(newName),
                             value(newValue),
                             width(newWidth) {}

    virtual const char getType() = 0;

    std::string name;
    void* value;
    unsigned width;
};

class SignalI : public Signal {
public:
    SignalI(
        std::string newName,
        void* newValue,
        unsigned newWidth) : Signal(newName, newValue, newWidth) {}

    virtual const char getType() override { return 'I'; }
};

class SignalC : public Signal {
public:
    SignalC(
        std::string newName,
        void* newValue,
        unsigned newWidth) : Signal(newName, newValue, newWidth) {}

    virtual const char getType() override { return 'C'; }
};

class SignalS : public Signal {
public:
    SignalS(
        std::string newName,
        void* newValue,
        unsigned newWidth) : Signal(newName, newValue, newWidth) {}

    virtual const char getType() override { return 'S'; }
};

class SignalQ : public Signal {
public:
    SignalQ(
        std::string newName,
        void* newValue,
        unsigned newWidth) : Signal(newName, newValue, newWidth) {}

    virtual const char getType() override { return 'Q'; }
};

class Property {
public:
    Property(
        std::string newFileName,
        unsigned newIndex,
        unsigned newLineNumber,
        std::string newStatement) : fileName(newFileName),
                                    index(newIndex),
                                    lineNumber(newLineNumber),
                                    statement(newStatement) {}

    std::string fileName;
    unsigned index;
    unsigned lineNumber;
    std::string statement;
};

class FsmState {
public:
    FsmState(
        unsigned newIndex,
        unsigned newEncoding,
        std::vector<unsigned> newTransitionTable) : index(newIndex),
                                                    encoding(newEncoding),
                                                    transitionTable(newTransitionTable) {}

    unsigned index;
    unsigned encoding;
    std::vector<unsigned> transitionTable;
};

class FSM {
public:
    FSM(
        unsigned newIndex,
        std::string newVarName,
        unsigned newVarIndex) : index(newIndex),
                                varName(newVarName),
                                varIndex(newVarIndex) {}

    unsigned index;
    std::string varName;
    unsigned varIndex;
    std::vector<FsmState> states;
};

class DPI {
public:
    static bool assertion_flag;
    static unsigned assertion_index;
    static bool constraint_flag;
    static unsigned constraint_index;
    static unsigned constraint_count;
    static unsigned constraint_count_n;
    static bool stable_flag;
    static unsigned stable_index;
    static unsigned assertion_active_index;

    static void rst_asset_flag(void) {
        assertion_flag = false;
    }

    static bool get_asset_flag(void) {
        return assertion_flag;
    }

    static void rst_constraint_flag(void) {
        constraint_flag = false;
    }

    static bool get_constraint_flag(void) {
        return constraint_flag;
    }

    static void rst_stable_flag(void) {
        stable_flag = false;
    }

    static bool get_stable_flag(void) {
        return stable_flag;
    }
};

bool DPI::assertion_flag             = false;
unsigned DPI::assertion_index        = 0;
bool DPI::constraint_flag            = false;
unsigned DPI::constraint_index       = 0;
unsigned DPI::constraint_count       = 0;
unsigned DPI::constraint_count_n     = 0;
bool DPI::stable_flag                = false;
unsigned DPI::stable_index           = 0;
unsigned DPI::assertion_active_index = -1;

void set_assert_flag(int index) {
    if (index == DPI::assertion_active_index) {
        DPI::assertion_flag  = true;
        DPI::assertion_index = index;
    } else {
        return;
        // std::cout << "###INFO set_asset_flag: " << index << std::endl;
    }
}

void set_constraint_flag(int index) {
    DPI::constraint_flag  = true;
    DPI::constraint_index = index;
    DPI::constraint_count_n++;
    // std::cout << "###INFO set_constraint_flag: " << index << std::endl;
}

void set_stable_flag(int index) {
    DPI::stable_flag  = true;
    DPI::stable_index = index;
    // std::cout << "###INFO set_stable_flag: " << index << std::endl;
}

#endif /* crpg_h */
