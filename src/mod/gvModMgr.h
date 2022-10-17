#ifndef GV_MOD_MGR
#define GV_MOD_MGR

#include <string>
#include <vector>
using namespace std;

class GVModMgr;

// Command Categories Enum
enum GVCmdType
{
   // Revealed command
   GV_CMD_TYPE_REVEALED    = 0,
   GV_CMD_TYPE_COMMON      = 1,
   GV_CMD_TYPE_VERIFY      = 2,
   GV_CMD_TYPE_SIMULATE    = 3,
   GV_CMD_TYPE_NETWORK     = 4,
   GV_CMD_TYPE_ABC         = 5,
   GV_CMD_TYPE_MOD         = 6,
};

const string GVEngineString[] = {
    "yosys",
    "abc"
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
    GV_MOD_ENGINE_ABC = 1
};

extern GVModMgr*     gvModMgr;

class GVModMgr
{
    public:
        GVModMgr();
        ~GVModMgr();
        bool checkModeType(GVCmdType& currCmdType);
//get functions
        bool         getInputFileExist();
        string       getInputFileName();
        string       getAigFileName();
        string       getTopModuleName();
        GVModType    getGVMode();
        GVModEngine  getGVEngine();
        string       getModPrompt();
        int          getSafe();
//set functions
        void         setInputFileExist(bool exist);
        void         setInputFileName(string& filename);
        void         setAigFileName(string aigFileName);
        void         setTopModuleName(string topModuleName);
        void         setGVMode(GVModType mode);
        void         setGVEngine(GVModEngine engine);
        void         setModPromt();
        void         setSafe(int p);

    private:

        bool               _inputFileExist;
        string             _inputFileName;
        string             _aig_name;
        string             _modPrompt;
        string             _top_module_name; 
        GVModType          _gvMode;
        GVModEngine        _gvEng;
        vector<GVCmdType>  _vrfMode;
        vector<GVCmdType>  _setupMode;
        int                _property;
        bool               _propertySet;
};


#endif
