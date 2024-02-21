#ifndef GV_CMD_MGR_C
#define GV_CMD_MGR_C

#include "gvCmdMgr.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "gvMsg.h"
#include "util.h"

extern "C"
{
#include <readline/history.h>
#include <readline/readline.h>
}

/* -------------------------------------------------- *\
 * Class GVCmdMgr Implementations
\* -------------------------------------------------- */

bool GVCmdExec::operator<(const GVCmdExec& rhs) const {
    string lstr, rstr;
    for (size_t idx = 0; idx < _cmdLen; ++idx) {
        if (idx != 0) lstr.append(" ");
        lstr.append(_mandCmd[idx]);
        lstr.append(_optCmd[idx]);
    }
    for (size_t idx = 0; idx < rhs._cmdLen; ++idx) {
        if (idx != 0) rstr.append(" ");
        rstr.append(rhs._mandCmd[idx]);
        rstr.append(rhs._optCmd[idx]);
    }
    return (lstr < rstr);
}

void GVCmdExec::lexOptions(const string& option, vector<string>& tokens) const {
    string token;
    size_t n = myStrGetTok(option, token);
    while (token.size()) {
        tokens.push_back(token);
        n = myStrGetTok(option, token, n);
    }
}

bool GVCmdExec::lexSingleOption(const string& option, string& token, bool optional) const {
    size_t n = myStrGetTok(option, token);
    if (!optional) {
        if (token.size() == 0) {
            errorOption(GV_CMD_OPT_MISSING, "");
            return false;
        }
    }
    if (n != string::npos) {
        errorOption(GV_CMD_OPT_EXTRA, option.substr(n));
        return false;
    }
    return true;
}

GVCmdExecStatus
GVCmdExec::errorOption(GVCmdOptionError err, const string& opt) const {
    switch (err) {
        case GV_CMD_OPT_MISSING:
            if (opt.size())
                gvMsg(GV_MSG_ERR) << "Missing option \"" << opt << "\" !!" << endl;
            else
                gvMsg(GV_MSG_ERR) << "Missing option"
                                  << "!!" << endl;
            break;
        case GV_CMD_OPT_EXTRA:
            gvMsg(GV_MSG_ERR) << "Extra option \"" << opt << "\" !!" << endl;
            break;
        case GV_CMD_OPT_ILLEGAL:
            gvMsg(GV_MSG_ERR) << "Illegal option \"" << opt << "\" !!" << endl;
            break;
        case GV_CMD_OPT_FOPEN_FAIL:
            gvMsg(GV_MSG_ERR) << "Error: cannot open file \"" << opt << "\" !!"
                              << endl;
            break;
        default:
            gvMsg(GV_MSG_ERR) << "Unknown option error type \"" << err << "\" !!"
                              << endl;
            break;
    }
    return GV_CMD_EXEC_ERROR;
}

bool GVCmdExec::checkCmd(const string& check) const {
    unsigned len = this->getCmdLen();
    bool result  = false;
    size_t space = 0, nxt_space;

    for (unsigned word = 1; word <= len; ++word) {
        nxt_space = check.find(' ', space + 1);
        if (nxt_space == string::npos) nxt_space = check.size();

        for (unsigned i = space + 1; i <= nxt_space; ++i) {
            string checkMand = check.substr(space, i - space);
            string checkOpt  = check.substr(i, nxt_space - space - i);

            if (checkMandCmd(checkMand, word) &&
                (checkOpt.empty() || checkOptCmd(checkOpt, word))) {
                result = true;
                break;
            }
        }
        if (result == false) return false;

        result = false;
        space  = nxt_space + 1;
    }

    return true;
}

bool GVCmdExec::checkCmd(const string& check, size_t idx) const {
    if (idx >= this->getCmdLen()) return false;

    bool result = false;

    for (unsigned i = 1, n = check.size(); i <= n; ++i) {
        string checkMand = check.substr(0, i);
        string checkOpt  = check.substr(i, n + 1 - i);

        if (checkMandCmd(checkMand, idx) &&
            (checkOpt.empty() || checkOptCmd(checkOpt, idx))) {
            result = true;
            break;
        }
    }
    return result;
}

bool GVCmdExec::checkMandCmd(const string& check, size_t idx) const {
    if (check.size() != _mandCmd[idx - 1].size()) return false;
    for (unsigned i = 0, n = _mandCmd[idx - 1].size(); i < n; ++i) {
        if (!check[i]) return true;
        char ch1 = tolower(_mandCmd[idx - 1].at(i));
        char ch2 = tolower(check[i]);
        if (ch1 != ch2) return false;
    }
    return true;
}

bool GVCmdExec::checkOptCmd(const string& check, size_t idx) const {
    if (check.size() > _optCmd[idx - 1].size()) return false;
    for (unsigned i = 0, n = _optCmd[idx - 1].size(); i < n; ++i) {
        if (!check[i]) return true;
        char ch1 = tolower(_optCmd[idx - 1].at(i));
        char ch2 = tolower(check[i]);
        if (ch1 != ch2) return false;
    }
    return true;
}

