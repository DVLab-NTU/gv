#ifndef GV_YOSYS_MGR
#define GV_YOSYS_MGR

#include "gvType.h"
#include "kernel/yosys.h"

using namespace std;
// USING_YOSYS_NAMESPACE

// class YosysMgr;
// extern YosysMgr* yosysMgr;

struct YosysSignal;
struct YosysFSM;
typedef std::vector<YosysSignal*> SignalVec;
typedef std::vector<YosysFSM> FSMVec;

struct DesignInfo {
    DesignInfo(std::string clk, std::string rst) {
        clkName = {"clk", "clock"};
        rstName = {"rst", "reset"};
    }
    DesignInfo(){};
    std::vector<std::string> clkName;
    std::vector<std::string> rstName;
    bool reset;
};

struct YosysSignal {
    YosysSignal(Yosys::RTLIL::Wire* w) : _wire(w) {
        _name  = _wire->name.str().substr(1);
        _id    = _wire->port_id;
        _width = _wire->width;
    };
    inline int getId() { return _id; }
    inline int getWidth() { return _width; }
    inline const std::string& getName() { return _name; }

    Yosys::RTLIL::Wire* _wire;
    std::string _name;
    int _width;
    int _id;
};

struct YosysFSM {
    int stateIn;
    int stateOut;
    std::string ctrlIn;
    std::string ctrlOut;
};

// struct YosysSignal {
//     std::string _name;
//     int _width;
//     int _id;
// };

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
    ~YosysMgr();

    void init();
    void reset();

    void saveDesign(const string&);
    void saveTopModuleName();
    void loadDesign(const string&);
    void loadSimPlugin();
    void deleteDesign(const string&);
    void resetDesign();

    void readBlif(const string&);
    void readVerilog(const string&);
    void readAiger(const string&);
    void writeBlif(const string&);
    void writeAiger(const string&);

    void showSchematic();
    void extractFSM();
    void fetchAllFSMInfo();
    void fetchFSMInfo(Yosys::RTLIL::Cell*);
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
    void printFSM();

private:
    unsigned _property;
    Yosys::RTLIL::Module* _topModule;
    Yosys::Design* _design;
    SignalVec _piList;
    SignalVec _poList;
    SignalVec _clkList;
    SignalVec _rstList;
    SignalVec _regList;
    FSMVec _fsmList;
    DesignInfo _designInfo;
    vector<string> _yosysSigTypeStr;
    vector<std::string> _fileVec;
    std::string _topModuleName;
};

#endif
