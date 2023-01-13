#ifndef GV_MOD_MGR
#define GV_MOD_MGR

#include "gvCmdMgr.h"
#include <string>
#include <vector>
using namespace std;

class GVModMgr;

const string GVEngineString[] = {"yosys", "abc"};

const string GVModTypeString[] = {"gv", "SETUP", "vrf"};

enum GVModType
{
    GV_MOD_TYPE_NONE   = 0,
    GV_MOD_TYPE_SETUP  = 1,
    GV_MOD_TYPE_VERIFY = 2
};

enum GVModEngine
{
    GV_MOD_ENGINE_YOSYS = 0,
    GV_MOD_ENGINE_ABC   = 1
};

extern GVModMgr* gvModMgr;

class GVModMgr
{
    public:
        GVModMgr();
        ~GVModMgr();
        // get functions
        bool        getInputFileExist();
        string      getInputFileName();
        string      getAigFileName();
        string      getTopModuleName();
        GVModType   getGVMode();
        GVModEngine getGVEngine();
        string      getModPrompt();
        int         getSafe();
        bool        getWizard() { return _wizard; };

        // set functions
        void setInputFileExist(bool exist);
        void setInputFileName(string& filename);
        void setAigFileName(string aigFileName);
        void setTopModuleName(string topModuleName);
        void setGVMode(GVModType mode);
        void setGVEngine(GVModEngine engine);
        void setModPromt();
        void setSafe(int p);
        void setWizard(bool wiz) { _wizard = wiz; };
        void setWizardContent(string prompt);

        // GV tutorial wizard
        void printWizardPrompt(int promptStart, int promptLength);
        void printWizardProgress(int pos, int promptNum);

    private:
        bool              _inputFileExist;
        string            _inputFileName;
        string            _aig_name;
        string            _modPrompt;
        string            _top_module_name;
        GVModType         _gvMode;
        GVModEngine       _gvEng;
        vector<GVCmdType> _vrfMode;
        vector<GVCmdType> _setupMode;
        vector<string>    _wizardContent;
        int               _property;
        bool              _propertySet;
        bool              _wizard;
};

#endif
