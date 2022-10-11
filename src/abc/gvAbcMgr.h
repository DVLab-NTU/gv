#ifndef GV_ABC_MGR
#define GV_ABC_MGR

#include "gvAbcNtk.h"
#include "gvAigMgr.h"
#include <string>
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

using namespace std;

class AbcMgr;
extern AbcMgr* abcMgr;
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;

extern "C"
{
    void   Abc_Start();
    void   Abc_Stop();
    Abc_Frame_t * Abc_FrameGetGlobalFrame();
    int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );
    Abc_Ntk_t * Abc_FrameReadNtk( Abc_Frame_t * p );
}


class AbcMgr
{
    public:
        AbcMgr() { init(); }
        ~AbcMgr() { reset(); }

        void init();
        void reset();

        void abcReadDesign(string&);
        void abcNtk2Aig();

        Abc_Frame_t * get_Abc_Frame_t() { return pAbc; }
        abcNtkMgr *   get_abcNtkMgr() { return pNtkMgr; }
        abcAigMgr *   get_aigNtkMgr() { return pAigMgr; }


    private:
        Abc_Frame_t *    pAbc; 
        abcNtkMgr *      pNtkMgr;
        abcAigMgr *      pAigMgr;
};


#endif
