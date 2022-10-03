#ifndef GV_ABC_CMD_H
#define GV_ABC_CMD_H

#include "gvCmdMgr.h"

GV_COMMAND(GVABCReadCmd,         GV_CMD_TYPE_ABC);
GV_COMMAND(GVABCPrintCmd,        GV_CMD_TYPE_ABC);
// support for all abc's original commands 
GV_COMMAND(GVABCOriginalCmd,     GV_CMD_TYPE_ABC);

#endif