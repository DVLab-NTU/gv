#ifndef GV_BDD_NODE_H
#define GV_BDD_NODE_H

#include <iostream>
#include <vector>
#include <map>
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "bdd/cudd/cudd.h"
#include "bdd/cudd/cuddInt.h"



using namespace std;

class BddMgr;

class BddNode {
    public:
        static BddNode            _one;
        static BddNode            _zero;

        BddNode() {}
        BddNode(DdNode* node) { _node = node; }
        ~BddNode() {}

        void init();

    //     const BddNode& getLeft() const;
    //     const BddNode& getRight() const;
    //     BddNode getLeftCofactor(unsigned i) const;
    //     BddNode getRightCofactor(unsigned i) const;
    //     unsigned getLevel() const;
    //     unsigned getRefCount() const;
    //     bool isNegEdge() const {}
    //     bool isPosEdge() const { return !isNegEdge(); }

        DdNode* getNode() const { return _node; }

        DdNode*  operator () () const { return _node; }
        BddNode  operator ~ () const;
        BddNode& operator = (const BddNode& n);
        BddNode  operator & (const BddNode& n) const;
        BddNode& operator &= (const BddNode& n);
        BddNode  operator | (const BddNode& n) const;
        BddNode& operator |= (const BddNode& n);
        BddNode  operator ^ (const BddNode& n) const;
        BddNode& operator ^= (const BddNode& n);

        static void setBddMgr(BddMgr* m) { _BddMgr = m; }



    private:
        DdNode*                   _node;
        size_t                    index;
        static BddMgr*            _BddMgr;
};

#endif