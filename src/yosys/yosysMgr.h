#ifndef GV_YOSYS_MGR
#define GV_YOSYS_MGR

#include "kernel/yosys.h"

using namespace std;
USING_YOSYS_NAMESPACE

class YosysMgr;
extern YosysMgr* yosysMgr;

class YosysMgr {
public:
    YosysMgr() { init(); }
    ~YosysMgr() {}

    void init();
    void reset();

    void setLogging(const bool& = false);
    void createMapping(const string&);
    void printDesignInfo(const bool&);
    void showSchematic();
    void readBlif(const string&);
    void readVerilog(const string&);
    void readAiger(const string&);
    void writeBlif(const string&);
    void writeAiger(const string&);

private:
};

#endif
