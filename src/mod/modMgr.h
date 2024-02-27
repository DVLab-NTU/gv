#ifndef MOD_MGR
#define MOD_MGR

#include <string>

using namespace std;

class ModMgr;

const string ModTypeString[] = {"gv", "setup", "vrf"};

enum ModType {
    MOD_TYPE_NONE   = 0,
    MOD_TYPE_SETUP  = 1,
    MOD_TYPE_VERIFY = 2
};
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

    // set functions
    void setGVMode(const ModType& = ModType::MOD_TYPE_SETUP);
    void setModPromt();

private:
    string _modPrompt;
    ModType _gvMode;
};

#endif
