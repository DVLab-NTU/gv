#ifndef GV_VRF_CMD_C
#define GV_VRF_CMD_C

#include "gvVrfCmd.h"
#include "gvAbcMgr.h"
#include "gvMsg.h"
#include "gvUsage.h"
#include "gvModMgr.h"
#include "util.h"
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

bool
GVinitVrfCmd() {
    return (gvCmdMgr->regCmd("Formal Verify", 1, 1, new GVFormalVerifyCmd));
}

//------------------------------------------------------------------------------------------------------------
// Formal Verify [-bmc <int_depth> [-options] | -ind <PO_idx> [-options] | -pdr [-options] | -itp [-options]]
//------------------------------------------------------------------------------------------------------------

GVCmdExecStatus
GVFormalVerifyCmd ::exec(const string& option) {
    gvMsg(GV_MSG_IFO) << "I am GVFormalVerifyCmd " << endl;

    if (gvModMgr->getAigFileName() == "") {
        gvMsg(GV_MSG_IFO) << "[ERROR]: Please use command \"READ DESIGN\" or \"VErilog2 Aig\" to read/make the aig file first !!\n";
        return GV_CMD_EXEC_NOP;
    }

    bool bmc = false, pdr = false, itp = false, ind = false;
    // bmc
    int    bmc_depth, bmc_S, bmc_T, bmc_H, bmc_G, bmc_C, bmc_D, bmc_J, bmc_I, bmc_P, bmc_Q, bmc_R;
    string bmc_option, bmc_L, bmc_W;
    // k-induction
    int    PO_idx;
    int    ind_F, ind_P, ind_C, ind_M, ind_L, ind_N, ind_B;
    string ind_option;
    // pdr
    int    pdr_M, pdr_F, pdr_C, pdr_D, pdr_Q, pdr_T, pdr_H, pdr_G, pdr_S;
    string pdr_option, pdr_L, pdr_I;
    // itp
    int    itp_C, itp_F, itp_T, itp_K;
    string itp_option, itp_L, itp_I;

    char           fname[128];
    vector<string> options;
    GVCmdExec::lexOptions(option, options);

    // get filename, formal method (can keep doing error handling)
    size_t n = options.size();
    for (size_t i = 0; i < n; ++i) {
        const string& token = options[i];
        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           BMC
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!ind) && (!pdr) && (!itp) && !myStrNCmp("-bmc", token, 4)) {
            // if no specify int_depth
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if int_depth not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" time frame for BMC (int_depth) !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                bmc       = true;
                bmc_depth = stoi(options[i + 1]);
            }
        }
        if (bmc && ((!myStrNCmp("-S", token, 2)) || (!myStrNCmp("-T", token, 2)) || (!myStrNCmp("-H", token, 2)) || (!myStrNCmp("-G", token, 2)) ||
                    (!myStrNCmp("-C", token, 2)) || (!myStrNCmp("-D", token, 2)) || (!myStrNCmp("-J", token, 2)) || (!myStrNCmp("-I", token, 2)) ||
                    (!myStrNCmp("-P", token, 2)) || (!myStrNCmp("-Q", token, 2)) || (!myStrNCmp("-R", token, 2)))) {
            // if no specify <num>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if int_depth not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                string bmc_sub_option;
                if (!myStrNCmp("-S", token, 2)) {
                    bmc_S          = stoi(options[i + 1]);
                    bmc_sub_option = " -S " + to_string(bmc_S);
                } else if (!myStrNCmp("-T", token, 2)) {
                    bmc_T          = stoi(options[i + 1]);
                    bmc_sub_option = " -T " + to_string(bmc_T);
                } else if (!myStrNCmp("-H", token, 2)) {
                    bmc_H          = stoi(options[i + 1]);
                    bmc_sub_option = " -H " + to_string(bmc_H);
                } else if (!myStrNCmp("-G", token, 2)) {
                    bmc_G          = stoi(options[i + 1]);
                    bmc_sub_option = " -G " + to_string(bmc_G);
                } else if (!myStrNCmp("-C", token, 2)) {
                    bmc_C          = stoi(options[i + 1]);
                    bmc_sub_option = " -C " + to_string(bmc_C);
                } else if (!myStrNCmp("-D", token, 2)) {
                    bmc_D          = stoi(options[i + 1]);
                    bmc_sub_option = " -D " + to_string(bmc_D);
                } else if (!myStrNCmp("-J", token, 2)) {
                    bmc_J          = stoi(options[i + 1]);
                    bmc_sub_option = " -J " + to_string(bmc_J);
                } else if (!myStrNCmp("-I", token, 2)) {
                    bmc_I          = stoi(options[i + 1]);
                    bmc_sub_option = " -I " + to_string(bmc_I);
                } else if (!myStrNCmp("-P", token, 2)) {
                    bmc_P          = stoi(options[i + 1]);
                    bmc_sub_option = " -P " + to_string(bmc_P);
                } else if (!myStrNCmp("-Q", token, 2)) {
                    bmc_Q          = stoi(options[i + 1]);
                    bmc_sub_option = " -Q " + to_string(bmc_Q);
                } else if (!myStrNCmp("-R", token, 2)) {
                    bmc_R          = stoi(options[i + 1]);
                    bmc_sub_option = " -R " + to_string(bmc_R);
                }
                bmc_option += bmc_sub_option;
            }
        }
        if (bmc && ((!myStrNCmp("-L", token, 2)) || (!myStrNCmp("-W", token, 2)))) {
            // if no specify <filename>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            } else {
                string bmc_sub_option;
                if (!myStrNCmp("-L", token, 2)) {
                    bmc_L          = options[i + 1];
                    bmc_sub_option = " -L " + bmc_L;
                }
                if (!myStrNCmp("-W", token, 2)) {
                    bmc_W          = options[i + 1];
                    bmc_sub_option = " -W " + bmc_W;
                }
                bmc_option += bmc_sub_option;
            }
        }
        if (bmc && ((!myStrNCmp("-a", token, 2)))) {
            bmc_option += " -a";
        }
        if (bmc && ((!myStrNCmp("-x", token, 2)))) {
            bmc_option += " -x";
        }
        if (bmc && ((!myStrNCmp("-d", token, 2)))) {
            bmc_option += " -d";
        }
        if (bmc && ((!myStrNCmp("-u", token, 2)))) {
            bmc_option += " -u";
        }
        if (bmc && ((!myStrNCmp("-r", token, 2)))) {
            bmc_option += " -r";
        }
        if (bmc && ((!myStrNCmp("-s", token, 2)))) {
            bmc_option += " -s";
        }
        if (bmc && ((!myStrNCmp("-g", token, 2)))) {
            bmc_option += " -g";
        }
        if (bmc && ((!myStrNCmp("-v", token, 2)))) {
            bmc_option += " -v";
        }
        if (bmc && ((!myStrNCmp("-z", token, 2)))) {
            bmc_option += " -z";
        }

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                       k-induction
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!itp) && (!pdr) && !myStrNCmp("-ind", token, 4)) {
            // if no specify <PO_idx>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if PO_idx not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                PO_idx = stoi(options[i + 1]);
            }
            ind = true;
        }
        if (ind && ((!myStrNCmp("-F", token, 2)) || (!myStrNCmp("-P", token, 2)) || (!myStrNCmp("-C", token, 2)) || (!myStrNCmp("-M", token, 2)) ||
                    (!myStrNCmp("-L", token, 2)) || (!myStrNCmp("-N", token, 2)) || (!myStrNCmp("-B", token, 2)))) {
            // if no specify <num>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if int_depth not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                string ind_sub_option;
                if (!myStrNCmp("-F", token, 2)) {
                    ind_F          = stoi(options[i + 1]);
                    ind_sub_option = " -F " + to_string(ind_F);
                } else if (!myStrNCmp("-P", token, 2)) {
                    ind_P          = stoi(options[i + 1]);
                    ind_sub_option = " -P " + to_string(ind_P);
                } else if (!myStrNCmp("-C", token, 2)) {
                    ind_C          = stoi(options[i + 1]);
                    ind_sub_option = " -C " + to_string(ind_C);
                } else if (!myStrNCmp("-M", token, 2)) {
                    ind_M          = stoi(options[i + 1]);
                    ind_sub_option = " -M " + to_string(ind_M);
                } else if (!myStrNCmp("-L", token, 2)) {
                    ind_L          = stoi(options[i + 1]);
                    ind_sub_option = " -L " + to_string(ind_L);
                } else if (!myStrNCmp("-N", token, 2)) {
                    ind_N          = stoi(options[i + 1]);
                    ind_sub_option = " -N " + to_string(ind_N);
                } else if (!myStrNCmp("-B", token, 2)) {
                    ind_B          = stoi(options[i + 1]);
                    ind_sub_option = " -B " + to_string(ind_B);
                }
                ind_option += ind_sub_option;
            }
        }

        if (ind && ((!myStrNCmp("-s", token, 2)))) {
            ind_option += " -s";
        }
        if (ind && ((!myStrNCmp("-b", token, 2)))) {
            ind_option += " -b";
        }
        if (ind && ((!myStrNCmp("-r", token, 2)))) {
            ind_option += " -r";
        }
        if (ind && ((!myStrNCmp("-t", token, 2)))) {
            ind_option += " -t";
        }
        if (ind && ((!myStrNCmp("-v", token, 2)))) {
            ind_option += " -v";
        }

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           PDR
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!ind) && (!itp) && !myStrNCmp("-pdr", token, 4)) {
            pdr = true;
        }
        if (pdr && ((!myStrNCmp("-M", token, 2)) || (!myStrNCmp("-F", token, 2)) || (!myStrNCmp("-C", token, 2)) || (!myStrNCmp("-D", token, 2)) ||
                    (!myStrNCmp("-Q", token, 2)) || (!myStrNCmp("-T", token, 2)) || (!myStrNCmp("-H", token, 2)) || (!myStrNCmp("-G", token, 2)) ||
                    (!myStrNCmp("-S", token, 2)))) {
            // if no specify <num>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if int_depth not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                string pdr_sub_option;
                if (!myStrNCmp("-M", token, 2)) {
                    pdr_M          = stoi(options[i + 1]);
                    pdr_sub_option = " -M " + to_string(pdr_M);
                } else if (!myStrNCmp("-F", token, 2)) {
                    pdr_F          = stoi(options[i + 1]);
                    pdr_sub_option = " -F " + to_string(pdr_F);
                } else if (!myStrNCmp("-C", token, 2)) {
                    pdr_C          = stoi(options[i + 1]);
                    pdr_sub_option = " -C " + to_string(pdr_C);
                } else if (!myStrNCmp("-D", token, 2)) {
                    pdr_D          = stoi(options[i + 1]);
                    pdr_sub_option = " -D " + to_string(pdr_D);
                } else if (!myStrNCmp("-Q", token, 2)) {
                    pdr_Q          = stoi(options[i + 1]);
                    pdr_sub_option = " -Q " + to_string(pdr_Q);
                } else if (!myStrNCmp("-T", token, 2)) {
                    pdr_T          = stoi(options[i + 1]);
                    pdr_sub_option = " -T " + to_string(pdr_T);
                } else if (!myStrNCmp("-H", token, 2)) {
                    pdr_H          = stoi(options[i + 1]);
                    pdr_sub_option = " -H " + to_string(pdr_H);
                } else if (!myStrNCmp("-G", token, 2)) {
                    pdr_G          = stoi(options[i + 1]);
                    pdr_sub_option = " -G " + to_string(pdr_G);
                } else if (!myStrNCmp("-S", token, 2)) {
                    pdr_S          = stoi(options[i + 1]);
                    pdr_sub_option = " -S " + to_string(pdr_S);
                }
                pdr_option += pdr_sub_option;
            }
        }
        if (pdr && ((!myStrNCmp("-L", token, 2)) || (!myStrNCmp("-I", token, 2)))) {
            // if no specify <filename>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            } else {
                string pdr_sub_option;
                if (!myStrNCmp("-L", token, 2)) {
                    pdr_L          = options[i + 1];
                    pdr_sub_option = " -L " + pdr_L;
                }
                if (!myStrNCmp("-I", token, 2)) {
                    pdr_I          = options[i + 1];
                    pdr_sub_option = " -I " + pdr_I;
                }
                pdr_option += pdr_sub_option;
            }
        }
        if (pdr && ((!myStrNCmp("-a", token, 2)))) {
            pdr_option += " -a";
        }
        if (pdr && ((!myStrNCmp("-x", token, 2)))) {
            pdr_option += " -x";
        }
        if (pdr && ((!myStrNCmp("-r", token, 2)))) {
            pdr_option += " -r";
        }
        if (pdr && ((!myStrNCmp("-m", token, 2)))) {
            pdr_option += " -m";
        }
        if (pdr && ((!myStrNCmp("-u", token, 2)))) {
            pdr_option += " -u";
        }
        if (pdr && ((!myStrNCmp("-y", token, 2)))) {
            pdr_option += " -y";
        }
        if (pdr && ((!myStrNCmp("-f", token, 2)))) {
            pdr_option += " -f";
        }
        if (pdr && ((!myStrNCmp("-q", token, 2)))) {
            pdr_option += " -q";
        }
        if (pdr && ((!myStrNCmp("-i", token, 2)))) {
            pdr_option += " -i";
        }
        if (pdr && ((!myStrNCmp("-p", token, 2)))) {
            pdr_option += " -p";
        }
        if (pdr && ((!myStrNCmp("-d", token, 2)))) {
            pdr_option += " -d";
        }
        if (pdr && ((!myStrNCmp("-e", token, 2)))) {
            pdr_option += " -e";
        }
        if (pdr && ((!myStrNCmp("-g", token, 2)))) {
            pdr_option += " -g";
        }
        if (pdr && ((!myStrNCmp("-j", token, 2)))) {
            pdr_option += " -j";
        }
        if (pdr && ((!myStrNCmp("-o", token, 2)))) {
            pdr_option += " -o";
        }
        if (pdr && ((!myStrNCmp("-n", token, 2)))) {
            pdr_option += " -n";
        }
        if (pdr && ((!myStrNCmp("-c", token, 2)))) {
            pdr_option += " -c";
        }
        if (pdr && ((!myStrNCmp("-t", token, 2)))) {
            pdr_option += " -t";
        }
        if (pdr && ((!myStrNCmp("-k", token, 2)))) {
            pdr_option += " -k";
        }
        if (pdr && ((!myStrNCmp("-v", token, 2)))) {
            pdr_option += " -v";
        }
        if (pdr && ((!myStrNCmp("-w", token, 2)))) {
            pdr_option += " -w";
        }
        if (pdr && ((!myStrNCmp("-z", token, 2)))) {
            pdr_option += " -z";
        }

        // ------------------------------------------------------------------------------------------------------------------------ //
        //                                                           ITP
        // ------------------------------------------------------------------------------------------------------------------------ //
        if ((!bmc) && (!ind) && (!pdr) && !myStrNCmp("-itp", token, 4)) {
            itp = true;
        }
        if (itp && ((!myStrNCmp("-C", token, 2)) || (!myStrNCmp("-F", token, 2)) || (!myStrNCmp("-T", token, 2)) || (!myStrNCmp("-K", token, 2)))) {
            // if no specify <num>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            }
            // if int_depth not an integer
            else if (!strspn(options[i + 1].c_str(), "0123456789")) {
                cout << "[ERROR]: Please input an \"integer\" for options !" << endl;
                return GVCmdExec::errorOption(GV_CMD_OPT_ILLEGAL, token);
            } else {
                string itp_sub_option;
                if (!myStrNCmp("-C", token, 2)) {
                    itp_C          = stoi(options[i + 1]);
                    itp_sub_option = " -C " + to_string(itp_C);
                } else if (!myStrNCmp("-F", token, 2)) {
                    itp_F          = stoi(options[i + 1]);
                    itp_sub_option = " -F " + to_string(itp_F);
                } else if (!myStrNCmp("-T", token, 2)) {
                    itp_T          = stoi(options[i + 1]);
                    itp_sub_option = " -T " + to_string(itp_T);
                } else if (!myStrNCmp("-K", token, 2)) {
                    itp_K          = stoi(options[i + 1]);
                    itp_sub_option = " -K " + to_string(itp_K);
                }
                itp_option += itp_sub_option;
            }
        }
        if (itp && ((!myStrNCmp("-L", token, 2)) || (!myStrNCmp("-I", token, 2)))) {
            // if no specify <filename>
            if ((i + 1) >= n) {
                return GVCmdExec::errorOption(GV_CMD_OPT_MISSING, token);
            } else {
                string itp_sub_option;
                if (!myStrNCmp("-L", token, 2)) {
                    itp_L          = options[i + 1];
                    itp_sub_option = " -L " + itp_L;
                }
                if (!myStrNCmp("-I", token, 2)) {
                    itp_I          = options[i + 1];
                    itp_sub_option = " -I " + itp_I;
                }
                itp_option += itp_sub_option;
            }
        }
        if (itp && ((!myStrNCmp("-i", token, 2)))) {
            itp_option += " -i";
        }
        if (itp && ((!myStrNCmp("-r", token, 2)))) {
            itp_option += " -r";
        }
        if (itp && ((!myStrNCmp("-t", token, 2)))) {
            itp_option += " -t";
        }
        if (itp && ((!myStrNCmp("-p", token, 2)))) {
            itp_option += " -p";
        }
        if (itp && ((!myStrNCmp("-o", token, 2)))) {
            itp_option += " -o";
        }
        if (itp && ((!myStrNCmp("-m", token, 2)))) {
            itp_option += " -m";
        }
        if (itp && ((!myStrNCmp("-c", token, 2)))) {
            itp_option += " -c";
        }
        if (itp && ((!myStrNCmp("-g", token, 2)))) {
            itp_option += " -g";
        }
        if (itp && ((!myStrNCmp("-b", token, 2)))) {
            itp_option += " -b";
        }
        if (itp && ((!myStrNCmp("-q", token, 2)))) {
            itp_option += " -q";
        }
        if (itp && ((!myStrNCmp("-k", token, 2)))) {
            itp_option += " -k";
        }
        if (itp && ((!myStrNCmp("-d", token, 2)))) {
            itp_option += " -d";
        }
        if (itp && ((!myStrNCmp("-v", token, 2)))) {
            itp_option += " -v";
        }
    }

    // command
    char   Command[1024], inname[128], formal_option[1024];
    string cmd         = "";
    string aigFileName = gvModMgr->getAigFileName();
    strcpy(inname, aigFileName.c_str());
    if (bmc) {
        strcpy(formal_option, bmc_option.c_str());
    }
    if (pdr) {
        strcpy(formal_option, pdr_option.c_str());
    }
    if (itp) {
        strcpy(formal_option, itp_option.c_str());
    }
    if (ind) {
        strcpy(formal_option, ind_option.c_str());
    }

    sprintf(Command, "read %s", inname);
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
    sprintf(Command, "strash");
    Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);

    // if specify multi-formal engine (-bmc 100 -pdr -itp), then execute all
    if (bmc) {
        cout << "\nSuccess: bmc " << endl;
        sprintf(Command, "bmc3 -F %d%s", bmc_depth, formal_option);
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
    } else if (pdr) {
        cout << "\nSuccess: pdr " << endl;
        sprintf(Command, "pdr%s", formal_option);
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
    } else if (itp) {
        cout << "\nSuccess: itp " << endl;
        sprintf(Command, "int%s", formal_option);
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
    } else if (ind) {
        cout << "\nSuccess: k-induction " << endl;
        // extract PO
        sprintf(Command, "cone -O %d -s", PO_idx);
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
        // do k-induction
        sprintf(Command, "indcut %s", formal_option);
        Cmd_CommandExecute(abcMgr->get_Abc_Frame_t(), Command);
    }

    if ((!bmc) && (!pdr) && (!itp) && (!ind)) {
        cout << "\n[ERROR]: Please specify a formal technique ([-bmc <int_depth> | -ind <PO_idx> | -pdr | -itp ])" << endl;
    }
    return GV_CMD_EXEC_DONE;
}

