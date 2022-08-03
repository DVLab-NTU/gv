#ifndef AOCMD_C
#define AOCMD_C

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "base/ver/ver.h"
#include "base/io/ioAbc.h"
#include "base/abc/abc.h"
#include "base/main/mainInt.h"
#include "aig/gia/gia.h"

extern Ver_Man_t * pMan;
extern Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
extern Gia_Man_t * Gia_ManFromAig( Aig_Man_t * p );
extern Gia_Man_t * Gia_PolynCoreDetectTest( Gia_Man_t * pGia, int fAddExtra, int fAddCones, int fVerbose );
extern Vec_Int_t * Ree_ManComputeCuts( Gia_Man_t * p, Vec_Int_t ** pvXors, int fVerbose );
static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Cut_CommandSim(Abc_Frame_t* pAbc, int argc, char** argv);

void Lsv_NtkPrintNodes(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
      Abc_ObjName(pFanin));
    }
    if (Abc_NtkHasSop(pNtk)) {
      printf("The SOP of this node:\n%s", (char*)pObj->pData);
    }
  }
}

void Cut_sim(Abc_Frame_t* pAbc, Abc_Ntk_t* pNtk) {

  Aig_Obj_t* pObj;
  Hash_IntMan_t * pHash = Hash_IntManStart( 1000 );
  int i, c, fAddExtra = 0, fAddCones = 0, fVerbose = 0, fVeryVerbose = 0;
  Abc_Ntk_t* pStrashedNtk = Abc_NtkStrash(pNtk, 0, 1, 0);
  Aig_Man_t* pAig = Abc_NtkToDar(pStrashedNtk, 0, 1);
  pAbc->pGia = Gia_ManFromAig( pAig );
  Vec_Int_t * vAdds = Ree_ManComputeCuts( pAbc->pGia, NULL, 1);
  
}

int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintNodes(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

int Cut_CommandSim(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Cut_sim(pAbc, pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: cut_sim [-h]\n");
  Abc_Print(-2, "\t        cut simulation\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}


void AO_Init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 1);
  Cmd_CommandAdd(pAbc, "CUT", "cut_sim", Cut_CommandSim, 1);
}

void AO_End(Abc_Frame_t* pAbc) {

}

#endif

