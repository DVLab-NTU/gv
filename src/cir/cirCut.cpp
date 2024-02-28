// /****************************************************************************
//   FileName     [ cirCut.cpp ]
//   PackageName  [ cir ]
//   Synopsis     [ Define cir cut functions ]
//   Author       [ Chung-Yang (Ric) Huang ]
//   Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
// ****************************************************************************/

#ifndef CIR_CUT_CPP
#define CIR_CUT_CPP

#include "cirCut.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "util.h"

using namespace std;

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/

// enrumerate the k-feasible cuts at the node
vector<CirCut*>
CirCutMan::gateGetCuts(CirGate* root, int k) {
    if (root->isGlobalRef()) return gate2Cut[root];
    root->setToGlobalRef();
    vector<CirCut*> rootCuts;
    CirCut* rootCut;
    rootCut            = new CirCut;
    rootCut->cutSize   = 1;
    rootCut->leafNodes = {root};
    rootCuts.push_back(rootCut);

    if (root->getType() == AIG_GATE) {
        vector<CirCut*> leftCuts, rightCuts;
        leftCuts  = gateGetCuts(root->getIn0Gate(), k);
        rightCuts = gateGetCuts(root->getIn1Gate(), k);
        for (int i = 0; i < leftCuts.size(); ++i) {
            for (int j = 0; j < rightCuts.size(); ++j) {
                int cutSize = leftCuts[i]->cutSize + rightCuts[j]->cutSize;
                if (cutSize > k) continue;
                rootCut          = new CirCut;
                rootCut->cutSize = cutSize;

                // insert left/right nodes cut
                rootCut->leafNodes.insert(rootCut->leafNodes.end(), leftCuts[i]->leafNodes.begin(), leftCuts[i]->leafNodes.end());
                rootCut->leafNodes.insert(rootCut->leafNodes.end(), rightCuts[j]->leafNodes.begin(), rightCuts[j]->leafNodes.end());
                rootCuts.push_back(rootCut);
            }
        }
    }
    gate2Cut[root] = rootCuts;
    // for(const auto& cut : rootCuts) {
    //     cout << "root " << root->getName() << " cut size " << cut->leafNodes.size() << endl;
    //     for(const auto& gate : cut->leafNodes) {
    //         cout << gate->getName() << " ";
    //     }
    //     cout << endl;
    // }
    return rootCuts;
}
#endif
