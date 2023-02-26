#ifndef GV_ABC_NTK_C
#define GV_ABC_NTK_C

#include "gvAbcNtk.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "gvAbcMgr.h"
#include <iomanip>
#include <iostream>

extern "C"
{
    int Abc_NtkDSat( Abc_Ntk_t * pNtk, ABC_INT64_T nConfLimit, ABC_INT64_T nInsLimit, int nLearnedStart, int nLearnedDelta, int nLearnedPerce, int fAlignPol, int fAndOuts, int fNewSolver, int fVerbose );
}

AbcMgr* abcNtkMgr::_AbcMgr = 0;

void
abcNtkMgr::init() {
    Total_num = Abc_NtkObjNum(pNtk);
    PI_num    = Abc_NtkCiNum(pNtk);
    PO_num    = Abc_NtkCoNum(pNtk);
    Node_num  = Abc_NtkNodeNum(pNtk);
}

void
abcNtkMgr::printSummary() const {
    cout << endl;
    cout << "Circuit Statistics" << endl;
    cout << "==================" << endl;
    cout << "  PI  " << setw(10) << right << PI_num << endl;
    cout << "  PO  " << setw(10) << right << PO_num << endl;
    cout << "  Node " << setw(10) << right << Node_num << endl;
    cout << "------------------" << endl;
    cout << "  Total" << setw(10) << right << PI_num + PO_num + Node_num << endl;
}

void
abcNtkMgr::sweep(bool verbose) {
    if (!Abc_NtkIsLogic(pNtk)) {
        cout << "The classical (SIS-like) sweep can only be performed on a logic network.\n" << endl;
        return;
    }
    Abc_NtkSweep(pNtk, verbose);
}

void Abc_NtkModelToVector( Abc_Ntk_t * pNtk, Vec_Int_t * vPiValues )
{
    int * pModel;
    pModel = pNtk->pModel;
    for (int  i = 0; i < Abc_NtkPiNum(pNtk); i++ )
        Vec_IntWriteEntry( vPiValues, i, pModel[i] );
}

void Abc_NtkVectorClearPars( Vec_Int_t * vPiValues, int nPars )
{
    for (int i = 0; i < nPars; i++ )
        Vec_IntWriteEntry( vPiValues, i, -1 );
}

void Abc_NtkVectorClearVars( Abc_Ntk_t * pNtk, Vec_Int_t * vPiValues, int nPars )
{
    for (int i = nPars; i < Abc_NtkPiNum(pNtk); i++ )
        Vec_IntWriteEntry( vPiValues, i, -1 );
}

void Abc_NtkVectorPrintPars( Vec_Int_t * vPiValues, int nPars )
{
    for (int i = 0; i < nPars; i++ )
        printf( "%d", Vec_IntEntry(vPiValues,i) );
}

void Abc_NtkVectorPrintVars( Abc_Ntk_t * pNtk, Vec_Int_t * vPiValues, int nPars )
{
    for (int i = nPars; i < Abc_NtkPiNum(pNtk); i++ )
        printf( "%d", Vec_IntEntry(vPiValues,i) );
} 

