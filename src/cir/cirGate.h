/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "cirDef.h"
// #include "gvSat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirPValue {
public:
    CirPValue(size_t v = 0) : _pValue(v) {}

    void rand();

    inline size_t operator()() const { return _pValue; }
    inline CirPValue operator~() const { return ~_pValue; }
    inline CirPValue operator&(const CirPValue& p) const {
        size_t v = _pValue & p._pValue;
        return v;
    }
    inline CirPValue& operator&=(const CirPValue& v) {
        _pValue &= v._pValue;
        return *this;
    }
    inline CirPValue& operator=(const CirPValue& v) {
        _pValue = v._pValue;
        return *this;
    }
    inline bool operator==(const CirPValue& v) const {
        return (_pValue == v._pValue);
    }

    friend ostream& operator<<(ostream& os, const CirPValue& v);

private:
    size_t _pValue;
};

class CirGateV {
public:
    CirGateV(size_t v = 0) : _gateV(v) {}

    size_t operator()() const { return _gateV; }
    CirGate* gate() const { return (CirGate*)(_gateV & AIG_PTR_MASK); }
    unsigned gateId() const;
    unsigned litId() const;
    bool isInv() const { return (_gateV & AIG_NEG_FANIN); }
    bool isPos() const { return !(_gateV & AIG_NEG_FANIN); }
    void setInv() { _gateV |= AIG_NEG_FANIN; }
    // if inv == true, flip the phase
    void setInv(bool inv) {
        if (inv) _gateV ^= AIG_NEG_FANIN;
    }
    bool isFloat() const { return (_gateV & AIG_FLT_FANIN); }
    void setFloat() { _gateV |= AIG_FLT_FANIN; }
    bool operator==(const CirGateV& v) const { return (_gateV == v._gateV); }
    bool operator!=(const CirGateV& v) const { return (_gateV != v._gateV); }
    bool operator<(const CirGateV& v) const { return (_gateV < v._gateV); }

    // inv ==> to reverse phase
    void set(CirGate* g, bool inv) {
        _gateV = size_t(g);
        if (inv) setInv();
    }
    void replace(CirGate* g, bool inv) { _gateV = size_t(g) + (inv ^ isInv()); }

    friend ostream& operator<<(ostream&, const CirGateV&);

    static const size_t AIG_EDGE_BITS = 2;
    static const size_t AIG_NEG_FANIN = 0x1;
    static const size_t AIG_FLT_FANIN = 0x2;
    static const size_t AIG_PTR_MASK =
        (~(size_t(0)) >> AIG_EDGE_BITS) << AIG_EDGE_BITS;

private:
    size_t _gateV;
};

class CirGate {
public:
    // CirGate(unsigned g, unsigned l) : _gid(g), _lineNo(l), _fecId(UINT_MAX),
    //                                   _ref(0), _pValue(0), _eqGate(0), _satVar(0) {}
    CirGate(unsigned g, unsigned l) : _gid(g), _lineNo(l), _fecId(UINT_MAX),
                                      _ref(0), _pValue(0), _eqGate(0) {}
    virtual ~CirGate() {}

    // Basic access methods
    //   void setGid(unsigned i) { _gid = i; }
    //   void setLineNo(unsigned l) { _lineNo = l; }
    unsigned getGid() const { return _gid; }
    unsigned getLineNo() const { return _lineNo; }
    virtual GateType getType() const  = 0;
    virtual string getTypeStr() const = 0;
    virtual void setIn0(CirGate*, bool inv){};
    virtual void setIn1(CirGate*, bool inv){};
    virtual CirGateV getIn0() const { return 0; }
    virtual CirGateV getIn1() const { return 0; }
    virtual CirGate* getIn0Gate() const { return 0; }
    virtual CirGate* getIn1Gate() const { return 0; }
    virtual unsigned getNumFanins() const { return 0; }
    virtual char* getName() const { return 0; }
    virtual bool isPi() const { return false; }
    virtual bool isPo() const { return false; }
    virtual bool isAig() const { return false; }
    virtual bool isConst() const { return false; }
    virtual bool isUndef() const { return false; }

    // Methods about circuit construction
    virtual void genConnections() {}
    virtual void genDfsList(vector<CirGate*>&) { setToGlobalRef(); }

    // Methods about circuit optimization
    // virtual CirGateV optimize(bool phase, GateList&) {
    //     setToGlobalRef();
    //     return size_t(this) + (phase ? 1 : 0);
    // }
    void merge(const string&, CirGate*, bool);
    // void replace(CirGate*, bool);
    void removeFanout(CirGate*) const;

    // Methods about circuit simulation
    virtual void pSim() {}
    void setPValue(const CirPValue& v) { _pValue = v; }
    const CirPValue& getPValue() const { return _pValue; }

