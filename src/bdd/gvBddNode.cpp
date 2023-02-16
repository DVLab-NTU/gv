#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "gvBddNode.h"
#include "gvBddMgr.h"

BddMgr* BddNode::_BddMgr = 0;
BddNode BddNode::_one;
BddNode BddNode::_zero;

void
BddNode::init()
{
   _node = Cudd_bddNewVar(_BddMgr->getDdMgr());
}

BddNode
BddNode::operator ~ () const
{
   return _BddMgr->addNotNode((*this));
}

BddNode&
BddNode::operator = (const BddNode& n)
{
   _node = n._node;
   return (*this);
}

BddNode
BddNode::operator & (const BddNode& n) const
{
   return _BddMgr->addAndNode((*this), n);
}

BddNode&
BddNode::operator &= (const BddNode& n)
{
   (*this) = (*this) & n;
   return (*this);
}

BddNode
BddNode::operator | (const BddNode& n) const
{
   return _BddMgr->addOrNode((*this), n);
}

BddNode&
BddNode::operator |= (const BddNode& n)
{
   (*this) = (*this) | n;
   return (*this);
}

BddNode
BddNode::operator ^ (const BddNode& n) const
{
   return _BddMgr->addXorNode((*this), n);
}

BddNode&
BddNode::operator ^= (const BddNode& n)
{
   (*this) = (*this) ^ n;
   return (*this);
}