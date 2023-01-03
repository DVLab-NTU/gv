#include "gvCnf.h"
#include "gvAigMgr.h"
#include <iostream>

using namespace std;

void Aig2Cnf(Aig_Man_t * pAig1, Aig_Man_t * pAig2,  cnf_obj_t& cnf_obj, bool old)
{
    int i;
    Aig_Obj_t *pObj;
    vector<lit> clause;
    cnf_obj.cnf.clear();
    // constraint 5
    Aig_ManForEachNode((old ? pAig1 : pAig2), pObj, i)
    {
        clause.clear();
        clause.push_back((Aig_IsComplement(Aig_ObjFanout0((old ? pAig1 : pAig2), pObj)->pFanin0) ? Abc_Var2Lit(pObj->Id, 1) : Abc_Var2Lit(pObj->Id, 0)) + 2 * (cnf_obj.nVars));
        clause.push_back((Aig_ObjFaninC0(pObj) ? Abc_Var2Lit(Aig_ObjFanin0(pObj)->Id, 0) : Abc_Var2Lit(Aig_ObjFanin0(pObj)->Id, 1)) + 2 * (cnf_obj.nVars));
        clause.push_back((Aig_ObjFaninC1(pObj) ? Abc_Var2Lit(Aig_ObjFanin1(pObj)->Id, 0) : Abc_Var2Lit(Aig_ObjFanin1(pObj)->Id, 1)) + 2 * (cnf_obj.nVars));
        add_TFO_constraint(clause, pAig1, pAig2, pObj, old, true);
        cnf_obj.cnf.push_back(clause);

        clause.clear();
        clause.push_back((Aig_IsComplement(Aig_ObjFanout0((old ? pAig1 : pAig2), pObj)->pFanin0) ? Abc_Var2Lit(pObj->Id, 0) : Abc_Var2Lit(pObj->Id, 1)) + 2 * (cnf_obj.nVars));
        clause.push_back((Aig_ObjFaninC0(pObj) ? Abc_Var2Lit(Aig_ObjFanin0(pObj)->Id, 1) : Abc_Var2Lit(Aig_ObjFanin0(pObj)->Id, 0)) + 2 * (cnf_obj.nVars));
        add_TFO_constraint(clause, pAig1, pAig2, pObj, old, true);
        cnf_obj.cnf.push_back(clause);

        clause.clear();
        clause.push_back((Aig_IsComplement(Aig_ObjFanout0((old ? pAig1 : pAig2), pObj)->pFanin0) ? Abc_Var2Lit(pObj->Id, 0) : Abc_Var2Lit(pObj->Id, 1)) + 2 * (cnf_obj.nVars));
        clause.push_back((Aig_ObjFaninC1(pObj) ? Abc_Var2Lit(Aig_ObjFanin1(pObj)->Id, 1) : Abc_Var2Lit(Aig_ObjFanin1(pObj)->Id, 0)) + 2 * (cnf_obj.nVars));
        add_TFO_constraint(clause, pAig1, pAig2, pObj, old, true);
        cnf_obj.cnf.push_back(clause);
    }

    // constraint 6
    Aig_ManForEachCi((old ? pAig1 : pAig2), pObj, i)
    {
        // cout << "PI id = " <<  pObj -> Id << endl;
        clause.clear(); // make sure the clause is empty
        constraint6(clause, pAig1, pAig2, pObj, cnf_obj, old);
        cnf_obj.cnf.push_back(clause);
    }
}

void WriteCnf(sat_solver * pSat, cnf_obj_t& cnf_obj)
{
    for(int i = 0; i < cnf_obj.cnf.size(); ++i)
    {
        sat_solver_addclause(pSat, &cnf_obj.cnf[i].front(), &cnf_obj.cnf[i].back() + 1);
    }
    // cout << cnf_obj.cnf.size() << " clauses written" << endl;
}

void 
cnf_obj_t::cnf_data_lift(int nNewVar)
{
    nVars += nNewVar;
}


