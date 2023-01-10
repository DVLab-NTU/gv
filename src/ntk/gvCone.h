/****************************************************************************
  FileName     [ gvCone.h ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ Define RTL logic cone manager ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2022-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_CONE_H
#define GV_CONE_H

#include "base/abc/abc.h"
#include "gvAbcNtk.h"
#include "gvNtk.h"
#include "kernel/sigtools.h"
#include "misc/vec/vec.h"
#include "opt/sim/sim.h"
#include <algorithm>
#include <iomanip>
#include <map>

using namespace std;

extern "C"
{
    void Abc_NtkDfsReverseNodes_rec(Abc_Obj_t* pNode, Vec_Ptr_t* vNodes);
}

template <typename T>
vector<T>
abcVtrPtr2vtr(Vec_Ptr_t* vp) {
    vector<T> myVtr;
    if (vp != NULL) {
        for (int i = 0; i < Vec_PtrSize(vp); ++i) {
            myVtr.push_back(((T)Vec_PtrEntry(vp, i)));
        }
        delete vp;
    }
    return myVtr;
};

template <typename T> // T can be const char* or string
string
rmSpaceBackslash(T str) {
    string new_str;
    int    i = 0;
    while (str[i]) {
        if (str[i] != ' ' && str[i] != '\\') new_str += str[i];
        i++;
    }
    return new_str;
};

// global variable
class GVConeComparitor
{
    public:
        map<string, int> _mapOfPiPo;
        size_t           getPiPoNum() { return _mapOfPiPo.size(); }
};
extern GVConeComparitor gvConeComparitor;

Vec_Ptr_t* GV_NtkDfsNodesReverse(Abc_Ntk_t* pNtk, Abc_Obj_t** ppNodes, int nNodes);

class GVCone
{
    public:
        virtual int   genHash() = 0;
        unsigned      getHash() { return _structuralHash; }
        virtual int   runTwoStepSim() { cout << "Not yet developped" << endl; }
        virtual int   runFirstStepSim(int pattern) { cout << "Not yet developped" << endl; }
        virtual int   runSecondStepSim(int pattern) { cout << "Not yet developped" << endl; }
        unsigned      getSimValue() { return _simPoValue; }
        void          setSimValue(unsigned v) { _simPoValue = v; }
        virtual void* getRootNode() = 0;

    protected:
        unsigned _structuralHash = 0;
        unsigned _simPoValue     = 0;
};

class GVAbcCone : public GVCone
{
    public:
        GVAbcCone() {}
        GVAbcCone(Abc_Obj_t* rootNode) : _rootNode(rootNode) {
            Abc_Obj_t* pNode;
            int        i;
            char*      pName;

            _TFIlist = abcVtrPtr2vtr<Abc_Obj_t*>(Abc_NtkDfsNodes(rootNode->pNtk, &rootNode, 1));
            Abc_NtkForEachPi(_rootNode->pNtk, pNode, i) {
                pName = Abc_ObjName(pNode);
                if (Abc_NodeIsTravIdCurrent(pNode)) {
                    // do the marking thing
                    if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
                        gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
                    }
                    _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)]);
                }
            }
            _TFOlist = abcVtrPtr2vtr<Abc_Obj_t*>(GV_NtkDfsNodesReverse(rootNode->pNtk, &rootNode, 1));
            Abc_NtkForEachPo(_rootNode->pNtk, pNode, i) {
                pName = Abc_ObjName(pNode);
                if (Abc_NodeIsTravIdCurrent(pNode)) {
                    // do the marking thing
                    if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end()) {
                        gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
                    }
                    _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)]);
                }
            }
            cout << Abc_ObjName(_rootNode) << " hash value: " << bitset<8>(_structuralHash) << endl;
        }
        ~GVAbcCone() {}

        // int runTwoStepSim() override;

        // int runFirstStepSim(int pattern) override;
        // int runSecondStepSim(int pattern) override;

        int genHash() override;

        bool checkInTFO(Abc_Obj_t* k) { return (find(_TFOlist.begin(), _TFOlist.end(), k) != _TFOlist.end()) || (k == _rootNode); }
        bool checkInTFI(Abc_Obj_t* k) { return (find(_TFIlist.begin(), _TFIlist.end(), k) != _TFIlist.end()) || (k == _rootNode); }

        vector<Abc_Obj_t*>& getTfo() { return _TFOlist; }

        void* getRootNode() override { return _rootNode; }

    private:
        Abc_Obj_t*         _rootNode;
        vector<Abc_Obj_t*> _TFIlist;
        vector<Abc_Obj_t*> _TFOlist;
};

class GVAbcConeMgr
{
    public:
        GVAbcConeMgr() {}
        GVAbcConeMgr(Abc_Ntk_t* d) : _design(d) { genConelist(); }
        ~GVAbcConeMgr() {
            for (auto cone : _coneList) {
                delete cone.second;
            }
            Sim_ManStop(_simMgr);
        }

        GVAbcCone* getCone(Abc_Obj_t* k) const { return _coneList.find(k)->second; }

        void genConelist() {
            Abc_Obj_t* pNode;
            int        i;
            runFirstStepSim();
            Abc_NtkForEachNode1(_design, pNode, i) {
                _coneList[pNode] = new GVAbcCone(pNode);
                runSecondStepSim(_coneList[pNode]);
            }
        }

        void runTwoStepSim() {
            runFirstStepSim();
            runSecondStepSim();
        }

        map<Abc_Obj_t*, GVAbcCone*>& getConeList() { return _coneList; }

    private:
        void                        runFirstStepSim();
        void                        runSecondStepSim();
        void                        runSecondStepSim(GVAbcCone*);
        Abc_Ntk_t*                  _design;
        map<Abc_Obj_t*, GVAbcCone*> _coneList;
        Sim_Man_t*                  _simMgr;
};

class GVRTLCone : public GVCone
{
    public:
        GVRTLCone() {}
        GVRTLCone(RTLIL::SigBit rootNode) : _rootNode(rootNode) {}
        ~GVRTLCone() {}

        // int runTwoStepSim();

        // int runFirstStepSim(int pattern);
        // int runSecondStepSim(int pattern);

        void setTfiList(string s) {
            _TFIlist.insert(s);
            // keep the vector item in sorted order
            // int temp = _TFIlist.size()-1;
            // for (int i = _TFIlist.size()-2; i>=0 ; --i){
            //    RTLIL::Wire* w = _TFIlist[i];
            //    _TFIlist[i] = _TFIlist[temp];
            //    _TFIlist[temp] = w;
            //    temp = i;
            // }
        }

        void setTfoList(string s) { _TFOlist.insert(s); }

        int   genHash() override;
        void* getRootNode() override { return &_rootNode; }

    private:
        RTLIL::SigBit _rootNode;
        pool<string>  _TFIlist; // wire id
        pool<string>  _TFOlist;
};

class GVRTLConeMgr
{
    public:
        GVRTLConeMgr() {}
        GVRTLConeMgr(GVRTLDesign* d) : _design(d) {}
        ~GVRTLConeMgr() {
            for (auto cone : _coneList) {
                delete cone.second;
            }
        }

        GVRTLCone* getCone(RTLIL::SigBit k) const { return _coneList.find(k)->second; }

        void genConelist(); // with DFS and BFS inside

        void genAllHash() {
            for (auto node : _coneList) {
                cout << log_signal(node.first) << " hash value: " << bitset<8>(node.second->genHash()).to_string() << endl;
            }
        }

        void runFirstStepSim(int pattern) {}

        map<RTLIL::SigBit, GVRTLCone*>& getConeList() { return _coneList; }

    private:
        GVRTLDesign*                   _design;
        map<RTLIL::SigBit, GVRTLCone*> _coneList;
};

void structCandidate(GVRTLConeMgr*, GVAbcConeMgr*);

#endif // GV_CONE_h