/* -------------------------------------------------- *\
 * Class GVCmdMgr Implementations
\* -------------------------------------------------- */

GVCmdMgr::GVCmdMgr(const string& p) : _defaultPrompt(p) {
    _prompt = _defaultPrompt;
    _history.clear();
}

GVCmdMgr::~GVCmdMgr() {
    if (_dofile.is_open()) _dofile.close();
}

bool GVCmdMgr::regCmd(const string& cmd, unsigned nCmp, GVCmdExec* e) {
    assert(e);
    assert(cmd.size());
    assert(nCmp);
    // check if cmd contains space
    if (cmd.find(' ') != string::npos) return false;

    unsigned s = cmd.size();
    if (s < nCmp) return false;

    string str = cmd;
    for (unsigned i = 0; i < nCmp; ++i) str[i] = toupper(str[i]);

    e->setCmdLen(1);
    e->setMandCmd(str.substr(0, nCmp));
    e->setOptCmd(str.substr(nCmp));

    GVCmdExecSet::iterator it = _cmdLib.find(e->getGVCmdType());
    GVCmdExecSubSet* cmdSet   = 0;
    if (it == _cmdLib.end()) {
        cmdSet = new GVCmdExecSubSet();
        _cmdLib.insert(make_pair(e->getGVCmdType(), cmdSet));
    } else
        cmdSet = it->second;
    cmdSet->insert(e);

    return true;
}

bool GVCmdMgr::regCmd(const string& cmd, unsigned nCmp1, unsigned nCmp2, GVCmdExec* e) {
    assert(e);
    assert(cmd.size());
    assert(nCmp1);
    assert(nCmp2);
    // check if cmd contains space
    size_t idx = cmd.find(' ');
    if (idx == string::npos) return false;

    string str1 = cmd.substr(0, idx);
    string str2 = cmd.substr(idx + 1);

    // check if cmd2 contains space
    if (str2.find(' ', idx) != string::npos) return false;

    unsigned s1 = str1.size();
    unsigned s2 = str2.size();
    if ((s1 < nCmp1) || (s2 < nCmp2)) return false;

    for (unsigned i = 0; i < nCmp1; ++i) str1[i] = toupper(str1[i]);
    for (unsigned i = 0; i < nCmp2; ++i) str2[i] = toupper(str2[i]);

    e->setCmdLen(2);
    e->setMandCmd(str1.substr(0, nCmp1));
    e->setOptCmd(str1.substr(nCmp1));
    e->setMandCmd(str2.substr(0, nCmp2));
    e->setOptCmd(str2.substr(nCmp2));

    GVCmdExecSet::iterator it = _cmdLib.find(e->getGVCmdType());
    GVCmdExecSubSet* cmdSet   = 0;
    if (it == _cmdLib.end()) {
        cmdSet = new GVCmdExecSubSet();
        _cmdLib.insert(make_pair(e->getGVCmdType(), cmdSet));
    } else
        cmdSet = it->second;
    cmdSet->insert(e);
    return true;
}

GVCmdExecStatus
GVCmdMgr::execOneCmd() {
    // Read User Command Input
    string str    = "";
    char* execCmd = new char[1024];
    if (_dofile.is_open()) {
        getline(_dofile, str);
        strcpy(execCmd, str.c_str());
        // Detect dofile comment(#) and blank command
        if (execCmd[0] != '#' && str.size() > 0)
            cout << getPrompt() << execCmd << endl;
        if (_dofile.eof()) closeDofile();
    } else
        execCmd = readline(getPrompt().c_str());
    assert(execCmd);

    // Detect dofile comment(#) for debugging
    if (execCmd[0] == '#') return GV_CMD_EXEC_NOP;

    if (addHistory(execCmd)) {
        add_history(_history.back().c_str());
        string option = "";
        GVCmdExec* e  = parseCmd(option);
        // Check command types
        if (e) {
            GVCmdType cmdType = e->getGVCmdType();
            if (getPrompt().find("vrf") != string::npos) {
                if (_vrfMode.find(cmdType) == _vrfMode.end()) {
                    cout << "Please switch to \"SETUP MODE\" !!" << endl;
                    return GV_CMD_EXEC_NOP;
                }
            } else {
                if (_setupMode.find(cmdType) == _setupMode.end()) {
                    cout << "Please switch to \"VRF MODE\" !!" << endl;
                    return GV_CMD_EXEC_NOP;
                }
            }
        }

        if (e) return e->exec(option);
    }
    delete[] execCmd;
    cout << endl;

    return GV_CMD_EXEC_NOP;
}

