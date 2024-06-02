#include <string>
#include <vector>

#include "VCDFileParser.hpp"
#include "VCDValue.hpp"

class VCDMgr {
public:
    VCDMgr(){};
    ~VCDMgr(){};

    void readVCDFile();
    void printVCDFile();

    void getValue();

private:
    VCDFileParser _vcdParser;
    VCDFile* _trace;
};