// this function add the TFO constraint of eij
void add_TFO_constraint(vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, bool old, bool is_self)
{
    if(pObj->Type == AIG_OBJ_CO) return;
    int iFanout, i;
    Aig_Obj_t * pFanout;
    
    if(!is_self)
    {
        if(old) // add the entire row into of eij into the clause
        {
            for(i = 0; i < Aig_ManCandNum(pAig2); ++i)
            {
                // cout << "ewofeiofjioejfoiejsiof "  << pObj->Id << " " << Aig_ManCiNum(pAig1) << endl;
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) * Aig_ManCandNum(pAig2) + i, 0));
            }
        }
        else
        {
            for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
            {
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) + Aig_ManCandNum(pAig2) * i, 0));
            }
        }
    }
    // else cout << pObj ->Id << endl;
    
    if(old)
    {
        Aig_ObjForEachFanout(pAig1, pObj, pFanout, iFanout, i)
        {
            // cout << "fanout id = " << pFanout->Id << endl;
            add_TFO_constraint(clause, pAig1, pAig2, pFanout, old, false);
        }
        // }
    }
    else
    {
        Aig_ObjForEachFanout(pAig2, pObj, pFanout, iFanout, i)
        {
            add_TFO_constraint(clause, pAig1, pAig2, pFanout, old, false);
        }
    }
}

void constraint4(sat_solver * pSat, Aig_Man_t * pAig1, Aig_Man_t * pAig2)
{
    Aig_Obj_t *pObj1, *pObj2;
    int i, j;
    // cnf_t cnf;
    vector<lit> clause;
    int previous_var_num_1  = (Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2));
    int previous_var_num_2  = (Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2) + Aig_ManCandNum(pAig1));
     Aig_ManForEachCand(pAig1, pObj1, i)
    {
         Aig_ManForEachCand(pAig2, pObj2, j)
        {
            clause.clear();
            clause.push_back(Abc_Var2Lit(previous_var_num_1 + i, 1)); // ~a
            clause.push_back(Abc_Var2Lit(previous_var_num_2 + j, 0)); // b
            clause.push_back(Abc_Var2Lit((j - 1) + (i - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
            sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);

            clause.clear();
            clause.push_back(Abc_Var2Lit(previous_var_num_1 + i, 0)); // a
            clause.push_back(Abc_Var2Lit(previous_var_num_2 + j, 1)); // ~b
            clause.push_back(Abc_Var2Lit((j - 1) + (i - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
            sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);
        }
    }

    // no need to match pi
    // Aig_ManForEachCi(pAig1, pObj1, i)
    // {
    //     clause.clear();
    //     clause.push_back(Abc_Var2Lit(previous_var_num_1 + i + 1, 1)); // ~a
    //     clause.push_back(Abc_Var2Lit(previous_var_num_2 + i, 0)); // b
    //     // clause.push_back(Abc_Var2Lit((j - Aig_ManCiNum(pAig2) - 1) + (i - Aig_ManCiNum(pAig1) - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
    //     sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);

    //     clause.clear();
    //     clause.push_back(Abc_Var2Lit(previous_var_num_1 + i + 1, 0)); // a
    //     clause.push_back(Abc_Var2Lit(previous_var_num_2 + i, 1)); // ~b
    //     // clause.push_back(Abc_Var2Lit((j - Aig_ManCiNum(pAig2) - 1) + (i - Aig_ManCiNum(pAig1) - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
    //     sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);
    // }

    // match po
    for(i = 0; i < Aig_ManCoNum(pAig1); ++i)
    {
        // cout << "hihihihihiihihihi " << Aig_ObjFaninC0(pObj1) << " " << Aig_IsComplement(pObj1->pFanin0) << " id " << Aig_ObjFanin0(pObj1)->Id << " " << previous_var_num_1 << " " << previous_var_num_2 << endl;
        clause.clear();
        clause.push_back(Abc_Var2Lit(previous_var_num_1 + Aig_ObjFanin0(Aig_ManCo(pAig1, i))->Id, 0)); // a
        clause.push_back(Abc_Var2Lit(previous_var_num_2 + Aig_ObjFanin0(Aig_ManCo(pAig2, i))->Id , 0)); // b
        // clause.push_back(Abc_Var2Lit((j - Aig_ManCiNum(pAig2) - 1) + (i - Aig_ManCiNum(pAig1) - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
        sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);

        clause.clear();
        clause.push_back(Abc_Var2Lit(previous_var_num_1 + Aig_ObjFanin0(Aig_ManCo(pAig1, i))->Id, 1)); // ~a
        clause.push_back(Abc_Var2Lit(previous_var_num_2 + Aig_ObjFanin0(Aig_ManCo(pAig2, i))->Id, 1)); // ~b
        // clause.push_back(Abc_Var2Lit((j - Aig_ManCiNum(pAig2) - 1) + (i - Aig_ManCiNum(pAig1) - 1) * Aig_ManCandNum(pAig2), 1)); // ~eij
        sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);
    }
}

void constraint6(vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, cnf_obj_t& cnf_obj, bool old)
{
    if(pObj->Type == AIG_OBJ_CO) return;
    int iFanout = -1, i;
    Aig_Obj_t * pFanout;
    
    // if(!is_self)
    // {
        if(old) // add the entire row into of eij into the clause
        {
            for(i = 0; i < Aig_ManCandNum(pAig2); ++i)
            {
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) * Aig_ManCandNum(pAig2) + i, 0));
            }
        }
        else
        {
            for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
            {
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) + Aig_ManCandNum(pAig2) * i, 0));
                // clause.push_back(Abc_Var2Lit((pObj->Id - Aig_ManCiNum(pAig2) - 1) + Aig_ManCandNum(pAig2) * i, 0));
            }
        }
    // }
    // else cout << pObj ->Id << endl;
    
    if(old)
    {
        Aig_ObjForEachFanout(pAig1, pObj, pFanout, iFanout, i)
        {
            // cout << "fanout id = " << pFanout->Id << endl;
            constraint6(clause, pAig1, pAig2, pFanout, cnf_obj, old);
        }
        // }
    }
    else
    {
        Aig_ObjForEachFanout(pAig2, pObj, pFanout, iFanout, i)
        {
            constraint6(clause, pAig1, pAig2, pFanout, cnf_obj, old);
        }
    }
}

