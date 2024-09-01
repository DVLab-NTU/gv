#include "abcMgr.h"
#include "proof/pdr/pdr.h"

void AbcMgr::runPDR(const bool &verbose) {
    // Start PDR
    Pdr_Par_t *pPars = new Pdr_Par_t();
    Pdr_ManSetDefaultParams(pPars);
    if (verbose) pPars->fVerbose = 1;
    pPars->fSolveAll = 1;
    pAbc->Status = Abc_NtkDarPdr(pNtk, pPars);
}