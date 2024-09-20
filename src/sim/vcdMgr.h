#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "VCDFileParser.hpp"
#include "VCDValue.hpp"

namespace gv {
namespace sim {
class VCDMgr;
}
}  // namespace gv

extern gv::sim::VCDMgr* vcdMgr;

struct PrintStatus {
    int colLimit;
    int rowLimit;
    int signalCount;
    bool isExceed;
};

typedef std::unordered_map<std::string, std::vector<VCDSignal*>> RefHash;
typedef std::unordered_map<std::string, char> BinToHexHash;
typedef std::string Hexadecimal;

namespace gv {
namespace sim {

class VCDMgr {
public:
    VCDMgr();
    ~VCDMgr();

    bool readVCDFile(const std::string& fileName);
    void parseAllSignals();

    void printScope(const VCDScope* const scope, VCDTime time);
    void printTimedSignal(VCDSignal* const signal, const VCDTime& time);
    void printSignal(std::vector<std::string> signals);
    void printAllSignals();
    void printAllSignalInfo(const size_t& num = 0);
    bool printSignalInfo(const VCDSignal* const signal, const size_t& idx);

    void setColLimit(const int& col) { _colLimit = col; };
    void setRowLimit(const int& row) { _rowLimit = row; };

    Hexadecimal getHexValue(VCDValue* const val) const;
    size_t getSignalWidth(VCDSignal* const signal) const {
        return signal->size;
    }
    size_t getNumTimeStamps() { return (*_times).size(); };
    size_t getNumSignals() { return (*_signals).size(); };
    size_t getNumScopes() { return (*_scopes).size(); };
    std::string getSignalName(const VCDSignal* const s) { return s->hash; }
    VCDTime getTimeStamp(const size_t& idx) const { return (*_times)[idx]; }
    VCDScope* getScope(const size_t& idx) const { return (*_scopes)[idx]; }
    VCDSignal* getSignal(const size_t& idx) const { return (*_signals)[idx]; }
    VCDSignal* getRefenceSignal(const std::string& ref);
    std::vector<VCDTime>* getTimeStamps() const { return _times; }
    std::vector<VCDScope*>* getScopes() const { return _scopes; };
    std::vector<VCDSignal*>* getSignals() const { return _signals; };
    VCDValue* getSignalValueAt(VCDSignal* signal, const VCDTime time) const;
    VCDValue* getSignalValueAt(const VCDSignalHash& hash,
                               const VCDTime time) const;
    VCDSignalValues* getSignalDeltaValue(const VCDSignalHash& hash) const;

private:
    void resetStatus();
    void incSigCount() { ++_printStatus.signalCount; };
    size_t getColLimit() const { return _printStatus.colLimit; };
    size_t getRowLimit() const { return _printStatus.rowLimit; };
    int getSigCount() const { return _printStatus.signalCount; };
    bool getisExceed() const { return _printStatus.isExceed; };
    void printTableRowTitle(const size_t& colNum) const;

    VCDFile* _trace;
    VCDFileParser _vcdParser;
    std::vector<VCDTime>* _times;
    std::vector<VCDScope*>* _scopes;
    std::vector<VCDSignal*>* _signals;
    PrintStatus _printStatus;
    RefHash _refToSignal;
    int _colLimit;
    int _rowLimit;
};

}  // namespace sim
}  // namespace gv