void
GVFormalVerifyCmd ::usage(const bool& verbose) const {
    // ------------------------------------------------------------------------------------------------------------------------ //
    //                                                           BMC
    // ------------------------------------------------------------------------------------------------------------------------ //
    gvMsg(GV_MSG_IFO) << "Usage: Formal Verify -bmc <int_depth> [-STHGCDJIPQR <num>] [-LW <filename>] [-axdursgvz]" << endl;
    gvMsg(GV_MSG_IFO) << "\t         performs bounded model checking with dynamic unrolling" << endl;
    gvMsg(GV_MSG_IFO) << "\t-S <num> : the starting time frame" << endl;
    gvMsg(GV_MSG_IFO) << "\t-T <num> : runtime limit, in seconds" << endl;
    gvMsg(GV_MSG_IFO) << "\t-H <num> : runtime limit per output, in miliseconds (with \"-a\")" << endl;
    gvMsg(GV_MSG_IFO) << "\t-G <num> : runtime gap since the last CEX, in seconds" << endl;
    gvMsg(GV_MSG_IFO) << "\t-C <num> : max conflicts at an output" << endl;
    gvMsg(GV_MSG_IFO) << "\t-D <num> : max conflicts after jumping (0 = infinity)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-J <num> : the number of timeframes to jump (0 = not used)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-I <num> : the number of PIs to abstract" << endl;
    gvMsg(GV_MSG_IFO) << "\t-P <num> : the max number of learned clauses to keep (0=unused)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-Q <num> : delta value for learned clause removal" << endl;
    gvMsg(GV_MSG_IFO) << "\t-R <num> : percentage to keep for learned clause removal" << endl;
    gvMsg(GV_MSG_IFO) << "\t-L <filename> : the log file name" << endl;
    gvMsg(GV_MSG_IFO) << "\t-W <filename> : the log file name with per-output details" << endl;
    gvMsg(GV_MSG_IFO) << "\t-a       : solve all outputs (do not stop when one is SAT)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-x       : toggle storing CEXes when solving all outputs" << endl;
    gvMsg(GV_MSG_IFO) << "\t-d       : toggle dropping (replacing by 0) SAT outputs" << endl;
    gvMsg(GV_MSG_IFO) << "\t-u       : toggle performing structural OR-decomposition" << endl;
    gvMsg(GV_MSG_IFO) << "\t-r       : toggle disabling periodic restarts" << endl;
    gvMsg(GV_MSG_IFO) << "\t-s       : toggle using Satoko by Bruno Schmitt" << endl;
    gvMsg(GV_MSG_IFO) << "\t-g       : toggle using Glucose 3.0 by Gilles Audemard and Laurent Simon" << endl;
    gvMsg(GV_MSG_IFO) << "\t-v       : toggle verbose output" << endl;
    gvMsg(GV_MSG_IFO) << "\t-z       : toggle suppressing report about solved outputs" << endl;

    // ------------------------------------------------------------------------------------------------------------------------ //
    //                                                       k-induction
    // ------------------------------------------------------------------------------------------------------------------------ //
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -ind <PO_idx> [-FPCMLNB num] [-sbrtv]" << endl;
    gvMsg(GV_MSG_IFO) << "\t          : K-step induction strengthened with cut properties to prove the property" << endl;
    gvMsg(GV_MSG_IFO) << "\t<PO_idx>  : the order of specified PO (idx starts from 0)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-F num : number of time frames for induction (1=simple)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-P num : number of time frames in the prefix (0=no prefix)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-C num : the max number of clauses to use for strengthening" << endl;
    gvMsg(GV_MSG_IFO) << "\t-M num : the cut size (2 <= M <= 12)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-L num : the max number of levels for cut computation" << endl;
    gvMsg(GV_MSG_IFO) << "\t-N num : the max number of cuts to compute at a node" << endl;
    gvMsg(GV_MSG_IFO) << "\t-B num : the max number of invariant batches to try" << endl;
    gvMsg(GV_MSG_IFO) << "\t-s     : toggle increment cut size in each batch" << endl;
    gvMsg(GV_MSG_IFO) << "\t-b     : toggle enabling BMC check" << endl;
    gvMsg(GV_MSG_IFO) << "\t-r     : toggle enabling register clauses" << endl;
    gvMsg(GV_MSG_IFO) << "\t-t     : toggle proving target / computing don't-cares" << endl;
    gvMsg(GV_MSG_IFO) << "\t-v     : toggle printing verbose information" << endl;

    // ------------------------------------------------------------------------------------------------------------------------ //
    //                                                           PDR
    // ------------------------------------------------------------------------------------------------------------------------ //
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -pdr [-MFCDQTHGS <num>] [-LI <filename>] [-axrmuyfqipdegjonctkvwz]" << endl;
    gvMsg(GV_MSG_IFO) << "\t         model checking using property directed reachability (aka IC3)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-M <num> : limit on unused vars to trigger SAT solver recycling" << endl;
    gvMsg(GV_MSG_IFO) << "\t-F <num> : limit on timeframes explored to stop computation" << endl;
    gvMsg(GV_MSG_IFO) << "\t-C <num> : limit on conflicts in one SAT call (0 = no limit)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-D <num> : limit on conflicts during ind-generalization (0 = no limit)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-Q <num> : limit on proof obligations before a restart (0 = no limit)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-T <num> : runtime limit, in seconds (0 = no limit)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-H <num> : runtime limit per output, in miliseconds (with \"-a\")" << endl;
    gvMsg(GV_MSG_IFO) << "\t-G <num> : runtime gap since the last CEX (0 = no limit)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-S <num> : * value to seed the SAT solver with" << endl;
    gvMsg(GV_MSG_IFO) << "\t-L <filename> : the log file name" << endl;
    gvMsg(GV_MSG_IFO) << "\t-I <filename> : the invariant file name" << endl;
    gvMsg(GV_MSG_IFO) << "\t-a       : toggle solving all outputs even if one of them is SAT" << endl;
    gvMsg(GV_MSG_IFO) << "\t-x       : toggle storing CEXes when solving all outputs" << endl;
    gvMsg(GV_MSG_IFO) << "\t-r       : toggle using more effort in generalization" << endl;
    gvMsg(GV_MSG_IFO) << "\t-m       : toggle using monolythic CNF computation" << endl;
    gvMsg(GV_MSG_IFO) << "\t-u       : toggle updated X-valued simulation" << endl;
    gvMsg(GV_MSG_IFO) << "\t-y       : toggle using structural flop priorities" << endl;
    gvMsg(GV_MSG_IFO) << "\t-f       : toggle ordering flops by cost before generalization" << endl;
    gvMsg(GV_MSG_IFO) << "\t-q       : toggle creating only shortest counter-examples" << endl;
    gvMsg(GV_MSG_IFO) << "\t-i       : toggle clause pushing from an intermediate timeframe" << endl;
    gvMsg(GV_MSG_IFO) << "\t-p       : toggle reusing proof-obligations in the last timeframe" << endl;
    gvMsg(GV_MSG_IFO) << "\t-d       : toggle dumping invariant (valid if init state is all-0)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-e       : toggle using only support variables in the invariant" << endl;
    gvMsg(GV_MSG_IFO) << "\t-g       : toggle skipping expensive generalization step" << endl;
    gvMsg(GV_MSG_IFO) << "\t-j       : toggle using simplified generalization step" << endl;
    gvMsg(GV_MSG_IFO) << "\t-o       : toggle using property output as inductive hypothesis" << endl;
    gvMsg(GV_MSG_IFO) << "\t-n       : * toggle skipping \'down\' in generalization" << endl;
    gvMsg(GV_MSG_IFO) << "\t-c       : * toggle handling CTGs in \'down\'" << endl;
    gvMsg(GV_MSG_IFO) << "\t-t       : toggle using abstraction" << endl;
    gvMsg(GV_MSG_IFO) << "\t-k       : toggle using simplified refinement" << endl;
    gvMsg(GV_MSG_IFO) << "\t-v       : toggle printing optimization summary" << endl;
    gvMsg(GV_MSG_IFO) << "\t-w       : toggle printing detailed stats" << endl;
    gvMsg(GV_MSG_IFO) << "\t-z       : toggle suppressing report about solved outputs" << endl;

    // ------------------------------------------------------------------------------------------------------------------------ //
    //                                                           ITP
    // ------------------------------------------------------------------------------------------------------------------------ //
    gvMsg(GV_MSG_IFO) << "\nUsage: Formal Verify -itp [-CFTK <num>] [-LI <filename>] [-irtpomcgbqkdv]" << endl;
    gvMsg(GV_MSG_IFO) << "\t         uses interpolation to prove the property" << endl;
    gvMsg(GV_MSG_IFO) << "\t-C <num> : the limit on conflicts for one SAT run" << endl;
    gvMsg(GV_MSG_IFO) << "\t-F <num> : the limit on number of frames to unroll" << endl;
    gvMsg(GV_MSG_IFO) << "\t-T <num> : the limit on runtime per output in seconds" << endl;
    gvMsg(GV_MSG_IFO) << "\t-K <num> : the number of steps in inductive checking" << endl;
    gvMsg(GV_MSG_IFO) << "\t         (K = 1 works in all cases; K > 1 works without -t and -b)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-L <filename> : the log file name" << endl;
    gvMsg(GV_MSG_IFO) << "\t-I <filename> : the file name for dumping interpolant" << endl;
    gvMsg(GV_MSG_IFO) << "\t-i       : toggle dumping interpolant/invariant into a file" << endl;
    gvMsg(GV_MSG_IFO) << "\t-r       : toggle rewriting of the unrolled timeframes" << endl;
    gvMsg(GV_MSG_IFO) << "\t-t       : toggle adding transition into the initial state" << endl;
    gvMsg(GV_MSG_IFO) << "\t-p       : toggle using original Pudlak's interpolation procedure" << endl;
    gvMsg(GV_MSG_IFO) << "\t-o       : toggle using optimized Pudlak's interpolation procedure" << endl;
    gvMsg(GV_MSG_IFO) << "\t-m       : toggle using MiniSat-1.14p (now, Windows-only)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-c       : toggle using inductive containment check" << endl;
    gvMsg(GV_MSG_IFO) << "\t-g       : toggle using bias for global variables using SAT" << endl;
    gvMsg(GV_MSG_IFO) << "\t-b       : toggle using backward interpolation (works with -t)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-q       : toggle using property in two last timeframes" << endl;
    gvMsg(GV_MSG_IFO) << "\t-k       : toggle solving each output separately" << endl;
    gvMsg(GV_MSG_IFO) << "\t-d       : toggle dropping (replacing by 0) SAT outputs (with -k is used)" << endl;
    gvMsg(GV_MSG_IFO) << "\t-v       : toggle verbose output" << endl;
}

void
GVFormalVerifyCmd ::help() const {
    gvMsg(GV_MSG_IFO) << setw(20) << left << "Formal Verify: "
                      << "Use options to execute specific formal engine." << endl;
}



#endif