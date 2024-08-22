#ifndef GV_ABC_MGR
#define GV_ABC_MGR

#include <cstddef>
#include <map>
#include <string>

#include "abcExt.h"
#include "cirDef.h"

using namespace std;

class AbcMgr;
extern AbcMgr* abcMgr;

struct ABCParams {
    ABCParams() : fInvert(0), fTechMap(0), fSkipStrash(0), fVerbose(0), fGiaSimple(0),
                  fCheck(0), fLibInDir(0), pTopModule(nullptr), pDefines(nullptr), pFileName(new char[100]){};
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
    void readAiger(const ABCParams&);
    void readSeqVerilog(const ABCParams&);
    void readCombVerilog(const ABCParams&);
    void readVerilogNew(const ABCParams&);
    void buildAigName(map<unsigned, string>&);
    void travPreprocess();
    void travAllObj(const FileType&, map<unsigned, string>);
    void giaToCir(const FileType&, map<unsigned, string>);
    void initCir(const FileType&);
    void cirToGia();
    void cirToAig(IDMap&);
    void execCmd(const char*);

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
