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
    void createMapping(const string& fileName);

private:
};

#endif
