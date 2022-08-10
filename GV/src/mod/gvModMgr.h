#ifndef GV_MOD_MGR
#define GV_MOD_MGR

#include <string>

using namespace std;

class GVModMgr;


const string GVEngineString[] = {
    "yosys",
    "abc",
    "v3"
};

const string GVModTypeString[] = {
    "gv",
    "SETUP",
    "vrf"
};

enum GVModType{
   GV_MOD_TYPE_NONE = 0,
   GV_MOD_TYPE_SETUP = 1,
   GV_MOD_TYPE_VERIFY = 2
};

enum GVModEngine{
    GV_MOD_ENGINE_YOSYS = 0,
    GV_MOD_ENGINE_ABC = 1,
    GV_MOD_ENGINE_V3 = 2
};

extern GVModMgr*     gvModMgr;

class GVModMgr
{
    public:
        GVModMgr();
        ~GVModMgr();
//get functions
        bool         getInputFileExist();
        string       getInputFileName();
        string       getAigFileName();
        GVModType    getGVMode();
        GVModEngine  getGVEngine();
        string       getModPrompt();
//set functions
        void         setInputFileExist(bool exist);
        void         setInputFileName(string& filename);
        void         setAigFileName(string aigFileName);
        void         setGVMode(GVModType mode);
        void         setGVEngine(GVModEngine engine);
        void         setModPromt();

    private:
        bool          _inputFileExist;
        string        _inputFileName;
        string        _aig_name;
        string        _modPrompt;
        GVModType     _gvMode;
        GVModEngine   _gvEng;
};


#endif
