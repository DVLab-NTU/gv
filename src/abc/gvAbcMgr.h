#ifndef GV_ABC_MGR
#define GV_ABC_MGR

#include <cstddef>
#include <map>
#include <string>

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "cirGate.h"
#include "gvAbcNtk.h"

using namespace std;

class AbcMgr;
extern AbcMgr* abcMgr;
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;

extern "C"
{
    void Abc_Start();
    void Abc_Stop();
    int Cmd_CommandExecute(Abc_Frame_t* pAbc, const char* sCommand);
    Abc_Frame_t* Abc_FrameGetGlobalFrame();
    Abc_Ntk_t* Abc_FrameReadNtk(Abc_Frame_t* p);
    Gia_Man_t* Wln_BlastSystemVerilog(char* pFileName, char* pTopModule, char* pDefines, int fSkipStrash, int fInvert, int fTechMap, int fVerbose);
}

struct ABCParam {
    ABCParam() : fBlast(0), fInvert(0), fTechMap(0), fSkipStrash(0), fCollapse(0), fVerbose(0), fGiaSimple(0), fCheck(0), pTopModule(NULL), pDefines(NULL) {
        pFileName = new char[100];
    };
    char* pFileName;
    char* pTopModule;
    char* pDefines;
    int fBlast;
    int fInvert;
    int fTechMap;
    int fSkipStrash;
    int fCollapse;
    int fVerbose;
    int fGiaSimple;
    int fCheck;
};

class AbcMgr {
public:
    AbcMgr();
    ~AbcMgr() { reset(); }

    void init();
    void reset();
    void abcReadDesign(string&);
    void readAig(const ABCParam&);
    void readVerilog(const ABCParam&);
    void buildAigName(map<unsigned, string>&);
    void travPreprocess();
    void travAllObj(vector<CirPiGate*>& piList, vector<CirPoGate*>& poList, vector<CirRoGate*>& roList, vector<CirRiGate*>& riList,
                    vector<CirGate*>& totGateList, map<unsigned, string>& id2Name, const CirFileType& fileType);

    Abc_Frame_t* get_Abc_Frame_t() { return pAbc; }
    abcNtkMgr* get_abcNtkMgr() { return pNtkMgr; }
    Gia_Man_t* pGia;

private:
    Abc_Frame_t* pAbc;
    abcNtkMgr* pNtkMgr;
};

#endif
