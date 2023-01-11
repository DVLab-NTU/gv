#ifndef GV_BDD_MGR_H
#define GV_BDD_MGR_H

#include <map>
#include "gvBddNode.h"

using namespace std;

class BddNode;

typedef vector<BddNode>                        BddArr;
typedef map<string, BddNode>                   BddMap;
typedef map<string, BddNode>::const_iterator   BddMapConstIter;

extern BddMgr* bddMgr;

class BddMgr
{
    public:

        BddMgr(unsigned nVars = 0) { init(nVars); }
        ~BddMgr() { reset(); }

        void init(unsigned nVars = 0);
        void init(DdManager* dd, Abc_Ntk_t* ntk){
            DdMgr = dd;
            pNtk = ntk;
        }
        void reset(unsigned nVars = 0);

        BddNode getSupport(size_t i) const { return _supports[i]; }
        size_t getNumSupports() const { return _supports.size(); }

        void addCube();
        void addNewVars(size_t n);
        bool addBddNode(const string& nodeName, BddNode);
        bool isBddNode(const string& nodeName);
        BddNode getBddNode(const string& nodeName);
        BddNode findNodeByName(const string& nodeName);
        void addBddIthNode(const string& nodeName, size_t n, int i);
        void addBddAtLevelNode(const string& nodeName, size_t n, int level);

        DdNode* getOne() {return _one;}
        DdNode* getZero() {return _one;}


        BddNode addNotNode(BddNode x1);
        BddNode addAndNode(BddNode x1, BddNode x2);
        BddNode addOrNode(BddNode x1, BddNode x2);
        BddNode addNandNode(BddNode x1, BddNode x2);
        BddNode addNorNode(BddNode x1, BddNode x2);
        BddNode addXorNode(BddNode x1, BddNode x2);
        BddNode addXnorNode(BddNode x1, BddNode x2);
        BddNode addIteNode(BddNode f, BddNode g, BddNode h);

        void reorderBySifting();
        void reorderBySwapping();
        void report(BddNode);
        void draw(string, string);
        void construct();

        void printInfo();
        void printInfo(BddNode);
        DdManager*  getDdMgr() { return DdMgr; }
    
    private:
        vector<BddNode>  _supports;
        DdManager*       DdMgr;
        Abc_Ntk_t*       pNtk;

        BddArr           _bddArr;
        BddMap           _bddMap;
        DdNode*          _one;
        DdNode*          _zero;

};



#endif
