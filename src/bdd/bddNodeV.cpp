/****************************************************************************
  FileName     [ bddNode.cpp ]
  PackageName  [ ]
  Synopsis     [ Define BDD Node member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "bddNodeV.h"
#include "bddMgrV.h"

// Initialize static data members
//
BddMgrV* BddNodeV::_BddMgrV = 0;
BddNodeVInt* BddNodeVInt::_terminal = 0;
BddNodeV BddNodeV::_one;
BddNodeV BddNodeV::_zero;
bool BddNodeV::_debugBddAddr = false;
bool BddNodeV::_debugRefCount = false;

// We check the hash when a new BddNodeVInt is possibly being created
BddNodeV::BddNodeV(size_t l, size_t r, size_t i, BDD_EDGEV_FLAG f)
{
   BddNodeVInt* n = _BddMgrV->uniquify(l, r, i);
   // n should not = 0
   assert(n != 0);
   _nodeV = size_t(n) + f;
   n->incRefCount();
}

// Copy constructor also needs to increase the _refCount
// Need to check if n._nodeV != 0
BddNodeV::BddNodeV(const BddNodeV& n) : _nodeV(n._nodeV)
{
   BddNodeVInt* t = getBddNodeVInt();
   if (t)
      t->incRefCount();
}

// This function is called only when n is obtained from uniquify()...
// ==> n should not be "0"
BddNodeV::BddNodeV(BddNodeVInt* n, BDD_EDGEV_FLAG f)
{
   assert(n != 0);
   _nodeV = size_t(n) + f;
   n->incRefCount();
}

// Need to check if n._nodeV != 0
BddNodeV::BddNodeV(size_t v) : _nodeV(v)
{
   BddNodeVInt* n = getBddNodeVInt();
   if (n)
      n->incRefCount();
}

// Need to check if _nodeV != 0
BddNodeV::~BddNodeV()
{
   BddNodeVInt* n = getBddNodeVInt();
   if (n)
      n->decRefCount();
}

const BddNodeV&
BddNodeV::getLeft() const
{
   assert(getBddNodeVInt() != 0);
   return getBddNodeVInt()->getLeft();
}

const BddNodeV&
BddNodeV::getRight() const
{
   assert(getBddNodeVInt() != 0);
   return getBddNodeVInt()->getRight();
}

// [Note] i SHOULD NOT < getLevel()
BddNodeV
BddNodeV::getLeftCofactor(unsigned i) const
{
   assert(getBddNodeVInt() != 0);
   assert(i > 0);
   if (i > getLevel()) return (*this);
   if (i == getLevel())
      return isNegEdge()? ~getLeft() : getLeft();
   BddNodeV t = getLeft().getLeftCofactor(i);
   BddNodeV e = getRight().getLeftCofactor(i);
   if (t == e) return isNegEdge()? ~t : t;
   BDD_EDGEV_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      BDD_NEG_EDGEV: BDD_POS_EDGEV;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return BddNodeV(t(), e(), getLevel(), f);
}

// [Note] i SHOULD NOT < getLevel()
BddNodeV
BddNodeV::getRightCofactor(unsigned i) const
{
   assert(getBddNodeVInt() != 0);
   assert(i > 0);
   if (i > getLevel()) return (*this);
   if (i == getLevel())
      return isNegEdge()? ~getRight() : getRight();
   BddNodeV t = getLeft().getRightCofactor(i);
   BddNodeV e = getRight().getRightCofactor(i);
   if (t == e) return isNegEdge()? ~t : t;
   BDD_EDGEV_FLAG f = (isNegEdge() ^ t.isNegEdge())?
                      BDD_NEG_EDGEV: BDD_POS_EDGEV;
   if (t.isNegEdge()) { t = ~t; e = ~e; }
   return BddNodeV(t(), e(), getLevel(), f);
}

unsigned
BddNodeV::getLevel() const
{
   return getBddNodeVInt()->getLevel();
}

unsigned
BddNodeV::getRefCount() const
{
   return getBddNodeVInt()->getRefCount();
}

// Note: BddNodeV a = b;
// ==> a's original BddNodeVInt's reference count should --
//     a's new BddNodeVInt's reference count should ++
BddNodeV&
BddNodeV::operator = (const BddNodeV& n)
{
   BddNodeVInt* t = getBddNodeVInt();
   if (t)
      t->decRefCount();
   _nodeV = n._nodeV;
   t = getBddNodeVInt();
   if (t)
      t->incRefCount();
   return (*this);
}

BddNodeV
BddNodeV::operator & (const BddNodeV& n) const
{
   return _BddMgrV->ite((*this), n, BddNodeV::_zero);
}

BddNodeV&
BddNodeV::operator &= (const BddNodeV& n)
{
   (*this) = (*this) & n;
   return (*this);
}

BddNodeV
BddNodeV::operator | (const BddNodeV& n) const
{
   return _BddMgrV->ite((*this), BddNodeV::_one, n);
}

BddNodeV&
BddNodeV::operator |= (const BddNodeV& n)
{
   (*this) = (*this) | n;
   return (*this);
}

BddNodeV
BddNodeV::operator ^ (const BddNodeV& n) const
{
   return _BddMgrV->ite((*this), ~n, n);
}

BddNodeV&
BddNodeV::operator ^= (const BddNodeV& n)
{
   (*this) = (*this) ^ n;
   return (*this);
}

// (*this) < n iff
// (i) level < n.level
// (ii) level == n.level && _nodeV < n._nodeV
bool
BddNodeV::operator < (const BddNodeV& n) const
{
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_nodeV < n._nodeV)));
}

// (*this) < n iff
// (i) level < n.level
// (ii) level == n.level && _nodeV <= n._nodeV
bool
BddNodeV::operator <= (const BddNodeV& n) const
{
   unsigned l1 = getLevel();
   unsigned l2 = n.getLevel();
   return ((l1 < l2) ||
          ((l1 == l2) && (_nodeV <= n._nodeV)));
}

bool
BddNodeV::isTerminal() const
{
   return (getBddNodeVInt() == BddNodeVInt::_terminal);
}

BddNodeV
BddNodeV::exist(unsigned l) const
{
   if (l == 0) return (*this);

   map<size_t, size_t> existMap;
   return existRecur(l, existMap);
}

BddNodeV
BddNodeV::existRecur(unsigned l, map<size_t, size_t>& existMap) const
{
   if (isTerminal()) return (*this);

   unsigned thisLevel = getLevel();
   if (l > thisLevel) return (*this);

   map<size_t, size_t>::iterator mi = existMap.find(_nodeV);
   if (mi != existMap.end()) return (*mi).second;

   BddNodeV left = getLeftCofactor(thisLevel);
   BddNodeV right = getRightCofactor(thisLevel);
   if (l == thisLevel) {
      BddNodeV res = left | right;
      existMap[_nodeV] = res();
      return res;
   }

   // l must < getLevel()
   bool isNegEdge = false;
   BddNodeV t = left.existRecur(l, existMap);
   BddNodeV e = right.existRecur(l, existMap);
   if (t == e) {
      existMap[_nodeV] = t();
      return t;
   }
   if (t.isNegEdge()) {
      t = ~t; e = ~e; isNegEdge = true;
   }
   BddNodeV res(_BddMgrV->uniquify(t(), e(), thisLevel));
   if (isNegEdge) res = ~res;
   existMap[_nodeV] = res();
   return res;
}

// Move the BDD nodes in the cone >= fromLevel to toLevel.
// After the move, there will be no BDD nodes between [fromLevel, toLevel).
// Return the resulted BDD node.
//
// [Note] If (fromLevel > toLevel) ==> move down
//        If (fromLevel < toLevel) ==> move up
//
// Before the move, need to make sure:
// 1. (thisLevel - fromLevel) < abs(fromLevel - toLevel)
// 2. There is no node < fromLevel (except for the terminal node).
//
// [Note] If (move up), there is no node above toLevel in the beginning.
//
// If any of the above is violated, isMoved will be set to false,
// no move will be made, and return (*this).
//
BddNodeV
BddNodeV::nodeMove(unsigned fromLevel, unsigned toLevel, bool& isMoved) const
{
   assert(fromLevel > 1);
   if (int(getLevel() - fromLevel) >= abs(int(fromLevel - toLevel)) ||
        containNode(fromLevel - 1, 1)) {
      isMoved = false;
      return (*this);
   }

   isMoved = true;
   map<size_t, size_t> moveMap;
   return nodeMoveRecur(fromLevel, toLevel, moveMap);
}

BddNodeV
BddNodeV::nodeMoveRecur
(unsigned fromLevel, unsigned toLevel, map<size_t, size_t>& moveMap) const
{
   unsigned thisLevel = getLevel();
   assert(thisLevel >= fromLevel);

   map<size_t, size_t>::iterator mi = moveMap.find(_nodeV);
   if (mi != moveMap.end()) return (*mi).second;

   BddNodeV left = getLeft();
   BddNodeV right = getRight();

   if (!left.isTerminal())
      left = left.nodeMoveRecur(fromLevel, toLevel, moveMap);
   if (!right.isTerminal())
      right = right.nodeMoveRecur(fromLevel, toLevel, moveMap);

   BddNodeVInt *n
   = _BddMgrV->uniquify(left(), right(), thisLevel - fromLevel + toLevel);
   BddNodeV ret = BddNodeV(size_t(n));
   if (isNegEdge()) ret = ~ret;

   moveMap[_nodeV] = ret();
   return ret;
}

// Check if there is any BddNodeV in the cone of level [bLevel, eLevel]
// return true if any
//
bool
BddNodeV::containNode(unsigned bLevel, unsigned eLevel) const
{
   bool res = containNodeRecur(bLevel, eLevel);
   unsetVisitedRecur();
   return res;
}

bool
BddNodeV::containNodeRecur(unsigned bLevel, unsigned eLevel) const
{
   BddNodeVInt* n = getBddNodeVInt();
   if (n->isVisited())
      return false;
   n->setVisited();

   unsigned thisLevel = getLevel();
   if (thisLevel < bLevel) return false;
   if (thisLevel <= eLevel) return true;

   if (getLeft().containNodeRecur(bLevel, eLevel)) return true;
   if (getRight().containNodeRecur(bLevel, eLevel)) return true;

   return false;
}     
   
size_t
BddNodeV::countCube() const
{  
   map<size_t, size_t> numCubeMap;
   return countCubeRecur(false, numCubeMap);
}  

size_t
BddNodeV::countCubeRecur(bool phase, map<size_t, size_t>& numCubeMap) const
{
   if (isTerminal())
      return ((phase ^ isNegEdge())? 0 : 1); 

   map<size_t, size_t>::iterator mi = numCubeMap.find(_nodeV);
   if (mi != numCubeMap.end()) return (*mi).second;

   unsigned numCube = 0;
   BddNodeV left = getLeft();
   numCube += left.countCubeRecur(phase ^ isNegEdge(), numCubeMap);
   BddNodeV right = getRight();
   numCube += right.countCubeRecur(phase ^ isNegEdge(), numCubeMap);

   numCubeMap[_nodeV] = numCube;
   return numCube;
}

BddNodeV
BddNodeV::getCube(size_t ith) const
{  
   ith %= countCube();
   BddNodeV res = BddNodeV::_one;
   size_t idx = 0;
   getCubeRecur(false, idx, ith, res);
   return res;
}
   
// return true if the target-th cube is met
bool
BddNodeV::getCubeRecur
(bool phase, size_t& ith, size_t target, BddNodeV& res) const
{
   if (isTerminal()) {
      if (!(phase ^ isNegEdge())) {
         if (ith == target) return true;
         ++ith;
      }
      return false;
   }

   BddNodeV old = res;
   BddNodeV left = getLeft();
   res = old & _BddMgrV->getSupport(getLevel());
   if (left.getCubeRecur(phase ^ isNegEdge(), ith, target, res))
      return true;
   BddNodeV right = getRight();
   res = old & ~(_BddMgrV->getSupport(getLevel()));
   if (right.getCubeRecur(phase ^ isNegEdge(), ith, target, res))
      return true;

   return false;
}

vector<BddNodeV>
BddNodeV::getAllCubes() const
{
   vector<BddNodeV> allCubes;
   BddNodeV cube = BddNodeV::_one;
   getAllCubesRecur(false, cube, allCubes);
   return allCubes;
}

void
BddNodeV::getAllCubesRecur
(bool phase, BddNodeV& cube, vector<BddNodeV>& allCubes) const
{
   if (isTerminal()) {
      if (!(phase ^ isNegEdge()))
         allCubes.push_back(cube);
      return;
   }

   BddNodeV old = cube;
   BddNodeV left = getLeft();
   cube = old & _BddMgrV->getSupport(getLevel());
   left.getAllCubesRecur(phase ^ isNegEdge(), cube, allCubes);
   BddNodeV right = getRight();
   cube = old & ~(_BddMgrV->getSupport(getLevel()));
   right.getAllCubesRecur(phase ^ isNegEdge(), cube, allCubes);
}

// Assume this BddNodeV is a cube
string
BddNodeV::toString() const
{
   string str;
   toStringRecur(false, str);
   return str;
}

// return true if a cube is found
bool
BddNodeV::toStringRecur(bool phase, string& str) const
{
   if (isTerminal())
      return (!(phase ^ isNegEdge()));

   stringstream sstr;
   if (getLeft().toStringRecur(phase ^ isNegEdge(), str)) {
      sstr << "(" << getLevel() << ") ";
      str += sstr.str();
      return true;
   }
   else if (getRight().toStringRecur(phase ^ isNegEdge(), str)) {
      sstr << "!(" << getLevel() << ") ";
      str += sstr.str();
      return true;
   }
   return false;
}

ostream&
operator << (ostream& os, const BddNodeV& n)
{
   size_t nNodes = 0;
   n.print(os, 0, nNodes);
   n.unsetVisitedRecur();
   os << endl << endl << "==> Total #BddNodeVs : " << nNodes << endl;
   return os;
}

void
BddNodeV::print(ostream& os, size_t indent, size_t& nNodes) const
{
   for (size_t i = 0; i < indent; ++i)
      os << ' ';
   BddNodeVInt* n = getBddNodeVInt();
   os << '[' << getLevel() << "](" << (isNegEdge()? '-' : '+') << ") ";
   if (_debugBddAddr)
      os << n << " ";
   if (_debugRefCount)
      os << "(" << n->getRefCount() << ")";
   if (n->isVisited()) {
      os << " (*)";
      return;
   }
   else ++nNodes;
   n->setVisited();
   if (!isTerminal()) {
      os << endl;
      n->getLeft().print(os, indent + 2, nNodes);
      os << endl;
      n->getRight().print(os, indent + 2, nNodes);
   }
}

void
BddNodeV::unsetVisitedRecur() const
{
   BddNodeVInt* n = getBddNodeVInt();
   if (!n->isVisited()) return;
   n->unsetVisited();
   if (!isTerminal()) {
      n->getLeft().unsetVisitedRecur();
      n->getRight().unsetVisitedRecur();
   }
}

void
BddNodeV::drawBdd(const string& name, ofstream& ofile) const
{
   ofile << "digraph {" << endl;
   ofile << "   node [shape = plaintext];" << endl;
   ofile << "   ";
   for (unsigned l = getLevel(); l > 0; --l)
      ofile << l << " -> ";
   ofile << "0 [style = invis];" << endl;
   ofile << "   { rank = source; \"" << name << "\"; }" << endl;
   ofile << "   node [shape = ellipse];" << endl;
   ofile << "   \"" << name << "\" -> \"" << getLabel()
         << "\" [color = blue]";
   ofile << (isNegEdge()? " [arrowhead = odot]" : ";") << endl;

   drawBddRecur(ofile);

   ofile << "   { rank = same; 0; \"One\"; }" << endl;
   ofile << "}" << endl;

   unsetVisitedRecur( );
}

void
BddNodeV::drawBddRecur(ofstream& ofile) const
{
   BddNodeVInt* n = getBddNodeVInt();
   if (n->isVisited())
      return;
   n->setVisited();
   if(isTerminal())
      return;
   BddNodeV left = getLeft();
   BddNodeV right = getRight();

   ofile << "   { rank = same; " << getLevel() << "; \"" << getLabel()
         << "\"; }\n";

   ofile << "   \"" << getLabel() << "\" -> \"" << left.getLabel() << "\"";
   ofile << ((left.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   ofile << "   \"" << getLabel() << "\" -> \"" << right.getLabel()
         << "\"[style = dotted ] [color=red]";
   ofile << ((right.isNegEdge())? " [arrowhead=odot]" : ";") << endl;

   left.drawBddRecur(ofile);
   right.drawBddRecur(ofile);
}

// Don't put this->isNegEdge() on label
string
BddNodeV::getLabel() const
{
   if (isTerminal())
      return "One";

   ostringstream str;
   str << getBddNodeVInt();

   return str.str();
}

