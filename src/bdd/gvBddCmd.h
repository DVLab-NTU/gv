#ifndef GV_BDD_CMD_H
#define GV_BDD_CMD_H

#include "gvCmdMgr.h"

GV_COMMAND(GVBddInitCmd,          GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddResetCmd,         GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddAddVarCmd,        GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddSetVarCmd,        GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddInvCmd,           GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddAndCmd,           GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddOrCmd,            GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddNandCmd,          GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddNorCmd,           GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddXorCmd,           GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddXnorCmd,          GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddReportCmd,        GV_CMD_TYPE_BDD);
GV_COMMAND(GVBddDrawCmd,          GV_CMD_TYPE_BDD);

#endif