    // Methods about FRAIG
    void setFECId(unsigned i) { _fecId = i; }
    void resetFECId() { _fecId = UINT_MAX; }
    bool isFEC() const { return (_fecId != UINT_MAX); }
    unsigned getFECId() const { return _fecId; }
    void setEqGate(CirGate* g, bool inv) {
        CirGateV nv;
        while ((nv = g->getEqGate()) != 0) {
            g = nv.gate();
            inv ^= nv.isInv();
        }
        _eqGate.set(g, inv);
    }
    void resetEqGate() { _eqGate = 0; }
    const CirGateV& getEqGate() const { return _eqGate; }
    // void setSatVar(Var v) { _satVar = v; }
    // Var getSatVar() const { return _satVar; }

    // Printing functions
    virtual void printGate() const = 0;
    void reportGate() const;
    void reportFanin(int level) const;
    void reportFanout(int level) const;

    // Methods about _globalRef_s
    bool isGlobalRef() const { return (_ref == _globalRef_s); }
    bool isGlobalRef(unsigned diff) const {
        return ((_ref + diff) == _globalRef_s);
    }
    void setToGlobalRef() const { _ref = _globalRef_s; }
    void setToGlobalRef(unsigned diff) const {
        _ref = unsigned(_globalRef_s - diff);
    }
    static void setGlobalRef() { _globalRef_s++; }
    static void setGlobalRef(unsigned offset) { _globalRef_s += offset; }

    //   friend ostream& operator << (ostream& os, const CirAigGate* g);

private:
    // Printing functions
    void reportFaninRecur(int, int, bool) const;
    void reportFanoutRecur(int, int, bool) const;

protected:
    unsigned _gid;  // var ID
    unsigned _lineNo;
    unsigned _fecId;  // _fecId/2 => FEC ID; _fecId&1 => phase
    mutable unsigned _ref;
    CirPValue _pValue;
    CirGateV _eqGate;
    // Var _satVar;

    static unsigned _globalRef_s;

    // Protected methods about circuit optimization
    // [Note] The original fanin will become invalid!!
    virtual void replaceFanin(CirGate*, CirGate*, bool) {}
};

class CirPiGate : public CirGate {
public:
    CirPiGate(unsigned g, unsigned l) : CirGate(g, l), _name(0) {}
    ~CirPiGate() {
        if (_name) delete[] _name;
    }

    // Basic access methods
    GateType getType() const { return PI_GATE; }
    string getTypeStr() const { return "PI"; }
    void setName(char* s) { _name = s; }
    char* getName() const { return _name; }
    bool isPi() const { return true; }

    // Methods about circuit construction
    void genDfsList(vector<CirGate*>&);

    // Methods about circuit simulation
    void setRandPPattern() { _pValue.rand(); }

    // Printing functions
    void printGate() const;

private:
    char* _name;
};

class CirPoGate : public CirGate {
public:
    CirPoGate(unsigned g, unsigned l, size_t i = 0)
        : CirGate(g, l), _in0(i), _name(0) {}
    ~CirPoGate() {
        if (_name) delete[] _name;
    }

    // Basic access methods
    GateType getType() const { return PO_GATE; }
    string getTypeStr() const { return "PO"; }
    CirGateV getIn0() const { return _in0; }
    CirGate* getIn0Gate() const { return _in0.gate(); }
    unsigned getNumFanins() const { return 1; }
    void setName(char* s) { _name = s; }
    char* getName() const { return _name; }
    bool isPo() const { return true; }
    void setIn0(size_t i) { _in0 = i; }
    void setIn0(CirGate* faninGate, bool inv = false);

    // Methods about circuit construction
    void genConnections();
    void genDfsList(vector<CirGate*>&);

    // Methods about circuit simulation
    void pSim() {
        _pValue = _in0.isInv() ? ~(_in0.gate()->getPValue())
                               : _in0.gate()->getPValue();
    }

    // Methods about circuit optimization
    // CirGateV optimize(bool, GateList&);

    // Printing functions
    void printGate() const;

private:
    CirGateV _in0;
    char* _name;

    // Private methods about circuit optimization
    // [Note] old must == _in0, DO NOT CHECK!
    void replaceFanin(CirGate*, CirGate* n, bool inv) {
        _in0.replace(n, inv);
        if (n->isUndef()) _in0.setFloat();
    }
};

class CirRoGate : public CirGate {
public:
    CirRoGate(unsigned g, unsigned l, size_t i = 0)
        : CirGate(g, l), _in0(i), _name(0) {}
    ~CirRoGate() {
        if (_name) delete[] _name;
    }

    // Basic access methods
    GateType getType() const { return RO_GATE; }
    string getTypeStr() const { return "RO"; }
    CirGateV getIn0() const { return _in0; }
    CirGate* getIn0Gate() const { return _in0.gate(); }
    unsigned getNumFanins() const { return 1; }
    void setName(char* s) { _name = s; }
    char* getName() const { return _name; }
    bool isRo() const { return true; }
    void setIn0(size_t i) { _in0 = i; }
    void setIn0(CirGate* faninGate, bool inv = false);

    // Methods about circuit construction
    void genDfsList(vector<CirGate*>&);

    // Methods about circuit simulation
    void pSim() {
        _pValue = _in0.isInv() ? ~(_in0.gate()->getPValue())
                               : _in0.gate()->getPValue();
    }
    void zInitPPattern() { _pValue = 0; }  // zero initiate the simulattion value

