#ifndef ABC_EXT_H
#define ABC_EXT_H

#include <string>

#include "base/abc/abc.h"
#include "base/main/mainInt.h"
#include "gvType.h"

/**
 * @brief ABC structure type.
 *
 */
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
typedef struct Pdr_Par_t_ Pdr_Par_t;

/**
 * @brief Extern functions under the circuit manager
 *
 */
extern "C" {
    void Abc_Start();
    void Abc_Stop();
    int Cmd_CommandExecute(Abc_Frame_t* pAbc, const char* sCommand);
    Abc_Frame_t* Abc_FrameGetGlobalFrame();
    Abc_Ntk_t* Io_Read(char* pFileName, Io_FileType_t FileType, int fCheck, int fBarBufs);
    Abc_Ntk_t* Abc_FrameReadNtk(Abc_Frame_t* p);
    Gia_Man_t* Wln_BlastSystemVerilog(char* pFileName, char* pTopModule, char* pDefines, int fSkipStrash, int fInvert, int fTechMap, int fLibInDir, int fVerbose);
    int Abc_NtkDarPdr(Abc_Ntk_t* pNtk, Pdr_Par_t* pPars);
}

/**
 * @brief Extern functions under the circuit manager
 *
 */
extern void initCirMgr(const int& piNum, const int& poNum, const int& regNum, const int& totNum);
extern bool inputIsPi(const int& gateId);
extern void parseInput(const int& idx, const int& gateId);
extern void parseOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, std::string poName);
extern void parseAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv);
extern void parseRi(const int& idx, const int& gateId, const int& in0Id, const int& inv);
extern void parseRiRo(const int& riGid, const int& roGid);
extern int parseRo(const int& idx, const int& gateId, const FileType& fileType);
extern void parseConst0();
extern void parseConst1();
extern unsigned getNumPIs();
extern unsigned getNumPOs();
extern unsigned getNumLATCHs();
extern unsigned getNumAIGs();
extern unsigned getAigIn0Gid(const unsigned& idx);
extern unsigned getAigIn1Gid(const unsigned& idx);
extern unsigned getPoIn0Gid(const unsigned& idx);
extern unsigned getRiIn0Gid(const unsigned& idx);
extern int getAigIn0Cp(const unsigned& idx);
extern int getAigIn1Cp(const unsigned& idx);
extern int getPoIn0Cp(const unsigned& idx);
extern int getRiIn0Cp(const unsigned& idx);

#endif
