#ifndef GV_ABC_NTK_H
#define GV_ABC_NTK_H

#include <vector>
using namespace std;

class AbcMgr;
class NtkMgr;
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
typedef struct Abc_Obj_t_ Abc_Obj_t;
typedef struct Aig_Obj_t_ Aig_Obj_t;


class NtkMgr
{
    public:
        NtkMgr(Abc_Ntk_t* ntk) { init(ntk); }
        ~NtkMgr() {}

        void init(Abc_Ntk_t* ntk);

        // DFS
        unsigned*   Aig_ObjDFS(Aig_Obj_t*) const;
        void        Aig_AllDFS();

        // FRAIG
        void        sweep();
        void        optimize();
        void        strash();
        void        fraig();

        // Simulation
        void        randomSim();

        // FEC
        void        initFEC();
        void        checkFEC();

        // Rewiring
        void        rewireByFanins(Aig_Obj_t*, Aig_Obj_t*, Aig_Obj_t*);

        // Print Information
        void        printSummary() const;
        void        printPIs() const;
        void        printPOs() const;
        void        printNetlist(Aig_Obj_t*) const;
        Abc_Ntk_t * getNtk() { return pNtk; }

    
    private:
        Abc_Ntk_t*               pNtk;
        static AbcMgr*           _AbcMgr;

        // for AIG
        Aig_Obj_t**              PI_List;
        Aig_Obj_t**              PO_List; 
        unsigned*                DFS_List;    
        unsigned                 Total_num;
        unsigned                 PI_num;
        unsigned                 PO_num;
        unsigned                 Node_num; 
};


#endif