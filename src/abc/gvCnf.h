#ifndef GV_CNF
#define GV_CNF

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "gvAbcNtk.h"
#include <vector>

typedef vector<vector<lit>> cnf_t;
// typedef vector<lit> clause_t;
typedef struct cnf_obj_t_ cnf_obj_t;
// static inline int     var_value     (sat_solver* s, int v)            { return s->assigns[v];  }

struct cnf_obj_t_
{
    cnf_t cnf;
    int   nVars;

    void cnf_data_lift(int nNewVar);
};


void Aig2Cnf(Aig_Man_t * pAig1, Aig_Man_t * pAig2, cnf_obj_t& cnf, bool old);
void WriteCnf(sat_solver * pSat, cnf_obj_t& cnf);
void add_TFO_constraint(vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, bool old, bool is_self);
void constraint4(sat_solver * pSat, Aig_Man_t * pAig1, Aig_Man_t * pAig2);
void constraint6(vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, cnf_obj_t& cnf_obj, bool old);
void constraint7(sat_solver * pSat, Aig_Man_t * pAig1, Aig_Man_t * pAig2);
void add_TFO_eij(sat_solver * pSat, vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, bool old, bool is_self);
int  aigIds2var(int IdOld, int IdGol, Aig_Man_t * pAig1, Aig_Man_t * pAig2);
void cutCheck(sat_solver * pSat, Aig_Man_t * pAig1, Aig_Man_t * pAig2, vector<int>& assump);
void satAnalysis(sat_solver * pSat, sat_solver * pSatCut, Aig_Man_t * pAig1, Aig_Man_t * pAig2, vector<int>& assump);
#endif