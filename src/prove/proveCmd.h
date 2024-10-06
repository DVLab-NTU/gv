/****************************************************************************
  FileName     [ proveCmd.h ]
  PackageName  [ prove ]
  Synopsis     [ Define basic prove package commands ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "gvCmdMgr.h"

// ============================================================================
// Classes for Prove package commands
// ============================================================================
GV_COMMAND(PInitialStateCmd, GV_CMD_TYPE_PROVE);
GV_COMMAND(PTransRelationCmd, GV_CMD_TYPE_PROVE);
GV_COMMAND(PImageCmd, GV_CMD_TYPE_PROVE);
GV_COMMAND(PCheckPropertyCmd, GV_CMD_TYPE_PROVE);
