#ifndef GV_ABC_NTK_H
#define GV_ABC_NTK_H

#include <vector>
using namespace std;

class AbcMgr;
class abcNtkMgr;
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_   Abc_Ntk_t;
typedef struct Abc_Obj_t_   Abc_Obj_t;

class abcNtkMgr
{
    public:
        abcNtkMgr(Abc_Ntk_t* ntk) : pNtk(ntk) { init(); }
        ~abcNtkMgr() {}

        void init();

        void sweep(bool);

        void       printSummary() const;
        Abc_Ntk_t* getNtk() { return pNtk; }
        void       abcQBF(int, int, bool);

    private:
        Abc_Ntk_t*     pNtk;
        static AbcMgr* _AbcMgr;

        vector<Abc_Obj_t*> PI_List;
        vector<Abc_Obj_t*> PO_List;
        vector<Abc_Obj_t*> DFS_List;
        unsigned           Total_num;
        unsigned           PI_num;
        unsigned           PO_num;
        unsigned           Node_num;
};
#endif