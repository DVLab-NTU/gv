#include "vcdMgr.h"

/**
 * @brief
 *
 */
void VCDMgr::printAllSignalInfo(const size_t& num) {
    size_t n = (num == 0) ? getNumSignals() : num;
    std::cout << "0: time" << std::endl;
    for (size_t i = 0; i < n; ++i) {
        VCDSignal* signal = getSignal(i);
        printSignalInfo(signal, i);
    }
}

/**
 * @brief
 *
 */
void VCDMgr::printSignalInfo(VCDSignal* signal, const size_t& idx) {
    if (signal == nullptr) {
        std::cout << "ERROR: Signal is nullptr !!\n";
    }
    std::cout << idx + 1 << ": " << signal->scope->name << "."
              << signal->reference << std::endl;
}

/**
 * @brief
 *
 * @param scope
 */
void VCDMgr::printVCDFile(std::string scope) {
    resetStatus();
    printAllSignalInfo();

    std::cout << std::endl;
    for (size_t i = 0, n = getSignals()->size() + 1; i < n; ++i) std::cout << i << " ";
    std::cout << "\n";
    for (size_t i = 0, n = getSignals()->size() + 1; i < n; ++i) std::cout << "==";
    std::cout << "\n";

    for (size_t i = 0; i < _times->size() && i < getRowLimit(); ++i) {
        VCDTime time = getTimeStamp(i);
        std::cout << time << " ";
        for (size_t j = 0; j < _scopes->size(); ++j) {
            printScope(getScope(j), time);
        }
        _printStatus.signalCount = 0;
        std::cout << "\n";
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
    for (size_t i = 0; i < numSignals && getSigCount() < getColLimit(); ++i, incSigCount()) {
        VCDSignal* signal = scope->signals[i];
        VCDValue* val = getSignalValueAt(signal, time);
        std::cout << getDecValue(val) << " ";
    }
}

/**
 * @brief
 *
 */
void VCDMgr::printSignal(std::vector<std::string> signalsStrVec) {
    resetStatus();
    std::cout << "0: time" << std::endl;
    for (size_t i = 0; i < signalsStrVec.size(); ++i) {
        VCDSignal* signal = getRefenceSignal(signalsStrVec[i]);
        printSignalInfo(signal, i);
    }

    for (size_t i = 0, n = signalsStrVec.size() + 1; i < n; ++i) std::cout << i << " ";
    std::cout << "\n";
    for (size_t i = 0, n = signalsStrVec.size() + 1; i < n; ++i) std::cout << "==";
    std::cout << "\n";

    for (size_t t = 1; t < getNumTimeStamps() && t < getRowLimit(); ++t) {
        std::cout << t << " ";
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
    std::cout << getDecValue(val) << " ";
}