#ifndef GV_ABC_NTK_C
#define GV_ABC_NTK_C

#include <iostream>
#include <iomanip>
#include "gvAbcNtk.h"
#include "gvAbcMgr.h"

AbcMgr* NtkMgr::_AbcMgr = 0;

extern "C"
{
    unsigned*   GV_NtkBasicInfo(Abc_Ntk_t* pNtk);
    Aig_Obj_t** GV_NtkGetPiList(Abc_Ntk_t* pNtk);
    Aig_Obj_t** GV_NtkGetPoList(Abc_Ntk_t* pNtk);
    unsigned    GV_ObjId(Aig_Obj_t* pObj);
    void        GV_ObjDFS(Aig_Obj_t* pObj);
    unsigned*   GV_AllDFS(Abc_Ntk_t* pNtk);
}

void NtkMgr::init(Abc_Ntk_t* ntk)
{
    unsigned* info = GV_NtkBasicInfo(ntk);
    PI_List =  GV_NtkGetPiList(ntk);
    PO_List =  GV_NtkGetPoList(ntk);
    pNtk = ntk;
    Total_num = info[0];
    PI_num = info[1];
    PO_num = info[2];
    Node_num = info[3];
    // cout << Total_num << "  " << PI_num << "  " << PO_num << "  " << AIG_num << endl;
}

void NtkMgr::printSummary() const
{
  cout << endl;
  cout << "Circuit Statistics" << endl;
  cout << "==================" << endl;
  cout << "  PI  " << setw(10) << right << PI_num << endl;
  cout << "  PO  " << setw(10) << right << PO_num << endl;
  cout << "  Node " << setw(10) << right << Node_num << endl;
  cout << "------------------" <<endl;
  cout << "  Total" << setw(10) << right << PI_num+PO_num+Node_num << endl;

}

void NtkMgr::printPIs() const
{
    cout << "PIs of the AIG:";
    for(int i = 0; i < PI_num; ++i) {
        std::cout << " " << GV_ObjId(PI_List[i]);
    }
    cout << endl;
}

void NtkMgr::printPOs() const
{
    cout << "POs of the AIG:";
    for(int i = 0; i < PO_num; ++i) {
        std::cout << " " << GV_ObjId(PO_List[i]);
    }
    cout << endl;
}

void NtkMgr::Aig_AllDFS()
{
    DFS_List = GV_AllDFS(pNtk);
    for(int i = 0; i < Total_num; ++i) {
        std::cout << " " << DFS_List[i];
    }
    cout << endl;
}



#endif