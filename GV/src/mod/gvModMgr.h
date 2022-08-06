#ifndef GV_MOD_MGR
#define GV_MOD_MGR

#include <string>

using namespace std;

class GVModMgr;

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

        bool         getInputFileExist();
        GVModType    getGvMode();
        GVModEngine  getGvEngine();

        void setInputFileExist(bool exist);
        void setGvMode(GVModType mode);
        void setGvEngine(GVModEngine engine);

    private:
        bool          _inputFileExist;
        GVModType     _gvMode;
        GVModEngine   _gvEng;

        
};


#endif
