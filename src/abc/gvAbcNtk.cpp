#ifndef GV_ABC_NTK_C
#define GV_ABC_NTK_C

#include <iostream>
#include <iomanip>
#include "gvAbcNtk.h"
#include "gvAbcMgr.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

AbcMgr* abcNtkMgr::_AbcMgr = 0;

void abcNtkMgr::init()
{
    Total_num = Abc_NtkObjNum(pNtk);
    PI_num = Abc_NtkCiNum(pNtk);
    PO_num = Abc_NtkCoNum(pNtk);
    Node_num = Abc_NtkNodeNum(pNtk);
}

void abcNtkMgr::printSummary() const
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

void abcNtkMgr::sweep( bool verbose) {
    if ( !Abc_NtkIsLogic(pNtk) )
    {
        cout << "The classical (SIS-like) sweep can only be performed on a logic network.\n" << endl;
        return;
    }
    Abc_NtkSweep( pNtk, verbose );
}

#endif