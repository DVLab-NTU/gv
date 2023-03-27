/****************************************************************************
  FileName     [ bddCmd.h ]
  PackageName  [ ]
  Synopsis     [ Define classes for BDD commands ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_CMD_H
#define BDD_CMD_H

#include "gvCmdMgr.h"

// ============================================================================
// Classes for BDD package commands 
// ============================================================================

GV_COMMAND(BResetCmd,    GV_CMD_TYPE_BDD);
/*V3_COMMAND(BSetVarCmd,   CMD_TYPE_BDD);
V3_COMMAND(BInvCmd,      CMD_TYPE_BDD);
V3_COMMAND(BAndCmd,      CMD_TYPE_BDD);
V3_COMMAND(BOrCmd,       CMD_TYPE_BDD);
V3_COMMAND(BNandCmd,     CMD_TYPE_BDD);
V3_COMMAND(BNorCmd,      CMD_TYPE_BDD);
V3_COMMAND(BXorCmd,      CMD_TYPE_BDD);
V3_COMMAND(BXnorCmd,     CMD_TYPE_BDD);
V3_COMMAND(BCofactorCmd, CMD_TYPE_BDD);
V3_COMMAND(BExistCmd,    CMD_TYPE_BDD);
V3_COMMAND(BCompareCmd,  CMD_TYPE_BDD);
V3_COMMAND(BSimulateCmd, CMD_TYPE_BDD);
V3_COMMAND(BReportCmd,   CMD_TYPE_BDD);
V3_COMMAND(BDrawCmd,     CMD_TYPE_BDD);
V3_COMMAND(BSetOrderCmd, CMD_TYPE_BDD);
V3_COMMAND(BConstructCmd,CMD_TYPE_BDD);
*/

#endif // BDD_CMD_H
