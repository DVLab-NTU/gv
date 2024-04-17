#ifndef MOD_MGR
#define MOD_MGR

#include <string>

#include "gvType.h"

using namespace std;

class ModMgr;

const string ModTypeString[] = {"gv", "setup", "vrf", "app"};

extern ModMgr* modeMgr;

class ModMgr {
public:
    ModMgr();
    ~ModMgr(){};
    // reset
    void reset();

    // get functions
    string getModPrompt() {
        setModPromt();  // update mode prompt
        return _modPrompt;
    };
    inline ModType getCurrMode() { return _gvMode; };

    // set functions
    void setGVMode(const ModType& = ModType::MOD_TYPE_SETUP);
    void setModPromt();

private:
    string _modPrompt;
    ModType _gvMode;
};

#endif
