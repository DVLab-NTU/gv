#ifndef GV_YOSYS_MGR
#define GV_YOSYS_MGR

#include "gvType.h"
#include "kernel/yosys.h"

using namespace std;
USING_YOSYS_NAMESPACE

class YosysMgr;
extern YosysMgr* yosysMgr;

struct YosysSignal;
typedef vector<YosysSignal*> SignalVec;

struct DesignInfo {
    DesignInfo(string clk, string rst) {
        clkName = {"clk", "clock"};
        rstName = {"rst", "reset"};
    }
    DesignInfo(){};
    vector<string> clkName;
    vector<string> rstName;
    bool reset;
};

struct YosysSignal {
    YosysSignal(RTLIL::Wire* w) : _wire(w) {
        _name  = _wire->name.str().substr(1);
        _id    = _wire->port_id;
        _width = _wire->width;
    };
    inline int getId() { return _id; }
    inline int getWidth() { return _width; }
    const string& getName() { return _name; }

    RTLIL::Wire* _wire;
    string _name;
    int _width;
    int _id;
};

class YosysMgr {
public:
    enum YosysSigType {
        PI,
        PO,
        CLK,
        RST,
        REG
    };
    YosysMgr();
    ~YosysMgr() {}

    void init();
    void reset();

    void saveDesign(const string&);
    void saveTopModuleName();
    void loadDesign(const string&);
    void loadSimPlugin();

    void readBlif(const string&);
    void readVerilog(const string&);
    void readAiger(const string&);
    void writeBlif(const string&);
    void writeAiger(const string&);

    void showSchematic();
    void printDesignInfo(const bool& = false);
    void createMapping(const string&);
    void runPass(const string&);

    void setLogging(const bool& = false);
    void setSafeProperty(const unsigned& p = 0) { _property = p; };
    void setTopModuleName(const string n) { _topModuleName = n; };

    unsigned getSafeProperty() const { return _property; };
    string getPoName(const unsigned&) const;
    inline YosysSignal* getPi(const unsigned& idx) const { return _piList[idx]; }
    inline YosysSignal* getPo(const unsigned& idx) const { return _poList[idx]; }
    inline YosysSignal* getClk(const unsigned& idx) const { return _clkList[idx]; }
    inline YosysSignal* getRst(const unsigned& idx) const { return _rstList[idx]; }
    inline YosysSignal* getReg(const unsigned& idx) const { return _regList[idx]; }
    inline unsigned getNumPIs() { return _piList.size(); }
    inline unsigned getNumPOs() { return _poList.size(); }
    inline unsigned getNumCLKs() { return _clkList.size(); }
    inline unsigned getNumRSTs() { return _rstList.size(); }
    inline unsigned getNumREGs() { return _regList.size(); }
    inline string getTopModuleName() { return _topModuleName; };

    void assignSignal();
    void printSignal(const YosysSigType&);

private:
    unsigned _property;
    RTLIL::Module* _topModule;
    SignalVec _piList;
    SignalVec _poList;
    SignalVec _clkList;
    SignalVec _rstList;
    SignalVec _regList;
    DesignInfo _designInfo;
    vector<string> _yosysSigTypeStr;
    string _topModuleName;
};

#endif
