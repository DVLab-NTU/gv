#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstddef>
#include <string>

#include "vcdMgr.h"

/**
 * @brief
 *
 */
void VCDMgr::printAllSignalInfo(const size_t& num) {
    size_t n = 0;
    if (num == 0)
        n = (getNumSignals() < getColLimit()) ? getNumSignals() : getColLimit();
    fmt::println("0: time");
    for (size_t i = 0; i < n; ++i) {
        VCDSignal* signal = getSignal(i);
        printSignalInfo(signal, i);
    }
    fmt::println("");
}

/**
 * @brief
 *
 */
void VCDMgr::printSignalInfo(VCDSignal* signal, const size_t& idx) {
    if (idx > getColLimit()) {
        std::cout << "ERROR: Column index out of bound !!\n";
        return;
    }
    if (signal == nullptr) {
        std::cout << "ERROR: Signal is nullptr !!\n";
        return;
    }
    fmt::println("{0}: {1}.{2}", idx + 1, signal->scope->name,
                 signal->reference);
}

void VCDMgr::printTableRowTitle(const size_t& colNum) const {
    size_t n = (colNum > getColLimit()) ? getColLimit() + 1 : colNum + 1;
    for (size_t i = 0; i < n; ++i) fmt::print("{0:<5}", i);
    fmt::println("\n{0:=^{1}}", "", 5 * n);
}

/**
 * @brief
 *
 * @param scope
 */
void VCDMgr::printAllSignals(std::string scope) {
    resetStatus();
    printAllSignalInfo();
    printTableRowTitle(getNumSignals());

    for (size_t i = 0; i < _times->size() && i < getRowLimit(); ++i) {
        VCDTime time = getTimeStamp(i);
        fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, "{:<5}", time);
        for (size_t j = 0; j < _scopes->size(); ++j) {
            printScope(getScope(j), time);
        }
        _printStatus.signalCount = 0;
        fmt::println("");
    }
}

/**
 * @brief
 *
 * @param scope
 * @param time
 */
void VCDMgr::printScope(VCDScope* scope, VCDTime time) {
    size_t numSignals = scope->signals.size();
    for (size_t i = 0; i < numSignals && getSigCount() < getColLimit();
         ++i, incSigCount()) {
        VCDSignal* signal = scope->signals[i];
        VCDValue* val = getSignalValueAt(signal, time);
        fmt::print("{:<5}", getHexValue(val));
    }
}

/**
 * @brief
 *
 */
void VCDMgr::printSignal(std::vector<std::string> signalsStrVec) {
    resetStatus();
    fmt::println("0: time");
    for (size_t i = 0; i < signalsStrVec.size(); ++i) {
        VCDSignal* signal = getRefenceSignal(signalsStrVec[i]);
        printSignalInfo(signal, i);
    }
    fmt::println("");
    printTableRowTitle(signalsStrVec.size());
    for (size_t t = 0; t < getNumTimeStamps() && t < getRowLimit(); ++t) {
        fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, "{:<5}", t);
        for (size_t i = 0, n = signalsStrVec.size(); i < n; ++i) {
            VCDSignal* signal = getRefenceSignal(signalsStrVec[i]);
            printTimedSignal(signal, t);
        }
        std::cout << std::endl;
    }
}

/**
 * @brief
 *
 */
void VCDMgr::printTimedSignal(VCDSignal* const signal, const VCDTime& time) {
    VCDValue* val = getSignalValueAt(signal, time);
    if (val == nullptr) {
        std::cout << "ERROR: Value is nullptr !!\n";
    }
    fmt::print("{:<5}", getHexValue(val));
}