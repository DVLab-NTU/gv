#ifndef GV_MOD_MGR
#define GV_MOD_MGR

#include <string>
#include <vector>

#include "gvCmdMgr.h"
using namespace std;

class GVModMgr;

const string GVModTypeString[] = {"gv", "setup", "vrf"};

enum GVModType {
    GV_MOD_TYPE_NONE   = 0,
    GV_MOD_TYPE_SETUP  = 1,
    GV_MOD_TYPE_VERIFY = 2
};

enum GVModEngine {
    GV_MOD_ENGINE_YOSYS = 0,
    GV_MOD_ENGINE_ABC   = 1
};

extern GVModMgr* gvModMgr;

class GVModMgr {
public:
    GVModMgr();
    ~GVModMgr(){};
    // get functions
    string getModPrompt() {
        setModPromt();  // update mode prompt
        return _modPrompt;
    };
    string getInputFileName() { return _inputFileName; };
    string getAigFileName() { return _aig_name; };
    int getSafe() { return _property; }

    // set functions
    void setAigFileName(const string& aigFileName) { _aig_name = aigFileName; };
    void setTopModuleName(string topModuleName);
    void setGVMode(GVModType mode);
    void setGVEngine(GVModEngine engine);
    void setModPromt();
    void setSafe(int p);

    // reset
    void reset();

private:
    string _inputFileName;
    string _aig_name;
    string _modPrompt;
    GVModType _gvMode;
    int _property;
    bool _propertySet;
};

#endif
