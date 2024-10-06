#ifndef GV_SIM_CMD_C
#define GV_SIM_CMD_C

#include "satCmd.h"

#include <iomanip>
#include <fstream>

#include "gvMsg.h"

#include "minisatMgr.h"
#include "satMgr.h"
#include "util.h"

#include "iostream"

bool initSatCmd() {
    return (gvCmdMgr->regCmd("SATSolve DIMACS", 4, 6, new SatSolveDimacCmd));
}

using namespace gv::sat;

//----------------------------------------------------------------------
// EXPeriment
//----------------------------------------------------------------------
GVCmdExecStatus SatSolveDimacCmd::exec(const string& option) {
    //! Place your experimental functions and commands here
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    if (options.size() < 2) return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, "");
    if (options.size() > 2) return GVCmdExec::errorOption(GV_CMD_OPT_EXTRA, options[2]);

    if (myStrNCmp("-File", options[0], 2) != 0)
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[0]);

    string filename = options[1];
    ifstream file(filename);
    if (!file.is_open()) {
        gvMsg(GV_MSG_ERR) << "File " << filename << " does NOT Exist !!" << endl;
        return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, options[1]);
    }

    SatSolverMgr *gvSatSolver = new gv::sat::MinisatMgr();
    gvSatSolver->solve_dimacs_cnf(filename);

    return GV_CMD_EXEC_DONE;
}

void SatSolveDimacCmd::usage(const bool& verbose) const {
    gvMsg(GV_MSG_IFO) << "Usage: SATSolve DIMACS <-File <string(dimacsFormatFileName)> >" << endl;
}

void SatSolveDimacCmd::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << std::left << "SATSolve DIMACS: "
                      << "Command for satsolving DIMACS format file." << endl;
}

#endif
