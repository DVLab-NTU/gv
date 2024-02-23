/****************************************************************************
  FileName     [ cirCmd.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic cir package commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_CMD_H
#define CIR_CMD_H

#include "gvCmdMgr.h"

GV_COMMAND(CirReadCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirPrintCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirGateCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirWriteCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirSimCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirSweepCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirOptCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirStrashCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirFraigCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirMiterCmd, GV_CMD_TYPE_NETWORK);
// GV_COMMAND(CirEffortCmd, GV_CMD_TYPE_NETWORK);

#endif  // CIR_CMD_H
