/****************************************************************************
  FileName     [ bddMgrV.h ]
  PackageName  [ ]
  Synopsis     [ Define BDD Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_MGRV_H
#define BDD_MGRV_H

#include <map>

#include "bddNodeV.h"
#include "myHashMap.h"

using namespace std;

class BddNodeV;

typedef vector<size_t> BddArr;
typedef map<string, size_t> BddMap;
typedef pair<string, size_t> BddMapPair;
typedef map<string, size_t>::const_iterator BddMapConstIter;

extern BddMgrV* bddMgrV;

class BddHashKeyV {
public:
    // HINT: define constructor(s)
    BddHashKeyV(size_t l, size_t r, unsigned i) : _l(l), _r(r), _i(i) {}

    // HINT: implement "()" and "==" operators
    // Get a size_t number;
    // ==> to get bucket number, need to % _numBuckers in Hash
    size_t operator()() const { return ((_l << 3) + (_r << 3) + _i); }

    bool operator==(const BddHashKeyV& k) {
        return (_l == k._l) && (_r == k._r) && (_i == k._i);
    }

private:
    // HINT: define your own data members
    size_t _l;
    size_t _r;
    unsigned _i;
};

class BddCacheKeyV {
public:
    // HINT: define constructor(s)
    BddCacheKeyV() {}
    BddCacheKeyV(size_t f, size_t g, size_t h) : _f(f), _g(g), _h(h) {}

    // HINT: implement "()" and "==" operators
    // Get a size_t number;
    // ==> to get cache address, need to % _size in Cache
    size_t operator()() const { return ((_f << 3) + (_g << 3) + (_h << 3)); }

    bool operator==(const BddCacheKeyV& k) const {
        return (_f == k._f) && (_g == k._g) && (_h == k._h);
    }

private:
    // HINT: define your own data members
    size_t _f;
    size_t _g;
    size_t _h;
};

class BddMgrV {
    typedef HashMap<BddHashKeyV, BddNodeVInt*> BddHash;
    typedef Cache<BddCacheKeyV, size_t> BddCache;

public:
    BddMgrV(size_t nin = 128, size_t h = 8009, size_t c = 30011) { init(nin, h, c); }
    ~BddMgrV() { reset(); }

    void init(size_t nin, size_t h, size_t c);
    void restart();

    // for building BDDs
    BddNodeV ite(BddNodeV f, BddNodeV g, BddNodeV h);

    // for _supports
    const BddNodeV& getSupport(size_t i) const { return _supports[i]; }
    size_t getNumSupports() const { return _supports.size(); }

    // for _uniqueTable
    BddNodeVInt* uniquify(size_t l, size_t r, unsigned i);

    // for _bddArr: access by unsigned (ID)
    bool addBddNodeV(unsigned id, size_t nodeV);
    BddNodeV getBddNodeV(unsigned id) const;

    // for _bddMap: access by string
    bool addBddNodeV(const string& nodeName, size_t nodeV);
    void forceAddBddNodeV(const string& nodeName, size_t nodeV);
    BddNodeV getBddNodeV(const string& nodeName) const;

    // Applications
    int evalCube(const BddNodeV& node, const string& vector) const;
    bool drawBdd(const string& nodeName, const string& dotFile) const;

    // For prove
    void buildPInitialState();
    void buildPTransRelation();
    void buildPImage(int level);
    BddNodeV restrict(const BddNodeV&, const BddNodeV&);
    void runPCheckProperty(const string& name, BddNodeV property);
    bool isPFixed() const { return _isFixed; }
    BddNodeV getPInitState() const { return _initState; }
    BddNodeV getPTr() const { return _tr; }
    BddNodeV getPTri() const { return _tri; }
    BddNodeV getPReachState() const {
        return (_reachStates.empty()) ? _initState : _reachStates.back();
    }
    void resetProof() {}
    BddNodeV find_ns(BddNodeV cs);
    BddNodeV ns_to_cs(BddNodeV ns);
    // string getNsBdd(size_t t) {
    //    map<string, size_t>::iterator it;
    //    for (it=_bddMap.begin(); it!=_bddMap.end(); ++it) {
    //       if(it->second == t) return it->first; break;
    //    }
    //    return "";
    // }

private:
    // level = 0: const 1;
    // level = 1: lowest input variable
    // level = nin: highest input variable
    vector<BddNodeV> _supports;
    BddHash _uniqueTable;
    BddCache _computedTable;

    BddArr _bddArr;
    BddMap _bddMap;

    // For prove
    bool _isFixed;
    BddNodeV _initState;
    BddNodeV _tr;
    BddNodeV _tri;
    vector<BddNodeV> _reachStates;

    void reset();
    bool checkIteTerminal(const BddNodeV&, const BddNodeV&, const BddNodeV&,
                          BddNodeV&);
    void standardize(BddNodeV& f, BddNodeV& g, BddNodeV& h, bool& isNegEdge);
};

#endif  // BDD_MGRV_H
