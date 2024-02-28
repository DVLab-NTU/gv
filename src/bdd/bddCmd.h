/****************************************************************************
  FileName     [ bddCmd.h ]
  PackageName  [ ]
  Synopsis     [ Define classes for BDD commands ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_CMD_H
#define BDD_CMD_H

#include "gvCmdMgr.h"

// ============================================================================
// Classes for BDD package commands
// ============================================================================
GV_COMMAND(BResetCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BSetVarCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BInvCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BAndCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BOrCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BNandCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BNorCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BXorCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BXnorCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BCofactorCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BExistCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BCompareCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BSimulateCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BReportCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BDrawCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BSetOrderCmd, GV_CMD_TYPE_BDD);
GV_COMMAND(BConstructCmd, GV_CMD_TYPE_BDD);

#endif  // BDD_CMD_H
