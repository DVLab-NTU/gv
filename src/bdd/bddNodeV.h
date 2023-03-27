/****************************************************************************
  FileName     [ bddNodeV.h ]
  PackageName  [ ]
  Synopsis     [ Define basic BDD Node data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BDD_NODEV_H
#define BDD_NODEV_H

#include <vector>
#include <map>
#include <iostream>

using namespace std;

#define BDD_EDGEV_BITS      2
//#define BDD_NODE_PTR_MASKV  ((UINT_MAX >> BDD_EDGEV_BITS) << BDD_EDGEV_BITS)
#define BDD_NODE_PTR_MASKV  ((~(size_t(0)) >> BDD_EDGEV_BITS) << BDD_EDGEV_BITS)

class BddMgrV;
class BddNodeVInt;

enum BDD_EDGEV_FLAG
{
   BDD_POS_EDGEV = 0,
   BDD_NEG_EDGEV = 1,

   BDD_EDGEV_DUMMY  // dummy end
};

class BddNodeV
{
public:
   static BddNodeV          _one;
   static BddNodeV          _zero;
   static bool             _debugBddAddr;
   static bool             _debugRefCount;

   // no node association yet
   BddNodeV() : _nodeV(0) {}
   // We check the hash when a new node is possibly being created
   BddNodeV(size_t l, size_t r, size_t i, BDD_EDGEV_FLAG f = BDD_POS_EDGEV);
   // Copy constructor also needs to increase the _refCount
   BddNodeV(const BddNodeV& n);
   // n must have been uniquified...
   BddNodeV(BddNodeVInt* n, BDD_EDGEV_FLAG f = BDD_POS_EDGEV);
   // 
   BddNodeV(size_t v);
   // Destructor is the only place to decrease _refCount
   ~BddNodeV();

   // Basic access functions
   const BddNodeV& getLeft() const;
   const BddNodeV& getRight() const;
   BddNodeV getLeftCofactor(unsigned i) const;
   BddNodeV getRightCofactor(unsigned i) const;
   unsigned getLevel() const;
   unsigned getRefCount() const;
   bool isNegEdge() const { return (_nodeV & BDD_NEG_EDGEV); }
   bool isPosEdge() const { return !isNegEdge(); }

   // Operators overloading
   size_t operator () () const { return _nodeV; }
   BddNodeV operator ~ () const { return (_nodeV ^ BDD_NEG_EDGEV); }
   BddNodeV& operator = (const BddNodeV& n);
   BddNodeV operator & (const BddNodeV& n) const;
   BddNodeV& operator &= (const BddNodeV& n);
   BddNodeV operator | (const BddNodeV& n) const;
   BddNodeV& operator |= (const BddNodeV& n);
   BddNodeV operator ^ (const BddNodeV& n) const;
   BddNodeV& operator ^= (const BddNodeV& n);
   bool operator == (const BddNodeV& n) const { return (_nodeV == n._nodeV); }
   bool operator != (const BddNodeV& n) const { return (_nodeV != n._nodeV); }
   bool operator < (const BddNodeV& n) const;
   bool operator <= (const BddNodeV& n) const;
   bool operator > (const BddNodeV& n) const { return !((*this) <= n); }
   bool operator >= (const BddNodeV& n) const { return !((*this) < n); }

   // Other BDD operations
   BddNodeV exist(unsigned l) const;
   BddNodeV nodeMove(unsigned fLevel, unsigned tLevel, bool& isMoved) const;
   size_t countCube() const;
   BddNodeV getCube(size_t ith=0) const;
   vector<BddNodeV> getAllCubes() const;
   string toString() const;

   friend ostream& operator << (ostream& os, const BddNodeV& n);

   // For BDD drawing
   void drawBdd(const string&, ofstream&) const;
   string getLabel() const;

   // Static functions
   static void setBddMgrV(BddMgrV* m) { _BddMgrV = m; }

private:
   size_t                  _nodeV;

   // Static data mebers
   static BddMgrV*          _BddMgrV;

   // Private functions
   BddNodeVInt* getBddNodeVInt() const {
      return (BddNodeVInt*)(_nodeV & BDD_NODE_PTR_MASKV); }
   bool isTerminal() const;
   void print(ostream&, size_t, size_t&) const;
   void unsetVisitedRecur() const;
   void drawBddRecur(ofstream&) const;
   BddNodeV existRecur(unsigned l, map<size_t, size_t>&) const;
   BddNodeV nodeMoveRecur(unsigned f, unsigned t, map<size_t, size_t>&) const;
   bool containNode(unsigned b, unsigned e) const;
   bool containNodeRecur(unsigned b, unsigned e) const;
   size_t countCubeRecur(bool phase, map<size_t, size_t>& numCubeMap) const;
   bool getCubeRecur(bool p, size_t& ith, size_t target, BddNodeV& res) const;
   void getAllCubesRecur(bool p, BddNodeV& c, vector<BddNodeV>& aCubes) const;
   bool toStringRecur(bool p, string& str) const;
};

// Private class
class BddNodeVInt
{
   friend class BddNodeV;
   friend class BddMgrV;

   // For const 1 and const 0
   BddNodeVInt() : _level(0), _refCount(0), _visited(0) {}

   // Don't initialize _refCount here...
   // BddNodeV() will call incRefCount() or decRefCount() instead...
   BddNodeVInt(size_t l, size_t r, unsigned ll)
   : _left(l), _right(r), _level(ll), _refCount(0), _visited(0) {}

   const BddNodeV& getLeft() const { return _left; }
   const BddNodeV& getRight() const { return _right; }
   unsigned getLevel() const { return _level; }
   unsigned getRefCount() const { return _refCount; }
   void incRefCount() { ++_refCount; }
   void decRefCount() { --_refCount; }
   bool isVisited() const { return (_visited == 1); }
   void setVisited() { _visited = 1; }
   void unsetVisited() { _visited = 0; }

   BddNodeV              _left;
   BddNodeV              _right;
   unsigned             _level    : 16;
   unsigned             _refCount : 15;
   unsigned             _visited  : 1;

   static BddNodeVInt*   _terminal;
};

#endif // BDD_NODEV_H