void constraint7(sat_solver * pSat, Aig_Man_t * pAig1, Aig_Man_t * pAig2)
{
    int i, j, eij;
    vector<lit> clause;
    cout << "Adding constraint7" << endl;
    for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
    {
        for(j = 0; j < Aig_ManCandNum(pAig1); ++j)
        {
            clause.clear();
            clause.push_back(Abc_Var2Lit(i * Aig_ManCandNum(pAig1) + j, 1));
            // cout << Aig_ManObj(pAig1, i + 1)->Id << " " << Aig_ManObj(pAig2, j + 1)->Id << "fkeiwfjoifew" << endl;
            // break;
            add_TFO_eij(pSat, clause, pAig1, pAig2, Aig_ManObj(pAig1, i + 1), true, true);
            add_TFO_eij(pSat, clause, pAig1, pAig2, Aig_ManObj(pAig2, j + 1), false, true);
            // sat_solver_addclause(pSat, &clause.front(), &clause.back());
        }
    }
}

void add_TFO_eij(sat_solver * pSat, vector<lit>& clause, Aig_Man_t * pAig1, Aig_Man_t * pAig2, Aig_Obj_t * pObj, bool old, bool is_self)
{
    if(pObj->Type == AIG_OBJ_CO) return;
    int i, j;
    int iFanout = -1;
    Aig_Obj_t * pFanout;
    int previous_var_num_1  = (Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2));
    int previous_var_num_2  = (Aig_ManCandNum(pAig1) * Aig_ManCandNum(pAig2) + Aig_ManCandNum(pAig1));
    if(!is_self)
    {
        // clause.push_back(Abc_Var2Lit(, 1));
        if(old)
        {
             for(i = 0; i < Aig_ManCandNum(pAig2); ++i)
            {
                // cout << clause.size() << endl;
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) * Aig_ManCandNum(pAig2) + i, 1));
                // cout << clause.size() << endl;
                sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);
                clause.pop_back();
            }
        }
        else
        {
            for(i = 0; i < Aig_ManCandNum(pAig1); ++i)
            {
                clause.push_back(Abc_Var2Lit((pObj->Id - 1) + Aig_ManCandNum(pAig2) * i, 1));
                sat_solver_addclause(pSat, &clause.front(), &clause.back() + 1);
                clause.pop_back();
            }
        }
    }

    if(old)
    {
        Aig_ObjForEachFanout(pAig1, pObj, pFanout, iFanout, i)
        {
            // cout << "fanout id = " << pFanout->Id << " " << pObj->Id << endl;
            add_TFO_eij(pSat, clause, pAig1, pAig2, pFanout, true, false);
        }
    }
    else
    {
        Aig_ObjForEachFanout(pAig2, pObj, pFanout, iFanout, i)
        {
            add_TFO_eij(pSat, clause, pAig1, pAig2, pFanout, false, false);
        }
    }
}