GVCmdExec*
GVCmdMgr::parseCmd(string& option) {
    assert(_history.size());
    string str = _history.back();
    assert(str[0] != 0 && str[0] != ' ');

    GVCmdExec* e = 0;
    string cmd;

    // normalize: keep only one space between two words
    unsigned delCount = 0;
    for (size_t i = 2, n = str.size(); i < n; ++i) {
        if ((str[i - 2] == ' ') && (str[i - 1] == ' ')) {
            for (size_t j = i; j < n; ++j) str[j - 1] = str[j];
            --i;
            ++delCount;
        }
    }
    str.erase(str.length() - delCount);

    // space count: indicates how many words there are in cmd
    unsigned spCount = 0;
    for (size_t i = 0, n = str.size(); i < n; ++i)
        if (str[i] == ' ') ++spCount;

    // try to match commands
    size_t idx = 0;
    for (unsigned i = 0; (i < spCount + 1) && i < 2; ++i) {
        idx = str.find(' ', idx + 1);
        cmd = str.substr(0, idx);

        e = getCmd(cmd);
        if (e) break;
    }

    // can't match any command
    if (!e) {
        // Chengyin added : Support linux commands
        string _cmd = "";
        if (str.size() >= 2) {
            // Support comments in dofile, NO execution
            if ((str[0] == '/') && (str[1] == '/')) return e;
        }
        for (idx = 0; idx < str.size(); ++idx) {
            if (str[idx] == ' ') break;
        }
        _cmd = str.substr(0, idx);
        if ((_cmd == "ls") || (_cmd == "vi") || (_cmd == "vim") ||
            (_cmd == "echo") || (_cmd == "cat") || (_cmd == "clear"))
            system(str.c_str());
        else
            gvMsg(GV_MSG_ERR) << "Illegal command!! (" << str << ")" << endl;
    } else if (idx != string::npos) {
        size_t opt = str.find_first_not_of(' ', idx);
        if (opt != string::npos) option = str.substr(opt);
    }

    return e;
}

GVCmdExec*
GVCmdMgr::getCmd(const string& cmd) const {
    GVCmdExec* e = 0;

    // space count: indicates how many words there are in cmd
    unsigned spCount = 0;
    for (size_t i = 0, n = cmd.size(); i < n; ++i)
        if (cmd[i] == ' ') ++spCount;

    GVCmdExecSet::const_iterator it;
    GVCmdExecSubSet::iterator is;
    for (it = _cmdLib.begin(); it != _cmdLib.end(); ++it) {
        for (is = it->second->begin(); is != it->second->end(); ++is) {
            if (((*is)->getCmdLen() == spCount + 1) && (*is)->checkCmd(cmd)) {
                e = *is;
                break;
            }
        }
    }

    return e;
}

GVCmdExecSubSet
GVCmdMgr::getCmdListFromPart(const string& cmd) const {
    GVCmdExecSubSet result;

    // space count: indicates how many words there are in cmd
    unsigned spCount = 0;
    for (size_t i = 0, n = cmd.size(); i < n; ++i)
        if (cmd[i] == ' ') ++spCount;

    GVCmdExecSet::const_iterator it;
    GVCmdExecSubSet::iterator is;
    for (it = _cmdLib.begin(); it != _cmdLib.end(); ++it) {
        if (it->first != GV_CMD_TYPE_REVEALED) {
            for (is = it->second->begin(); is != it->second->end(); ++is) {
                if (((*is)->getCmdLen() > spCount)) {
                    bool check = true;
                    for (unsigned i = 1; i <= spCount + 1; ++i) {
                        if (!(*is)->checkCmd(cmd, i)) {
                            check = false;
                            break;
                        }
                    }
                    if (check == true) result.insert(*is);
                }
            }
        }
    }
    return result;
}

void GVCmdMgr::printHelps(bool revealed) const {
    GVCmdExecSet::const_iterator it;
    GVCmdExecSubSet::iterator is;
    cout << endl;
    for (it = _cmdLib.begin(); it != _cmdLib.end(); ++it) {
        if ((revealed) ^ (it->first != GV_CMD_TYPE_REVEALED)) {
            cout << "========== " << GVCmdTypeString[it->first]
                 << " Commands : ==========" << endl;
            for (is = it->second->begin(); is != it->second->end(); ++is)
                (*is)->help();
            cout << endl;
        }
    }
}

void GVCmdMgr::printHistory(int nPrint) const {
    int historySize = _history.size();
    if (historySize == 0) {
        cout << "Empty command history!!" << endl;
        return;
    }
    if ((nPrint < 0) || (nPrint > historySize)) nPrint = historySize;
    assert(historySize >= nPrint);
    for (int i = historySize - nPrint; i < historySize; ++i)
        cout << "   " << i << ": " << _history[i] << endl;
}

bool GVCmdMgr::addHistory(char* cmd) {
    // remove ' ' at the end
    char* tmp = &(cmd[strlen(cmd) - 1]);
    while ((tmp >= cmd) && (*tmp == ' ')) *(tmp--) = 0;

    // remove ' ' in the beginning
    tmp = cmd;
    while (*tmp == ' ') ++tmp;

    // add to _history
    if (*tmp != 0) {
        _history.push_back(tmp);
        // gvMsg(MSG_LOG) << tmp << endl;
        return true;
    }
    return false;
}

#endif