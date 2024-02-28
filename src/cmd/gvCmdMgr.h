#ifndef GV_CMD_MGR_H
#define GV_CMD_MGR_H

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

class GVCmdExec;
class GVCmdMgr;

extern GVCmdMgr* gvCmdMgr;

// Command Categories to String
const string GVCmdTypeString[] = {"Revealed", "Common", "Verify", "Simulate", "Network",
                                  "Abc", "Yosys", "Mode", "Bdd", "Prove", "Itp"};

// Command Categories Enum
enum GVCmdType {
    // Revealed command
    GV_CMD_TYPE_REVEALED = 0,
    GV_CMD_TYPE_COMMON   = 1,
    GV_CMD_TYPE_VERIFY   = 2,
    GV_CMD_TYPE_SIMULATE = 3,
    GV_CMD_TYPE_NETWORK  = 4,
    GV_CMD_TYPE_ABC      = 5,
    GV_CMD_TYPE_YOSYS    = 6,
    GV_CMD_TYPE_MOD      = 7,
    GV_CMD_TYPE_BDD      = 8,
    GV_CMD_TYPE_PROVE    = 9,
    GV_CMD_TYPE_ITP      = 10,
};

enum GVCmdExecStatus {
    GV_CMD_EXEC_DONE  = 0,
    GV_CMD_EXEC_ERROR = 1,
    GV_CMD_EXEC_QUIT  = 2,
    GV_CMD_EXEC_NOP   = 3,
};

enum GVCmdOptionError {
    GV_CMD_OPT_MISSING    = 0,
    GV_CMD_OPT_EXTRA      = 1,
    GV_CMD_OPT_ILLEGAL    = 2,
    GV_CMD_OPT_FOPEN_FAIL = 3,
};

const unordered_set<GVCmdType> _setupMode{
    GV_CMD_TYPE_REVEALED,
    GV_CMD_TYPE_COMMON,
    GV_CMD_TYPE_NETWORK,
    GV_CMD_TYPE_ABC,
    GV_CMD_TYPE_MOD,
    GV_CMD_TYPE_BDD,
    GV_CMD_TYPE_YOSYS};

const unordered_set<GVCmdType> _vrfMode{
    GV_CMD_TYPE_VERIFY,
    GV_CMD_TYPE_SIMULATE,
    GV_CMD_TYPE_COMMON,
    GV_CMD_TYPE_MOD,
    GV_CMD_TYPE_PROVE,
    GV_CMD_TYPE_ITP};

#define GV_COMMAND(cmd, type)                  \
    class cmd : public GVCmdExec {             \
    public:                                    \
        cmd() : GVCmdExec(type) {}             \
        ~cmd() {}                              \
        GVCmdExecStatus exec(const string&);   \
        void usage(const bool& = false) const; \
        void help() const;                     \
    };

class GVCmdExec {
public:
    GVCmdExec(GVCmdType t) : _cmdType(t) {}
    virtual ~GVCmdExec() {}
    virtual GVCmdExecStatus exec(const string&)   = 0;
    virtual void usage(const bool& = false) const = 0;
    virtual void help() const                     = 0;

    inline GVCmdType getGVCmdType() const { return _cmdType; };
    inline void setCmdLen(unsigned n) { _cmdLen = n; }
    inline unsigned getCmdLen() const { return _cmdLen; }
    inline void setMandCmd(const string& str) { _mandCmd.push_back(str); }
    inline void setOptCmd(const string& str) { _optCmd.push_back(str); }
    inline const string& getMandCmd(size_t idx) const { return _mandCmd[idx]; }
    inline const string& getOptCmd(size_t idx) const { return _optCmd[idx]; }
    bool checkCmd(const string&) const;
    bool checkCmd(const string&, size_t) const;
    bool checkMandCmd(const string&, size_t) const;
    bool checkOptCmd(const string&, size_t) const;

    bool operator<(const GVCmdExec&) const;

protected:
    void lexOptions(const string&, vector<string>&) const;
    bool lexSingleOption(const string&, string&, bool optional = true) const;
    GVCmdExecStatus errorOption(GVCmdOptionError err, const string& opt) const;

private:
    GVCmdType _cmdType;
    unsigned _cmdLen;
    vector<string> _mandCmd;
    vector<string> _optCmd;
};

struct GVCmdCompare {
    bool operator()(const GVCmdExec* a, const GVCmdExec* b) const { return (*a < *b); }
};
typedef set<GVCmdExec*, GVCmdCompare> GVCmdExecSubSet;
typedef map<GVCmdType, GVCmdExecSubSet*> GVCmdExecSet;

class GVCmdMgr {
#define READ_BUF_SIZE 65536
#define PG_OFFSET 10

public:
    GVCmdMgr(const string&);
    ~GVCmdMgr();
    GVCmdExec* getCmd(const string&) const;
    GVCmdExecSubSet getCmdListFromPart(const string&) const;
    bool regCmd(const string&, unsigned, GVCmdExec*);
    bool regCmd(const string&, unsigned, unsigned, GVCmdExec*);

    GVCmdExecStatus execOneCmd();

    void printHelps(bool = false) const;
    void printHistory(int = -1) const;

    bool openDofile(const string& dof) {
        _dofile.open(dof.c_str());
        return _dofile.is_open();
    }
    void closeDofile() {
        _dofile.close();
        _dofile.clear();
    }

    inline const string& getPrompt() const { return _prompt; }
    inline void updateModPrompt(const string newPromt) { _modPrompt = newPromt; }
    inline void setPrompt() { _prompt = _modPrompt + "> "; }

private:
    // Command Helper Functions
    bool addHistory(char*);
    GVCmdExec* parseCmd(string&);
    // Command Data members
    const string _defaultPrompt;  // Default Command Prompt
    string _modPrompt;            // Current Command Prompt
    string _prompt;               // Current Command Prompt
    ifstream _dofile;             // For Script Parsing
    GVCmdExecSet _cmdLib;
    vector<string> _history;
};

#endif