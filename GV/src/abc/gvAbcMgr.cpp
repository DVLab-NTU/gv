#include "gvAbcMgr.h"
#include "gvAbcNtk.h"
#include <iostream>
#include <string>

AbcMgr* abcMgr;

void
AbcMgr::init()
{
    Abc_Start();
	pAbc = Abc_FrameGetGlobalFrame();
}

void
AbcMgr::reset()
{
    delete pAbc;
}

void
AbcMgr::abcReadDesign(string& fileName)
{
    string pFileName = fileName;
    char Command[1000];
    cout << pFileName << endl;
    sprintf( Command, "read %s", pFileName );
    Cmd_CommandExecute( pAbc, "read example.v" );
}

void
AbcMgr::abcPrintDesign()
{
    Cmd_CommandExecute( pAbc, "lsv_print_nodes" );
}

