
#include "vcdMgr.h"

#include <fmt/core.h>

#include <cstddef>
#include <string>

#include "VCDValue.hpp"

//! Global VCD Manager pointer
VCDMgr* vcdMgr = nullptr;

/**
 * @brief Convert a VCDBit to a single char
 *
 * @param b
 * @return char
 */
static char VCDBit2Char(VCDBit b) {
    switch (b) {
        case (VCD_0):
            return '0';
        case (VCD_1):
            return '1';
        case (VCD_Z):
            return 'Z';
        case (VCD_X):
        default:
            return 'X';
    }
}

VCDMgr::VCDMgr() : _colLimit(15), _rowLimit(20) {}

VCDMgr::~VCDMgr() {
}

/**
 * @brief
 *
 */
void VCDMgr::resetStatus() {
    _printStatus.colLimit = _colLimit;
    _printStatus.rowLimit = _rowLimit;
    _printStatus.signalCount = 0;
    _printStatus.isExceed = false;
}

/**
 * @brief
 *
 */
bool VCDMgr::readVCDFile(const std::string& fileName) {
    _trace = _vcdParser.parse_file(fileName);
    if (_trace == nullptr) {
        fmt::print("ERROR: _trace is nullptr !!");
        return false;
    }
    parseAllSignals();
    return true;
}

/**
 * @brief
 *
 */
void VCDMgr::parseAllSignals() {
    // Create our container for the VCD Manager
    _times = _trace->get_timestamps();
    _scopes = _trace->get_scopes();
    _signals = _trace->get_signals();

    // Create the Signal Reference to VCDSignal* hash map
    // Key : std::string
    // Val : std::vector<VCDSignal*>
    for (size_t i = 0, n = getSignals()->size(); i < n; ++i) {
        VCDSignal* signal = getSignal(i);
        VCDSignalReference ref = signal->reference;
        _refToSignal[ref].push_back(signal);
    }

    // Solve the signal->scope == nullptr issue
    for (VCDScope* scope : *_trace->get_scopes()) {
        for (VCDSignal* signal : scope->signals) {
            signal->scope = scope;
        }
    }
}

/**
 * @brief
 *
 * @param ref
 * @return VCDSignal*
 */
VCDSignal* VCDMgr::getRefenceSignal(const std::string& ref) {
    if (_refToSignal.find(ref) != _refToSignal.end())
        if (_refToSignal[ref].size() == 0)
            return nullptr;
    return _refToSignal[ref][0];
}

/**
 * @brief
 *
 * @param val
 * @return long long int
 */
Decimal VCDMgr::getDecValue(VCDValue* const val) const {
    VCDValueType type = val->get_type();
    if (type == VCDValueType::VCD_REAL)
        return val->get_value_real();
    if (type == VCDValueType::VCD_SCALAR)
        return val->get_value_bit();

    std::string strVal = "";
    VCDBitVector* vecVal = val->get_value_vector();

    // Bitwidth is greater than 32 bits
    if (vecVal->size() > 32) return 0;

    for (VCDBitVector::iterator it = vecVal->begin(); it != vecVal->end(); ++it) {
        char c = VCDBit2Char(*it);
        if (c == 'X' || c == 'Z')
            return 0;
        strVal += c;
    }
    return (std::stoll(strVal, nullptr, 2));
}

/**
 * @brief
 *
 * @param signal
 * @param time
 * @return VCDValue*
 */
VCDValue* VCDMgr::getSignalValueAt(VCDSignal* signal, const VCDTime time) const {
    return _trace->get_signal_value_at(signal->hash, time);
}

/**
 * @brief
 *
 * @param hash
 * @param time
 * @return VCDValue*
 */
VCDValue* VCDMgr::getSignalValueAt(const VCDSignalHash& hash, const VCDTime time) const {
    return _trace->get_signal_value_at(hash, time);
}

/**
 * @brief
 *
 * @param hash
 * @return VCDSignalValues*
 */
VCDSignalValues* VCDMgr::getSignalDeltaValue(const VCDSignalHash& hash) const {
    return _trace->get_signal_values(hash);
}
