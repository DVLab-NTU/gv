#ifndef GV_ABC_MGR
#define GV_ABC_MGR

#include <cstddef>
#include <map>
#include <string>

#include "abcExt.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "cirDef.h"
#include "util.h"

using namespace std;

class AbcMgr;
extern AbcMgr* abcMgr;
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
typedef struct Pdr_Par_t_ Pdr_Par_t;

extern "C"
{
    void Abc_Start();
    void Abc_Stop();
    int Cmd_CommandExecute(Abc_Frame_t* pAbc, const char* sCommand);
    Abc_Frame_t* Abc_FrameGetGlobalFrame();
    Abc_Ntk_t* Abc_FrameReadNtk(Abc_Frame_t* p);
    Gia_Man_t* Wln_BlastSystemVerilog(char* pFileName, char* pTopModule, char* pDefines, int fSkipStrash, int fInvert, int fTechMap, int fLibInDir, int fVerbose);
    int Abc_NtkDarPdr(Abc_Ntk_t* pNtk, Pdr_Par_t* pPars);
}

struct ABCParam {
    ABCParam() : fInvert(0), fTechMap(0), fSkipStrash(0), fVerbose(0), fGiaSimple(0),
                 fCheck(0), fLibInDir(0), pTopModule(NULL), pDefines(NULL) {
        pFileName = new char[100];
    };
    char* pFileName;
    char* pTopModule;
    char* pDefines;
    int fInvert;
    int fTechMap;
    int fSkipStrash;
    int fVerbose;
    int fGiaSimple;
    int fLibInDir;
    int fCheck;
};

class AbcMgr {
public:
    AbcMgr();
    ~AbcMgr();

    void init();
    void reset();
    void readAig(const ABCParam&);
    void readVerilog(const ABCParam&);
    void buildAigName(map<unsigned, string>&);
    void travPreprocess();
    void travAllObj(const FileType&, map<unsigned, string>);
    void initCir(const FileType&);
    void cirToGia();
    void cirToAig(IDMap&);
    void execCmd(char*);

    // Verification command
    void runPDR(const bool&);
    void writeBlif(const string&);

    Abc_Frame_t* get_Abc_Frame_t() { return pAbc; }
    Gia_Man_t* pGia;

private:
    Abc_Frame_t* pAbc;
    Abc_Ntk_t* pNtk;
};

#endif
