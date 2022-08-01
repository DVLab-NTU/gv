#include "gvAbcMgr.h"
#include "gvAbcNtk.h"
#include <iostream>
#include <cstring>
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
    char pFileName[128];
    strcpy(pFileName, fileName.c_str());
    char Command[1000];
    sprintf( Command, "read %s", pFileName );
    Cmd_CommandExecute( pAbc, Command );
}

void
AbcMgr::abcPrintDesign()
{
    Cmd_CommandExecute( pAbc, "lsv_print_nodes" );
}

