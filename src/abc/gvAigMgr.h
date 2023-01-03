#ifndef GV_AIG_MGR_H
#define GV_AIG_MGR_H

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "gvAbcNtk.h"
#include <vector>

using namespace std;

class abcAigMgr;

typedef struct Abc_Frame_t_          Abc_Frame_t;
typedef struct Abc_Ntk_t_            Abc_Ntk_t;
typedef struct Abc_Obj_t_            Abc_Obj_t;
typedef struct Aig_Obj_t_            Aig_Obj_t;
typedef struct Aig_ManCut_t_         Aig_ManCut_t;
typedef struct Aig_Cut_t_            Aig_Cut_t;
typedef pair<Aig_Cut_t*, Aig_Cut_t*> cut_pair_t;
typedef pair<float, float> similarity_t;
typedef pair<int, float> obj_similarity_t; // The first element is similarity, the second element is node Id
typedef vector<obj_similarity_t> similarity_vec_t;

static inline int          Aig_ManCandNum( Aig_Man_t * p )        { return p->nObjs[AIG_OBJ_CI] + p->nObjs[AIG_OBJ_AND]+p->nObjs[AIG_OBJ_EXOR];  }
// iterators over all nodes
#define Aig_ManForEachCand( p, pObj, i )                                        \
    Vec_PtrForEachEntry( Aig_Obj_t *, p->vObjs, pObj, i ) if ( (pObj) == NULL || (!Aig_ObjIsNode(pObj) &&  !Aig_ObjIsCi(pObj)) ) {} else
class abcAigMgr
{
    public:
        abcAigMgr(Abc_Frame_t* pAbc, Abc_Ntk_t* ntk) : pNtk(ntk), pAbc(pAbc) { init(); }
        ~abcAigMgr() {}

        void init();

        Aig_Obj_t**     Aig_NodesDFS(Aig_Obj_t**, int) const;
        void            Aig_AllDFS();
        void            fraig();
        void            randomSim(bool);
        void            simTraversal( Vec_Ptr_t*, size_t*, bool);
        void            simTraversal2( Vec_Ptr_t* vNodes, size_t** simValue, size_t num_words, bool verbose);
        void            rewireByFanins(Aig_Obj_t*, Aig_Obj_t*, Aig_Obj_t*);

        void            printSummary(bool) const;
        void            printPIs() const;
        void            printPOs() const;
        void            printNetlist(Aig_Obj_t*) const;
        Aig_ManCut_t*   Aig_EnumerateCuts( Aig_Man_t*, int , int , int , bool );
        Aig_Obj_t*      getObjbyId(int id) {return Aig_ManObj(pMan, id);}
        Abc_Ntk_t*      getNtk() { return pNtk; }
        Aig_Man_t*      getAig() { return pMan; }
        Aig_Obj_t**     getPIList() { return PI_List;}    
        Aig_Obj_t**     getPOList() { return PO_List;} 

        // for cut sat
        void            simlirarity( char* ); // find the similarity of the cuts in the two circuits
        void            cutMatching();

    private:
        Abc_Frame_t* pAbc;
        Abc_Ntk_t*   pNtk;
        Aig_Man_t*   pMan;

        // for AIG
        Aig_Obj_t** PI_List;
        Aig_Obj_t** PO_List;
        Aig_Obj_t** DFS_List;
        unsigned    Total_num; // number of aigobj
        unsigned    PI_num;
        unsigned    PO_num;
        unsigned    Node_num;

        // for cut sat
        cut_pair_t*              pCutPair;      
        similarity_vec_t*        similarity_table; 
        Aig_Man_t                *pAig1, *pAig2;
        int*                     match_table;          


};
#endif