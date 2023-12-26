/****************************************************************************
  FileName     [ cirCmd.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic cir package commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_CMD_H
#define CIR_CMD_H

#include "gvCmdMgr.h"

GV_COMMAND(CirReadCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirPrintCmd, GV_CMD_TYPE_NETWORK);
GV_COMMAND(CirGateCmd, GV_CMD_TYPE_NETWORK);

#endif // CIR_CMD_H
