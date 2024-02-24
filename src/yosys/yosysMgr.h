#ifndef GV_YOSYS_MGR
#define GV_YOSYS_MGR

#include "fileType.h"
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
    void saveDesign(const string&);
    void loadDesign(const string&);
    void createMapping(const string&);

    void readBlif(const string&);
    void readVerilog(const string&);
    void readAiger(const string&);
    void writeBlif(const string&);
    void writeAiger(const string&);

    void showSchematic();
    void printDesignInfo(const bool&);

private:
    FileType _fileType;
};

#endif
