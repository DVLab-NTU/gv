/****************************************************************************
  FileName     [ satCmd.h ]
  PackageName  [ sat ]
  Synopsis     [ Define basic sat prove package commands ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "gvCmdMgr.h"

// ============================================================================
// Classes for Prove package commands
// ============================================================================
GV_COMMAND(SATVerifyItpCmd, GV_CMD_TYPE_ITP);
GV_COMMAND(SATVerifyBmcCmd, GV_CMD_TYPE_ITP);
