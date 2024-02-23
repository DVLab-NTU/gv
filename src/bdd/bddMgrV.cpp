/****************************************************************************
  FileName     [ bddMgr.cpp ]
  PackageName  [ ]
  Synopsis     [ BDD Manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "bddMgrV.h"

#include <cassert>
#include <fstream>
#include <iomanip>

#include "bddNodeV.h"
// #include "v3StrUtil.h"
#include "util.h"

using namespace std;

//----------------------------------------------------------------------
//    Global variables
//----------------------------------------------------------------------
BddMgrV* bddMgrV = 0;  // new BddMgrV;

//----------------------------------------------------------------------
//    External functions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//    static functions
//----------------------------------------------------------------------
static void swapBddNodeV(BddNodeV& f, BddNodeV& g) {
    BddNodeV tmp = f;
    f            = g;
    g            = tmp;
}

//----------------------------------------------------------------------
//    helper functions
//----------------------------------------------------------------------
BddNodeV getBddNodeV(const string& bddName) {
    int id;
    // if (v3Str2Int(bddName, id))  // bddName is an ID
    if (myStr2Int(bddName, id))  // bddName is an ID
        return bddMgrV->getBddNodeV(id);
    else  // bddNameis a name
        return bddMgrV->getBddNodeV(bddName);
}

//----------------------------------------------------------------------
//    class BddMgrV
//----------------------------------------------------------------------
// _level = 0 ==> const 1 & const 0
// _level = 1 ~ nin ==> supports
//
void BddMgrV::init(size_t nin, size_t h, size_t c) {
    reset();
    _uniqueTable.init(h);
    _computedTable.init(c);

    // This must be called first
    BddNodeV::setBddMgrV(this);
    BddNodeVInt::_terminal = uniquify(0, 0, 0);
    BddNodeV::_one         = BddNodeV(BddNodeVInt::_terminal, BDD_POS_EDGEV);
    BddNodeV::_zero        = BddNodeV(BddNodeVInt::_terminal, BDD_NEG_EDGEV);

    _supports.reserve(nin + 1);
    _supports.push_back(BddNodeV::_one);
    for (size_t i = 1; i <= nin; ++i)
        _supports.push_back(BddNodeV(BddNodeV::_one(), BddNodeV::_zero(), i));
}

// Called by the BSETOrder command
void BddMgrV::restart() {
    size_t nin = _supports.size() - 1;
    size_t h   = _uniqueTable.numBuckets();
    size_t c   = _computedTable.size();

    init(nin, h, c);
}

// This is a private function called by init() and restart()
void BddMgrV::reset() {
    _supports.clear();
    _bddArr.clear();
    _bddMap.clear();
    BddHash::iterator bi = _uniqueTable.begin();
    for (; bi != _uniqueTable.end(); ++bi)
        delete (*bi).second;
    _uniqueTable.reset();
    _computedTable.reset();
}

// [Note] Remeber to check "isNegEdge" when return BddNodeV!!!!!
//
BddNodeV
BddMgrV::ite(BddNodeV f, BddNodeV g, BddNodeV h) {
    bool isNegEdge = false;  // should only be flipped by "standardize()"

#define DO_STD_ITE 1  // NOTE: make it '0' if you haven't done standardize()!!
    standardize(f, g, h, isNegEdge);

    BddNodeV ret;

    // check terminal cases
    if (checkIteTerminal(f, g, h, ret)) {
        if (isNegEdge) ret = ~ret;
        return ret;  // no need to update tables
    }

    // check computed table
    //       based on your definition of BddCacheKey,
    //       instantiate a BddCacheKey k (i.e. pass in proper data members)
    // BddCacheKey k;  // Change this line!!
    BddCacheKeyV k(f(), g(), h());
    size_t ret_t;
    if (_computedTable.read(k, ret_t)) {
        if (isNegEdge) ret_t = ret_t ^ BDD_NEG_EDGEV;
        return ret_t;
    }

    // check top varaible
    unsigned v = f.getLevel();
    if (g.getLevel() > v)
        v = g.getLevel();
    if (h.getLevel() > v)
        v = h.getLevel();

    // recursion
    BddNodeV fl = f.getLeftCofactor(v),
             gl = g.getLeftCofactor(v),
             hl = h.getLeftCofactor(v);
    BddNodeV t  = ite(fl, gl, hl);

    BddNodeV fr = f.getRightCofactor(v),
             gr = g.getRightCofactor(v),
             hr = h.getRightCofactor(v);
    BddNodeV e  = ite(fr, gr, hr);

    // get result
    if (t == e) {
        // update computed table
        _computedTable.write(k, t());
        if (isNegEdge) t = ~t;
        return t;
    }

    // move bubble if necessary... ==> update isNedEdge
    bool moveBubble = t.isNegEdge();
#if DO_STD_ITE
    assert(!moveBubble);
#else
    if (moveBubble) {
        t = ~t;
        e = ~e;
    }
#endif

    // check unique table
    BddNodeVInt* ni = uniquify(t(), e(), v);
    ret_t           = size_t(ni);
#if !(DO_STD_ITE)
    if (moveBubble) ret_t = ret_t ^ BDD_NEG_EDGEV;
#endif
    // update computed table
    _computedTable.write(k, ret_t);
    if (isNegEdge)
        ret_t = ret_t ^ BDD_NEG_EDGEV;
    return ret_t;
}

void BddMgrV::standardize(BddNodeV& f, BddNodeV& g, BddNodeV& h, bool& isNegEdge) {
    // (1) Identical/Complement rules
    if (f == g)
        g = BddNodeV::_one;
    else if (f == ~g)
        g = BddNodeV::_zero;
    else if (f == h)
        h = BddNodeV::_zero;
    else if (f == ~h)
        h = BddNodeV::_one;

    // (2) Symmetrical rules
    if (g == BddNodeV::_one) {
        if (f > h) swapBddNodeV(f, h);
    } else if (g == BddNodeV::_zero) {
        if (f > h) {
            swapBddNodeV(f, h);
            f = ~f;
            h = ~h;
        }
    } else if (h == BddNodeV::_one) {
        if (f > g) {
            swapBddNodeV(f, g);
            f = ~f;
            g = ~g;
        }
    } else if (h == BddNodeV::_zero) {
        if (f > g) swapBddNodeV(f, g);
    } else if (g == ~h) {
        if (f > g) {
            swapBddNodeV(f, g);
            h = ~g;
        }
    }

    // (3) Complement edge rules
    // ==> both f and g will be posEdge afterwards
    if (f.isNegEdge()) {
        swapBddNodeV(g, h);
        f = ~f;
    }
    if (g.isNegEdge()) {
        g         = ~g;
        h         = ~h;
        isNegEdge = !isNegEdge;
    }
}

// Check if triplet (l, r, i) is in _uniqueTable,
// If not, create a new node;
// else, return the hashed one
//
BddNodeVInt*
BddMgrV::uniquify(size_t l, size_t r, unsigned i) {
    BddNodeVInt* n = 0;
    BddHashKeyV k(l, r, i);
    if (!_uniqueTable.check(k, n)) {
        n = new BddNodeVInt(l, r, i);
        _uniqueTable.forceInsert(k, n);
    }
    return n;
}

// return false if _bddArr[id] has aleady been inserted
bool BddMgrV::addBddNodeV(unsigned id, size_t n) {
    if (id >= _bddArr.size()) {
        unsigned origSize = _bddArr.size();
        _bddArr.resize(id + 1);
        for (unsigned i = origSize; i < _bddArr.size(); ++i)
            _bddArr[i] = 0;
    } else if (_bddArr[id] != 0)
        return false;
    _bddArr[id] = n;
    return true;
}

// return 0 if not in the map!!
BddNodeV
BddMgrV::getBddNodeV(unsigned id) const {
    if (id >= _bddArr.size())
        return size_t(0);
    return _bddArr[id];
}

// return false if str is already in the _bddMap!!
bool BddMgrV::addBddNodeV(const string& str, size_t n) {
    return _bddMap.insert(BddMapPair(str, n)).second;
}

void BddMgrV::forceAddBddNodeV(const string& str, size_t n) {
    _bddMap[str] = n;
}

// return 0 if not in the map!!
BddNodeV
BddMgrV::getBddNodeV(const string& name) const {
    BddMapConstIter bi = _bddMap.find(name);
    if (bi == _bddMap.end()) return size_t(0);
    return (*bi).second;
}

// return true if terminal case
bool BddMgrV::checkIteTerminal(const BddNodeV& f, const BddNodeV& g, const BddNodeV& h, BddNodeV& n) {
    if (g == h) {
        n = g;
        return true;
    }
    if (f == BddNodeV::_one) {
        n = g;
        return true;
    }
    if (f == BddNodeV::_zero) {
        n = h;
        return true;
    }
    if (g == BddNodeV::_one && h == BddNodeV::_zero) {
        n = f;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------
//    Application functions
//----------------------------------------------------------------------
// pattern[0 ~ n-1] ==> _support[1 ~ n]
//
// ==> return -1 if pattern is not legal
// ==> return 0/1 for evaluated result
int BddMgrV::evalCube(const BddNodeV& node, const string& pattern) const {
    size_t v = node.getLevel();
    size_t n = pattern.size();
    if (n < v) {
        cerr << "Error: " << pattern << " too short!!" << endl;
        return -1;
    }

    BddNodeV next = node;
    for (int i = v - 1; i >= 0; --i) {
        char c = pattern[i];
        if (c == '1')
            next = next.getLeftCofactor(i + 1);
        else if (c == '0')
            next = next.getRightCofactor(i + 1);
        else {
            cerr << "Illegal pattern: " << c << "(" << i << ")" << endl;
            return -1;
        }
    }
    return (next == BddNodeV::_one) ? 1 : 0;
}

bool BddMgrV::drawBdd(const string& name, const string& fileName) const {
    BddNodeV node = ::getBddNodeV(name);
    if (node() == 0) {
        cerr << "Error: \"" << name << "\" is not a legal BDD node!!" << endl;
        return false;
    }

    ofstream ofile(fileName.c_str());
    if (!ofile) {
        cerr << "Error: cannot open file \"" << fileName << "\"!!" << endl;
        return false;
    }

    node.drawBdd(name, ofile);

    return true;
}
