/****************************************************************************
  FileName     [ proveBdd.cpp ]
  PackageName  [ prove ]
  Synopsis     [ For BDD-based verification ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "bddMgrV.h"
#include "gvMsg.h"
#include "gvNtk.h"
#include "util.h"
#include <iomanip>
#include <iostream>
#include <vector>

void
BddMgrV::buildPInitialState() {
    // TODO : remember to set _initState
    // set initial state to all zero
    BddNodeV newNode;
    for (unsigned i = 0; i < gvNtkMgr->getFFSize(); ++i) {
        if (i == 0) {
            newNode = ite(bddMgrV->getBddNodeV(gvNtkMgr->getFF(i).id),
                          BddNodeV::_zero, BddNodeV::_one);
        } else {
            newNode = ite(bddMgrV->getBddNodeV(gvNtkMgr->getFF(i).id),
                          BddNodeV::_zero, newNode);
        }
    }
    _initState = newNode;
    _reachStates.clear();
    _reachStates.push_back(_initState);
}

void
BddMgrV::buildPTransRelation() {
    // TODO : remember to set _tr, _tri

    BddNodeV delta, y;
    GVNetId  delta_net;

    // build _tri
    for (unsigned i = 0; i < gvNtkMgr->getFFSize(); ++i) {
        // next state (y)'s name
        string nsStr = gvNtkMgr->getNetNameFromId(
            gvNtkMgr->getInputNetId(gvNtkMgr->getFF(i), 0).id);
        // get BDD by name
        y     = bddMgrV->getBddNodeV(nsStr);
        delta = bddMgrV->getBddNodeV(
            gvNtkMgr->getInputNetId(gvNtkMgr->getFF(i), 0).id);
        delta_net = gvNtkMgr->getInputNetId(gvNtkMgr->getFF(i), 0);
        if (bddMgrV->getBddNodeV(delta_net.id) == (size_t)0) {
            gvNtkMgr->buildBdd(delta_net);
        }
        // build _tri
        if (i == 0) {
            _tri = ~(y ^ delta);
        } else {
            _tri = _tri & ~(y ^ delta);
        }
    }

    // build _tr
    _tr = _tri;
    for (unsigned i = 0; i < gvNtkMgr->getInputSize(); ++i) {
        _tr = _tr.exist(gvNtkMgr->getInput(i).id);
    }
    for (unsigned i = 0; i < gvNtkMgr->getInoutSize(); ++i) {
        _tr = _tr.exist(gvNtkMgr->getInout(i).id);
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

void
BddMgrV::buildPImage(int level) {
    // TODO : remember to add _reachStates and set _isFixed
    // note:: _reachStates record the set of reachable states
    BddNodeV cube, ns;
    bool     isMoved;
    _isFixed = false;
    for (unsigned i = 0; i < level; ++i) {
        if (_isFixed) {
            cout << "Fixed point is reached (time : " << _reachStates.size() - 1
                 << ")" << endl;
            break;
        }
        // build next state
        if (_reachStates.size() == 1) {
            ns = _tr & _initState;
        } else {
            ns = _tr& restrict(_reachStates[_reachStates.size() - 1],
                               (~_reachStates[_reachStates.size() - 2]));
        }
        // existential
        for (unsigned j = 0; j < gvNtkMgr->getFFSize(); ++j) {
            ns = ns.exist(gvNtkMgr->getFF(j).id);
        }
        ns = ns.nodeMove(gvNtkMgr->getFF(0).id + gvNtkMgr->getFFSize(),
                         gvNtkMgr->getFF(0).id, isMoved);
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

void
BddMgrV::runPCheckProperty(const string& name, BddNodeV monitor) {
    // TODO : prove the correctness of AG(~monitor)
    BddNodeV             result, cs, ns, test;
    bool                 isMoved;
    int                  numofstate = _reachStates.size() - 1;
    vector<vector<bool>> counter_ex;
    vector<bool>         timeframe;

    ns = monitor & _reachStates.back();
    if (ns != BddNodeV::_zero) {
        // ~p can backtrace to init state ?
        while ((monitor & _reachStates[numofstate]).countCube() != 0) {
            if(numofstate == 0) break;
            numofstate--;
        }
        if(numofstate != 0)
            numofstate++;
        //cout << "This is reachable state : \n" << _reachStates[numofstate];
        //cout << "This is monitor : \n" << monitor;
        ns = monitor & _reachStates[numofstate];

        //cout << "Monitor \"" << name << "\" is violated." << endl;
        //cout << "Counter Example:" << endl;

        ns = ns.getCube(0);
        counter_ex.clear();
        /* === MODIFICATION FOR PROPERTY 0 IN c.v === */
        if(numofstate == 0 ){
            BddNodeV firstState = ns;
            for (unsigned j = 0; j < gvNtkMgr->getFFSize(); ++j) {
                firstState = firstState.exist(gvNtkMgr->getFF(j).id);
            }
            for (unsigned j = 0; j < gvNtkMgr->getInputSize(); ++j) {
                if (firstState.getLeft() != BddNodeV::_zero){ 
                    if(firstState.isNegEdge()) timeframe.push_back(0);
                    else                       timeframe.push_back(1);
                }
                else if (firstState.getRight() != BddNodeV::_zero){
                    if(firstState.isNegEdge()) timeframe.push_back(1);
                    else                       timeframe.push_back(0);
                }
            }
            counter_ex.push_back(timeframe);
            //cout << firstState << endl;
            //cout << firstState.toString();
        }
        /* === END OF MODIFICATION === */
        timeframe.clear();

        for (unsigned i = 0; i < numofstate; ++i) {
            // find legal current state
            ns = ns.nodeMove(gvNtkMgr->getFF(0).id,
                             gvNtkMgr->getFF(0).id + gvNtkMgr->getFFSize(),
                             isMoved);
            ns = _tri & ns & _reachStates[numofstate - 1 - i];
            for (unsigned j = 0; j < gvNtkMgr->getFFSize(); ++j) {
                ns = ns.exist(gvNtkMgr->getFF(j).id + gvNtkMgr->getFFSize());
            }

            // find valid input value
            for (unsigned j = 0; j < gvNtkMgr->getInputSize(); ++j) {
                if (ns.getLeftCofactor(gvNtkMgr->getInput(j).id) !=
                    BddNodeV::_zero) {
                    ns = ns.getLeftCofactor(gvNtkMgr->getInput(j).id);
                    timeframe.push_back(1);
                } else if (ns.getRightCofactor(gvNtkMgr->getInput(j).id) !=
                           BddNodeV::_zero) {
                    ns = ns.getRightCofactor(gvNtkMgr->getInput(j).id);
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
            for (unsigned j = 0; j < gvNtkMgr->getInputSize(); ++j) {
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
