/****************************************************************************
  FileName     [ proveBdd.cpp ]
  PackageName  [ prove ]
  Synopsis     [ For BDD-based verification ]
  Author       [ ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "bddMgrV.h"
#include "gvMsg.h"
// #include "gvNtk.h"
#include <iomanip>
#include <iostream>
#include <vector>

#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

void BddMgrV::buildPInitialState() {
    // TODO : remember to set _initState
    // set initial state to all zero
    BddNodeV newNode;
    for (unsigned i = 0; i < cirMgr->getNumLATCHs(); ++i) {
        if (i == 0) {
            newNode = ite(bddMgrV->getBddNodeV(cirMgr->getRo(i)->getGid()),
                          BddNodeV::_zero, BddNodeV::_one);
        } else {
            newNode = ite(bddMgrV->getBddNodeV(cirMgr->getRo(i)->getGid()),
                          BddNodeV::_zero, newNode);
        }
    }
    _initState = newNode;
    _reachStates.clear();
    _reachStates.push_back(_initState);
}

void BddMgrV::buildPTransRelation() {
    // TODO : remember to set _tr, _tri

    BddNodeV delta, y;

    // build _tri
    for (unsigned i = 0; i < cirMgr->getNumLATCHs(); ++i) {
        // next state (y)'s name
        string nsStr = cirMgr->getRi(i)->getName();
        // get BDD by name
        y     = bddMgrV->getBddNodeV(nsStr);
        delta = bddMgrV->getBddNodeV(cirMgr->getRi(i)->getGid());
        // build _tri
        if (i == 0) {
            _tri = ~(y ^ delta);
        } else {
            _tri = _tri & ~(y ^ delta);
        }
    }

    // build _tr
    _tr = _tri;
    for (unsigned i = 0; i < cirMgr->getNumPIs(); ++i) {
        _tr = _tr.exist(cirMgr->getPi(i)->getGid());
    }
}

BddNodeV BddMgrV::restrict(const BddNodeV& f, const BddNodeV& g) {
    if (g == BddNodeV::_zero) {
        cerr << "Error in restrict!!" << endl;
    }
    if (g == BddNodeV::_one) {
        return f;
    }
    if (f == BddNodeV::_zero || f == BddNodeV::_one) {
        return f;
    }
    unsigned a = g.getLevel();
    if (g.getLeftCofactor(a) == BddNodeV::_zero) {
        return restrict(f.getRightCofactor(a), g.getRightCofactor(a));
    }
    if (g.getRightCofactor(a) == BddNodeV::_zero) {
        return restrict(f.getLeftCofactor(a), g.getLeftCofactor(a));
    }
    if (f.getLeftCofactor(a) == f.getRightCofactor(a)) {
        return restrict(f, g.getLeftCofactor(a) | g.getRightCofactor(a));
    }
    BddNodeV newNode =
        (~getSupport(a)& restrict(f.getRightCofactor(a),
                                  g.getRightCofactor(a))) |
        (getSupport(a)& restrict(f.getLeftCofactor(a), g.getLeftCofactor(a)));
    return newNode;
}

void BddMgrV::buildPImage(int level) {
    // TODO : remember to add _reachStates and set _isFixed
    // note:: _reachStates record the set of reachable states
    BddNodeV cube, ns;
    bool isMoved;
    _isFixed = false;
    for (unsigned i = 0; i < level; ++i) {
        if (_isFixed) {
            cout << "Fixed point is reached (time : " << _reachStates.size() - 1 << ")" << endl;
            break;
        }
        // build next state
        if (_reachStates.size() == 1) {
            ns = _tr & _initState;
        } else {
            ns = _tr& restrict(_reachStates[_reachStates.size() - 1], (~_reachStates[_reachStates.size() - 2]));
        }
        // existential
        for (unsigned j = 0; j < cirMgr->getNumLATCHs(); ++j) {
            ns = ns.exist(cirMgr->getRo(j)->getGid());
        }
        int from = cirMgr->getRo(0)->getGid() + cirMgr->getNumLATCHs();
        int to   = cirMgr->getRo(0)->getGid();
        ns       = ns.nodeMove(cirMgr->getRo(0)->getGid() + cirMgr->getNumLATCHs(),
                               cirMgr->getRo(0)->getGid(), isMoved);
        // isFixed ?
        if (_reachStates.size() == 0) {
            ns = ns | _initState;
            if (ns == _initState) {
                _isFixed = true;
                cout << "Fixed point is reached (time : "
                     << _reachStates.size() - 1 << ")" << endl;
                break;
            }
        } else {
            ns = ns | _reachStates.back();
            if (ns == _reachStates.back()) {
                _isFixed = true;
                cout << "Fixed point is reached (time : "
                     << _reachStates.size() - 1 << ")" << endl;
                break;
            }
        }
        _reachStates.push_back(ns);
    }
}

void BddMgrV::runPCheckProperty(const string& name, BddNodeV monitor) {
    // TODO : prove the correctness of AG(~monitor)
    BddNodeV result, cs, ns, test;
    bool isMoved;
    int numofstate = _reachStates.size() - 1;
    vector<vector<bool>> counter_ex;
    vector<bool> timeframe;

    ns = monitor & _reachStates.back();
    if (ns != BddNodeV::_zero) {
        // ~p can backtrace to init state ?
        while ((monitor & _reachStates[numofstate]).countCube() != 0) {
            if (numofstate == 0) break;
            numofstate--;
        }
        if (numofstate != 0)
            numofstate++;
        ns = monitor & _reachStates[numofstate];
        ns = ns.getCube(0);
        counter_ex.clear();
        /* === MODIFICATION FOR PROPERTY 0 IN c.v === */
        if (numofstate == 0) {
            BddNodeV firstState = ns;
            for (unsigned j = 0; j < cirMgr->getNumLATCHs(); ++j) {
                firstState = firstState.exist(cirMgr->getRo(j)->getGid());
            }
            for (unsigned j = 0; j < cirMgr->getNumPIs(); ++j) {
                if (firstState.getLeft() != BddNodeV::_zero) {
                    if (firstState.isNegEdge()) timeframe.push_back(0);
                    else timeframe.push_back(1);
                } else if (firstState.getRight() != BddNodeV::_zero) {
                    if (firstState.isNegEdge()) timeframe.push_back(1);
                    else timeframe.push_back(0);
                }
            }
            counter_ex.push_back(timeframe);
        }
        /* === END OF MODIFICATION === */
        timeframe.clear();

        for (unsigned i = 0; i < numofstate; ++i) {
            // find legal current state
            ns = ns.nodeMove(cirMgr->getRo(0)->getGid(),
                             cirMgr->getRo(0)->getGid() + cirMgr->getNumLATCHs(),
                             isMoved);
            ns = _tri & ns & _reachStates[numofstate - 1 - i];
            for (unsigned j = 0; j < cirMgr->getNumLATCHs(); ++j) {
                ns = ns.exist(cirMgr->getRo(j)->getGid() + cirMgr->getNumLATCHs());
            }

            // find valid input value
            for (unsigned j = 0; j < cirMgr->getNumPIs(); ++j) {
                if (ns.getLeftCofactor(cirMgr->getPi(j)->getGid()) !=
                    BddNodeV::_zero) {
                    ns = ns.getLeftCofactor(cirMgr->getPi(j)->getGid());
                    timeframe.push_back(1);
                } else if (ns.getRightCofactor(cirMgr->getPi(j)->getGid()) !=
                           BddNodeV::_zero) {
                    ns = ns.getRightCofactor(cirMgr->getPi(j)->getGid());
                    timeframe.push_back(0);
                } else {
                    cerr << "error in monitor" << endl;
                }
            }
            counter_ex.push_back(timeframe);
            timeframe.clear();
        }
        for (unsigned i = 0; i < counter_ex.size(); ++i) {
            cout << i << ": ";
            for (unsigned j = 0; j < cirMgr->getNumPIs(); ++j) {
                cout << counter_ex[counter_ex.size() - 1 - i][j];
            }
            cout << endl;
        }
    } else {
        if (_isFixed) {
            cout << "Monitor \"" << name << "\" is safe." << endl;
        } else {
            cout << "Monitor \"" << name << "\" is safe up to time "
                 << _reachStates.size() - 1 << "." << endl;
        }
    }
}

BddNodeV
BddMgrV::find_ns(BddNodeV cs) {}

BddNodeV
BddMgrV::ns_to_cs(BddNodeV ns) {}
