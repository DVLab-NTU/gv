#include <fstream>
#include <iomanip>
#include <cassert>
#include "gvBddNode.h"
#include "gvBddMgr.h"

typedef ABC_PTRINT_T ptrint;
extern "C"
{
    int Cudd_PrintMinterm( DdManager * manager, DdNode * node );
}

using namespace std;

BddMgr* bddMgr = 0;

void
BddMgr::init(unsigned nVars)
{
    DdMgr = Cudd_Init( nVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
    BddNode::_one = BddNode(Cudd_ReadOne(DdMgr));
    BddNode::_zero = BddNode(Cudd_ReadZero(DdMgr));
    BddNode::setBddMgr(this);
}

void
BddMgr::reset(unsigned nVars)
{
    Cudd_Quit(DdMgr);
    DdMgr = Cudd_Init(nVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
    BddNode::_one = BddNode(Cudd_ReadOne(DdMgr));
    BddNode::_zero = BddNode(Cudd_ReadZero(DdMgr));
    BddNode::setBddMgr(this);
    _supports.clear();
}

void
BddMgr::addNewVars(size_t n)
{
    for (int i = 0; i < n; i++) {
      DdNode* newVar = Cudd_bddNewVar(DdMgr);
      BddNode newNode(newVar);
      _supports.push_back(newNode);
    }
}

void 
BddMgr::addCube() 
{
    DdNode *cube, *var, *tmp, *tmp2;
    int numvars = 0, numfunctions = 0;
    DdNode* ddnodearray[3];
    int inputs[3], ordering[3], outputs[3];
    cube = Cudd_ReadOne(DdMgr);
    Cudd_Ref(cube);

    for (int i=0; i<numvars; i++) {
        switch(inputs[ordering[i]])
        {
        case '0':
            var = Cudd_Not(Cudd_bddIthVar(DdMgr,i)); 
        break;
        case '1':
            var = Cudd_bddIthVar(DdMgr, i);
        break;
        case '-':
            var = Cudd_ReadOne(DdMgr);
        break;
        }
        tmp = Cudd_bddAnd(DdMgr, cube, var);
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(DdMgr, cube);
        cube = tmp;
    }
    for (int j=0; j<numfunctions; j++) {
        if (outputs[j] == '1') {
            tmp2 = Cudd_bddOr(DdMgr, ddnodearray[j], cube); 
            Cudd_Ref(tmp2);
            Cudd_RecursiveDeref(DdMgr, ddnodearray[j]);
            Cudd_RecursiveDeref(DdMgr, cube);
            ddnodearray[j] = tmp2;
        }
    }
}

bool
BddMgr::addBddNode(const string& str, BddNode n)
{
    return _bddMap.insert(pair<string, BddNode>(str, n)).second;
}

bool
BddMgr::isBddNode(const string& name)
{
    BddMapConstIter bi = _bddMap.find(name);
    if (bi == _bddMap.end()) return false;
    return true;
}

BddNode
BddMgr::getBddNode(const string& name)
{
    BddMapConstIter bi = _bddMap.find(name);
    if (bi == _bddMap.end()) return size_t(0);
    return (*bi).second;
}

BddNode
BddMgr::findNodeByName(const string& name)
{
    char bddName[128];
    strcpy(bddName, name.c_str());
    Abc_Obj_t* pNode = Abc_NtkFindNode(pNtk, bddName);
    if (pNode == NULL) return size_t(0);
    DdNode* bFunc = (DdNode *)pNode->pData;
    BddNode bddNode(bFunc);
    return bddNode;
}

// void
// BddMgr::addBddIthNode(const string& nodeName, size_t n, int i)
// {
//     DdNode *newVar = Cudd_bddIthVar(DdMgr, i);
//     Cudd_Ref(newVar);
// }

// void
// BddMgr::addBddAtLevelNode(const string& nodeName, size_t n, int level)
// {
//     DdNode *newVar = Cudd_bddNewVarAtLevel(DdMgr, level);
//     Cudd_Ref(newVar);
// }

BddNode
BddMgr:: addNotNode(BddNode x1)
{
    BddNode bddNode(Cudd_Not(x1.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addAndNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddAnd(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addNandNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddNand(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addOrNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddOr(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}


BddNode
BddMgr:: addNorNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddNor(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addXorNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddXor(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addXnorNode(BddNode x1, BddNode x2)
{
    BddNode bddNode(Cudd_bddXnor(DdMgr, x1.getNode(), x2.getNode()));
    return bddNode;
}

BddNode
BddMgr:: addIteNode(BddNode f, BddNode g, BddNode h)
{
    BddNode bddNode(Cudd_bddIte(DdMgr, f.getNode(), g.getNode(), h.getNode()));
    return bddNode;
}

// void
// BddMgr:: reorderBySifting()
// {
//     Cudd_AutodynEnable(DdMgr, CUDD_REORDER_SYMM_SIFT);
//     Cudd_ReduceHeap(DdMgr, CUDD_REORDER_SYMM_SIFT, 3000);     
// }

// void
// BddMgr:: reorderBySwapping()
// {
//     Cudd_AutodynEnable(DdMgr, CUDD_REORDER_RANDOM);
//     Cudd_ReduceHeap(DdMgr, CUDD_REORDER_RANDOM, 3000);     
// }

void
BddMgr::printInfo() 
{
    cout << "BddMgr nodes: " << Cudd_ReadNodeCount(DdMgr) << endl;
    cout << "BddMgr vars: " << Cudd_ReadSize(DdMgr) << endl;
    cout << "BddMgr reorderings: " << Cudd_ReadReorderings(DdMgr) << endl;
    cout << "BddMgr memory: " << Cudd_ReadMemoryInUse(DdMgr) << endl;
}

void
BddMgr::printInfo(BddNode bddNode) 
{
    printf("DdNode nodes: %d | ", Cudd_DagSize(bddNode.getNode()));
	printf("DdNode vars: %d | ", Cudd_SupportSize(DdMgr, bddNode.getNode()));
}

void
BddMgr::report(BddNode bddNode) 
{
    DdNode* bFunc = bddNode.getNode();
    int nvars = Cudd_ReadSize(DdMgr);
    int* sorted = NULL;
    int retval, slots,  n = 1;
    long  refAddr, diff, mask;
    char ** onames = NULL;
    st__table    *visited = NULL;
    st__generator *gen = NULL;
    DdNode* support = NULL, *scan;
    DdNodePtr* nodelist;

    // initialize the function array
    DdNode** f = (DdNode**)malloc(sizeof(DdNode*));
    f[0] = bFunc;

    printInfo();
    cout << endl;

    sorted = ABC_ALLOC(int,nvars);
    for (int i = 0; i < nvars; i++) sorted[i] = 0;
    support = Cudd_VectorSupport(DdMgr,f,n);
    if (support == NULL) cerr << "Error" << endl;
    cuddRef(support);
    scan = support;
    while (!cuddIsConstant(scan)) {
        sorted[scan->index] = 1;
        scan = cuddT(scan);
    }
    Cudd_RecursiveDeref(DdMgr,support);
    support = NULL; 
    visited = st__init_table( st__ptrcmp, st__ptrhash);
    if (visited == NULL) cerr << "Error" << endl;

    for (int i = 0; i < n; i++) {
        retval = cuddCollectNodes(Cudd_Regular(f[i]),visited);
        if (retval == 0) cerr << "Error" << endl;
    }

    refAddr = (long) Cudd_Regular(f[0]);
    diff = 0;
    gen = st__init_gen(visited);
    if (gen == NULL) cerr << "Error" << endl;
    while ( st__gen(gen, (const char **)&scan, NULL)) {
        diff |= refAddr ^ (long) scan;
    }
    st__free_gen(gen); gen = NULL;

    for (int i = 0; (unsigned) i < 8 * sizeof(long); i += 4) {
        mask = (1 << i) - 1;
        if (diff <= mask) break;
    }

    nodelist = DdMgr->constants.nodelist;
    slots = DdMgr->constants.slots;

    for (int i = 0; i < n; i++) {
        if (onames == NULL) {
            retval = printf("\"F%d\"", i);
        } else {
            retval = printf("\"  %s  \"", onames[i]);
        }
        if (retval == EOF) cerr << "Error" << endl;

        if (Cudd_IsComplement(f[i])) {
            retval = printf(" -> \"%lx\" [style = dotted];\n", ((mask & (ptrint) f[i]) / sizeof(DdNode)));
        } else {
            retval = printf(" -> \"%lx\" [style = solid];\n", ((mask & (ptrint) f[i]) / sizeof(DdNode)));
        }
        if (retval == EOF) cerr << "Error" << endl;
    }

    for (int i = 0; i < nvars; i++) {
        if (sorted[DdMgr->invperm[i]]) {
            nodelist = DdMgr->subtables[i].nodelist;
            slots = DdMgr->subtables[i].slots;
            for (int j = 0; j < slots; j++) {
                scan = nodelist[j];
                while (scan != NULL) {
                    if ( st__is_member(visited,(char *) scan)) {
                        retval = printf("[%d] (+) \"%lx\" -> \"%lx\";\n", 
                            DdMgr->invperm[i],
                            ((mask & (ptrint) scan) / sizeof(DdNode)),
                            ((mask & (ptrint) cuddT(scan)) / sizeof(DdNode)));
                        if (retval == EOF) cerr << "Error" << endl;
                        if (Cudd_IsComplement(cuddE(scan))) {
                            retval = printf("[%d] (-) \"%lx\" -> \"%lx\" [style = dotted];\n", 
                                DdMgr->invperm[i],
                                ((mask & (ptrint) scan) / sizeof(DdNode)),
                                ((mask & (ptrint) cuddE(scan)) / sizeof(DdNode)));
                        } else {
                            retval = printf("[%d] (-) \"%lx\" -> \"%lx\" [style = dashed];\n", 
                                DdMgr->invperm[i],
                                ((mask & (ptrint) scan) / sizeof(DdNode)),
                                ((mask & (ptrint) cuddE(scan)) / sizeof(DdNode)));
                        }
                        if (retval == EOF) cerr << "Error" << endl;
                    }
                    scan = scan->next;
                }
            }
        }
    }

    nodelist = DdMgr->constants.nodelist;
    slots = DdMgr->constants.slots;
    for (int j = 0; j < slots; j++) {
        scan = nodelist[j];
        while (scan != NULL) {
            if ( st__is_member(visited,(char *) scan)) {
                retval = printf("\"%lx\" [label = \"%g\"];\n", 
                    ((mask & (ptrint) scan) / sizeof(DdNode)), cuddV(scan));
                if (retval == EOF) cerr << "Error" << endl;
            }
            scan = scan->next;
        }
    }

}

void
BddMgr::draw(string bddName, string fileName) 
{
    FILE * outfile = fopen(fileName.c_str(), "w");

    char _bddName[128];
    strcpy(_bddName, bddName.c_str());
    Abc_Obj_t * pNode = Abc_NtkFindNode(pNtk, _bddName);
    DdNode* bFunc = (DdNode *)pNode->pData;

    // dump the function to .dot file
    DdNode **nodeArray = (DdNode**)malloc(sizeof(DdNode*));
    nodeArray[0] = bFunc;
    Cudd_DumpDot(DdMgr, 1, nodeArray, NULL, NULL, outfile);
    free(nodeArray);
    fclose (outfile); 
}

void
BddMgr::construct()
{
    // Add vars from the input file
    DdNode *newVar;
    for(int i=0; i<Cudd_ReadSize(DdMgr); ++i) {
        newVar = Cudd_bddIthVar(DdMgr, i);
        _supports.push_back(newVar);
    }
    // cout << _supports.size() << endl;
}