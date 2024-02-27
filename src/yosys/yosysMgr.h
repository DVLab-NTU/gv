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
    YosysMgr() : _fileType(VERILOG), _property(-1) { init(); }
    ~YosysMgr() {}

    void init();
    void reset();

    void saveDesign(const string&);
    void loadDesign(const string&);
    void loadSimPlugin();

    void readBlif(const string&);
    void readVerilog(const string&);
    void readAiger(const string&);
    void writeBlif(const string&);
    void writeAiger(const string&);

    void showSchematic();
    void printDesignInfo(const bool& = false);
    void createMapping(const string&);
    void runPass(const string&);

    void setLogging(const bool& = false);
    void setSafeProperty(const unsigned& p = 0) { _property = p; };
    unsigned getSafeProperty() const { return _property; };
    string getTopModuleName() const;

private:
    FileType _fileType;
    unsigned _property;
};

#endif
