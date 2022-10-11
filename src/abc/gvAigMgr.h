#ifndef GV_AIG_MGR_H
#define GV_AIG_MGR_H

#include <vector>
#include "gvAbcNtk.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

using namespace std;

class abcAigMgr;

typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
typedef struct Aig_Obj_t_ Aig_Obj_t;

class abcAigMgr
{
    public:
        abcAigMgr(Abc_Frame_t* pAbc, Abc_Ntk_t* ntk) : pNtk(ntk), pAbc(pAbc) { init(); }
        ~abcAigMgr() {}

        void init();

        Aig_Obj_t**     Aig_NodesDFS(Aig_Obj_t**, int) const;
        void            Aig_AllDFS();
        void            fraig();
        void            randomSim();
        void            simTraversal( Vec_Ptr_t*, size_t*);
        void            rewireByFanins(Aig_Obj_t*, Aig_Obj_t*, Aig_Obj_t*);

        void            printSummary(bool) const;
        void            printPIs() const;
        void            printPOs() const;
        void            printNetlist(Aig_Obj_t*) const;
        Aig_Obj_t*      getObjbyId(int id) {return Aig_ManObj(pMan, id);}
        Abc_Ntk_t*      getNtk() { return pNtk; }
        Aig_Obj_t**     getPIList() { return PI_List;}    
        Aig_Obj_t**     getPOList() { return PO_List;} 

    
    private:
        Abc_Frame_t*             pAbc;
        Abc_Ntk_t*               pNtk;
        Aig_Man_t*               pMan;

        // for AIG
        Aig_Obj_t**              PI_List;    
        Aig_Obj_t**              PO_List; 
        Aig_Obj_t**              DFS_List;
        unsigned                 Total_num;
        unsigned                 PI_num;
        unsigned                 PO_num;
        unsigned                 Node_num; 
};
#endif