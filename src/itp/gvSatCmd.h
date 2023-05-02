/****************************************************************************
  FileName     [ satCmd.h ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_SAT_CMD_H_
#define GV_SAT_CMD_H_

#include "gvCmdMgr.h"

// ============================================================================
// Classes for Prove package commands
// ============================================================================
GV_COMMAND(SATVerifyItpCmd, GV_CMD_TYPE_ITP);
GV_COMMAND(SATVerifyBmcCmd, GV_CMD_TYPE_ITP);

#endif
