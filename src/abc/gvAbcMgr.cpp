#include "gvAbcMgr.h"
#include "gvAbcNtk.h"
#include "gvBddMgr.h"
#include "base/abc/abc.h"
#include "bdd/cudd/cudd.h"
#include "sat/cnf/cnf.h"
#include <cstring>
#include <iostream>
#include <string>

AbcMgr* abcMgr;

void
AbcMgr::init() {
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
}

void
AbcMgr::reset() {
    delete pAbc;
}

void
AbcMgr::abcReadDesign(string& fileName) {
    char pFileName[128];
    strcpy(pFileName, fileName.c_str());
    char Command[1000];
    sprintf(Command, "read %s", pFileName);
    Cmd_CommandExecute(pAbc, Command);
    pNtkMgr = new abcNtkMgr(pAbc->pNtkCur);
}

void
AbcMgr::abcNtk2Aig() {
    Abc_Ntk_t* pAig = Abc_NtkStrash(pAbc->pNtkCur, 0, 0, 0);
    if (pAig == NULL) {
        cout << "Strashing has failed." << endl;
        return;
    } else {
        pAigMgr = new abcAigMgr(pAbc, pAig);
        cout << "Successfully transform netlist into AIG!" << endl;
    }
}

void
AbcMgr::abcNtk2Bdd() {
    int i;
    int pBdd = Abc_NtkToBdd(pAbc->pNtkCur);
    DdManager* DdMgr = (DdManager*) pAbc->pNtkCur->pManFunc; 
    bddMgr->init(DdMgr, pAbc->pNtkCur);
    bddMgr->construct();
    if (pBdd == 0) {
        cout << "Transforming has failed." << endl;
        return;
    } else {
        cout << "Successfully transform netlist into BDD!" << endl;
    }
}