    // Printing functions
    void printGate() const;

private:
    CirGateV _in0;
    char* _name;

    // Private methods about circuit optimization
    // [Note] old must == _in0, DO NOT CHECK!
    void replaceFanin(CirGate*, CirGate* n, bool inv) {
        _in0.replace(n, inv);
        if (n->isUndef()) _in0.setFloat();
    }
};

class CirRiGate : public CirGate {
public:
    CirRiGate(unsigned g, unsigned l, size_t i = 0)
        : CirGate(g, l), _in0(i), _name(0) {}
    ~CirRiGate() {
        if (_name) delete[] _name;
    }

    // Basic access methods
    GateType getType() const { return RI_GATE; }
    string getTypeStr() const { return "RI"; }
    CirGateV getIn0() const { return _in0; }
    CirGate* getIn0Gate() const { return _in0.gate(); }
    unsigned getNumFanins() const { return 1; }
    void setName(char* s) { _name = s; }
    char* getName() const { return _name; }
    bool isRi() const { return true; }
    void setIn0(size_t i) { _in0 = i; }
    void setIn0(CirGate* faninGate, bool inv = false);

    // Methods about circuit construction
    void genConnections();
    void genDfsList(vector<CirGate*>&);

    // Methods about circuit simulation
    void pSim() {
        _pValue = _in0.isInv() ? ~(_in0.gate()->getPValue())
                               : _in0.gate()->getPValue();
    }

    // Methods about circuit optimization
    // CirGateV optimize(bool, GateList&);

    // Printing functions
    void printGate() const;

private:
    CirGateV _in0;
    char* _name;

    // Private methods about circuit optimization
    // [Note] old must == _in0, DO NOT CHECK!
    void replaceFanin(CirGate*, CirGate* n, bool inv) {
        _in0.replace(n, inv);
        if (n->isUndef()) _in0.setFloat();
    }
};

class CirAigGate : public CirGate {
public:
    CirAigGate(unsigned g, unsigned l) : CirGate(g, l), _name(0) {}
    ~CirAigGate() {
        if (_name) delete[] _name;
    }
    CirGateV getIn0() const { return _in0; }
    CirGateV getIn1() const { return _in1; }
    CirGate* getIn0Gate() const { return _in0.gate(); }
    CirGate* getIn1Gate() const { return _in1.gate(); }
    unsigned getNumFanins() const { return 2; }
    void setIn0(size_t i) { _in0 = i; }
    void setIn1(size_t i) { _in1 = i; }
    void setIn0(CirGate* faninGate, bool inv = false);
    void setIn1(CirGate* faninGate, bool inv = false);
    void setName(char* s) { _name = s; }
    char* getName() const { return _name; }

    // Basic access methods
    GateType getType() const { return AIG_GATE; }
    string getTypeStr() const { return "AIG"; }
    bool isAig() const { return true; }

    // Methods about circuit construction
    void genConnections();
    void genDfsList(vector<CirGate*>&);

    // Methods about circuit optimization
    // CirGateV optimize(bool, GateList&);

    // Methods about circuit simulation
    void pSim() {
        _pValue = _in0.isInv() ? ~(_in0.gate()->getPValue())
                               : _in0.gate()->getPValue();
        _pValue &= _in1.isInv() ? ~(_in1.gate()->getPValue())
                                : _in1.gate()->getPValue();
    }

    // Printing functions
    void printGate() const;

private:
    CirGateV _in0;
    CirGateV _in1;
    char* _name;

    // Private methods about circuit optimization
    void replaceFanin(CirGate* o, CirGate* n, bool inv) {
        if (o == _in0.gate()) {
            _in0.replace(n, inv);
            if (n->isUndef()) _in0.setFloat();
        } else {
            _in1.replace(n, inv);
            if (n->isUndef()) _in1.setFloat();
        }
    }
};

class CirConstGate : public CirGate {
public:
    CirConstGate(unsigned g) : CirGate(g, 0) {}  // _satVar = 0
    ~CirConstGate() {}

    // Basic access methods
    GateType getType() const { return CONST_GATE; }
    string getTypeStr() const { return "CONST"; }
    bool isConst() const { return true; }

    // Methods about circuit construction
    void genDfsList(vector<CirGate*>&);

    // Printing functions
    void printGate() const;

private:
    char* _name;
};

class CirUndefGate : public CirGate {
public:
    CirUndefGate(unsigned g) : CirGate(g, 0) {}
    ~CirUndefGate() {}

    // Basic access methods
    GateType getType() const { return UNDEF_GATE; }
    string getTypeStr() const { return "UNDEF"; }
    bool isUndef() const { return true; }

    // Methods about circuit construction

    // Printing functions
    void printGate() const {
        cerr << "CirUndefGate::printGate() is called!!" << endl;
        exit(-1);
    }

private:
};

struct GateIdCmp {
public:
    GateIdCmp() {}

    bool operator()(const CirGateV& v0, const CirGateV& v1) {
        return (v0.gateId() < v1.gateId());
    }
};

#endif  // CIR_GATE_H
