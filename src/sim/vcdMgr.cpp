
#include "vcdMgr.h"

#include <fmt/core.h>

#include <cstddef>
#include <string>

#include "VCDParser.hpp"
#include "VCDValue.hpp"

//! Global VCD Manager pointer
VCDMgr* vcdMgr = nullptr;

//! unordered map to convert 4-bit binary strings to hexadecimal characters
BinToHexHash binToHexHash = {
    {"0000", '0'},
    {"0001", '1'},
    {"0010", '2'},
    {"0011", '3'},
    {"0100", '4'},
    {"0101", '5'},
    {"0110", '6'},
    {"0111", '7'},
    {"1000", '8'},
    {"1001", '9'},
    {"1010", 'A'},
    {"1011", 'B'},
    {"1100", 'C'},
    {"1101", 'D'},
    {"1110", 'E'},
    {"1111", 'F'}
};

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

/**
 * @brief
 *
 * @param valBitVec
 * @return std::string
 */
static std::string VCDVecToBinary(VCDBitVector* valBitVec) {
    std::string binaryString = "";
    for (VCDBitVector::iterator it = valBitVec->begin(); it != valBitVec->end();
         ++it) {
        char c = VCDBit2Char(*it);
        if (c == 'X' || c == 'Z') return 0;
        binaryString += c;
    }
    return binaryString;
}

/**
 * @brief Convert a binary string to a hexadecimal string.
 *
 * @param binary A binary string.
 * @return The corresponding hexadecimal string.
 */
static std::string binaryToHexString(const std::string& binary) {
    std::string hexString = "";

    // Pad the binary string with leading zeros if its length is not a multiple
    // of 4
    int padding = (4 - binary.size() % 4) % 4;
    std::string paddedBinary = std::string(padding, '0') + binary;

    // Convert each 4-bit segment to its hexadecimal equivalent
    for (size_t i = 0; i < paddedBinary.size(); i += 4) {
        std::string fourBits = paddedBinary.substr(i, 4);
        hexString += binToHexHash[fourBits];
    }

    return hexString;
}

VCDMgr::VCDMgr() : _colLimit(15), _rowLimit(20) {}

VCDMgr::~VCDMgr() {}

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
        if (_refToSignal[ref].size() == 0) return nullptr;
    return _refToSignal[ref][0];
}

/**
 * @brief
 *
 * @param val
 * @return long long int
 */
Hexadecimal VCDMgr::getHexValue(VCDValue* const val) const {
    VCDValueType type = val->get_type();
    if (type == VCDValueType::VCD_REAL)
        return "R";
    if (type == VCDValueType::VCD_SCALAR)
        return std::string(1, VCDBit2Char(val->get_value_bit()));

    std::string strVal = "";
    VCDBitVector* vecVal = val->get_value_vector();
    // Bitwidth is greater than 32 bits
    if (vecVal->size() > 32) return 0;
    std::string binary = VCDVecToBinary(vecVal);
    return binaryToHexString(binary);
}

/**
 * @brief
 *
 * @param signal
 * @param time
 * @return VCDValue*
 */
VCDValue* VCDMgr::getSignalValueAt(VCDSignal* signal,
                                   const VCDTime time) const {
    return _trace->get_signal_value_at(signal->hash, time);
}

/**
 * @brief
 *
 * @param hash
 * @param time
 * @return VCDValue*
 */
VCDValue* VCDMgr::getSignalValueAt(const VCDSignalHash& hash,
                                   const VCDTime time) const {
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