void
abcNtkMgr::abcQBF(int nPars, int nIters, bool fVerbose)
{
    if ( !Abc_NtkIsStrash(pNtk) ) {
        pNtk = Abc_NtkStrash( pNtk, 0, 1, 0 );
    }

    Abc_Ntk_t * pNtkVer, * pNtkSyn, * pNtkSyn2, * pNtkTemp;
    Vec_Int_t * vPiValues;
    abctime clkTotal = Abc_Clock(), clkS, clkV;
    int nInputs, RetValue, fFound = 0, nItersMax=10000;

    assert( Abc_NtkIsStrash(pNtk) );
    assert( Abc_NtkIsComb(pNtk) );
    assert( Abc_NtkPoNum(pNtk) == 1 );
    assert( nPars > 0 && nPars < Abc_NtkPiNum(pNtk) );
    //  assert( Abc_NtkPiNum(pNtk)-nPars < 32 );
    nInputs = Abc_NtkPiNum(pNtk) - nPars;

    // initialize the synthesized network with 0000-combination
    vPiValues = Vec_IntStart( Abc_NtkPiNum(pNtk) );

    // create random init value
    {
        int i;
        srand( time(NULL) );
        for ( i = nPars; i < Abc_NtkPiNum(pNtk); i++ )
            Vec_IntWriteEntry( vPiValues, i, rand() & 1 );
    }

    Abc_NtkVectorClearPars( vPiValues, nPars );
    pNtkSyn = Abc_NtkMiterCofactor( pNtk, vPiValues );
    if ( fVerbose )
    {
        printf( "Iter %2d : ", 0 );
        printf( "AIG = %6d  ", Abc_NtkNodeNum(pNtkSyn) );
        Abc_NtkVectorPrintVars( pNtk, vPiValues, nPars );
        printf( "\n" );
    }

    // iteratively solve
    for ( nIters = 0; nIters < nItersMax; nIters++ )
    {
        // solve the synthesis instance
        clkS = Abc_Clock();
        // RetValue = Abc_NtkMiterSat( pNtkSyn, 0, 0, 0, NULL, NULL );
        RetValue = Abc_NtkDSat( pNtkSyn, (ABC_INT64_T)0, (ABC_INT64_T)0, 0, 0, 0, 1, 0, 0, 0 );
        clkS = Abc_Clock() - clkS;
        if ( RetValue == 0 )
            Abc_NtkModelToVector( pNtkSyn, vPiValues );
        if ( RetValue == 1 )
        {
            break;
        }
        if ( RetValue == -1 )
        {
            printf( "Synthesis timed out.\n" );
            break;
        }
        // there is a counter-example

        // construct the verification instance
        Abc_NtkVectorClearVars( pNtk, vPiValues, nPars );
        pNtkVer = Abc_NtkMiterCofactor( pNtk, vPiValues );
        // complement the output
        Abc_ObjXorFaninC( Abc_NtkPo(pNtkVer,0), 0 );

        // solve the verification instance
        clkV = Abc_Clock();
        RetValue = Abc_NtkMiterSat( pNtkVer, 0, 0, 0, NULL, NULL );
        clkV = Abc_Clock() - clkV;
        if ( RetValue == 0 )
            Abc_NtkModelToVector( pNtkVer, vPiValues );
        Abc_NtkDelete( pNtkVer );
        if ( RetValue == 1 )
        {
            fFound = 1;
            break;
        }
        if ( RetValue == -1 )
        {
            printf( "Verification timed out.\n" );
            break;
        }
        // there is a counter-example

        // create a new synthesis network
        Abc_NtkVectorClearPars( vPiValues, nPars );
        pNtkSyn2 = Abc_NtkMiterCofactor( pNtk, vPiValues );
        // add to the synthesis instance
        pNtkSyn = Abc_NtkMiterAnd( pNtkTemp = pNtkSyn, pNtkSyn2, 0, 0 );
        Abc_NtkDelete( pNtkSyn2 );
        Abc_NtkDelete( pNtkTemp );

        if ( fVerbose )
        {
            printf( "Iter %2d : ", nIters+1 );
            printf( "AIG = %6d  ", Abc_NtkNodeNum(pNtkSyn) );
            Abc_NtkVectorPrintVars( pNtk, vPiValues, nPars );
            printf( "  " );
            ABC_PRT( "Syn", clkS );
        }
    }
    Abc_NtkDelete( pNtkSyn );
    // report the results
    if ( fFound )
    {
        int nZeros = Vec_IntCountZero( vPiValues );
        printf( "Parameters: " );
        Abc_NtkVectorPrintPars( vPiValues, nPars );
        printf( "  Statistics: 0=%d 1=%d\n", nZeros, Vec_IntSize(vPiValues) - nZeros );
        printf( "Solved after %d iterations.  ", nIters );
    }
    else if ( nIters == nItersMax )
        printf( "Quit after %d iterations.  ", nItersMax );
    else
        printf( "Implementation does not exist.  " );
    ABC_PRT( "Total runtime", Abc_Clock() - clkTotal );
    Vec_IntFree( vPiValues );
}

#endif