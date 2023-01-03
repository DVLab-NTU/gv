/****************************************************************************
  FileName     [ gvCone.h ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ Define RTL logic cone manager ]
  Author       [ ]
  Copyright    [ Copyleft(c) 2022-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_CONE_H
#define GV_CONE_H

#include <map>
#include "gvNtk.h"
#include "gvAbcNtk.h"
#include "base/abc/abc.h"
#include "misc/vec/vec.h"
#include "kernel/sigtools.h"

using namespace std;

extern "C" void Abc_NtkDfsReverseNodes_rec( Abc_Obj_t * pNode, Vec_Ptr_t * vNodes );

template <typename T>
vector<T> abcVtrPtr2vtr(Vec_Ptr_t *vp)
{
   vector<T> myVector;
   if (vp != NULL)
   {
      for (int i = 0; i < Vec_PtrSize(vp); ++i)
      {
         myVector.push_back(((T)Vec_PtrEntry(vp, i)));
      }
      delete vp;
   }
   return myVector;
};

// global variable
class GVConeComparitor
{
public:
   map<string, int> _mapOfPiPo;
};
extern GVConeComparitor gvConeComparitor;

Vec_Ptr_t * GV_NtkDfsNodesReverse( Abc_Ntk_t * pNtk, Abc_Obj_t ** ppNodes, int nNodes );

class GVAbcCone
{
public:
   GVAbcCone() {}
   GVAbcCone(Abc_Obj_t *rootNode) : _rootNode(rootNode)
   {
      Abc_Obj_t *pNode;
      int i;
      char *pName;

      _TFIlist = abcVtrPtr2vtr<Abc_Obj_t *>(Abc_NtkDfsNodes(rootNode->pNtk, &rootNode, 1));
      Abc_NtkForEachCi(_rootNode->pNtk, pNode, i)
      {
         pName = Abc_ObjName(pNode);
         if (Abc_NodeIsTravIdCurrent(pNode))
         {
            // do the marking thing
            if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end())
            {
               gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
            }
            _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)]);
         }
      }
      _TFOlist = abcVtrPtr2vtr<Abc_Obj_t *>(GV_NtkDfsNodesReverse(rootNode->pNtk, &rootNode, 1));
      Abc_NtkForEachPo(_rootNode->pNtk, pNode, i)
      {
         pName = Abc_ObjName(pNode);
         if (Abc_NodeIsTravIdCurrent(pNode))
         {
            // do the marking thing
            if (gvConeComparitor._mapOfPiPo.find(pName) == gvConeComparitor._mapOfPiPo.end())
            {
               gvConeComparitor._mapOfPiPo[pName] = gvConeComparitor._mapOfPiPo.size();
            }
            _structuralHash += pow(2, gvConeComparitor._mapOfPiPo[Abc_ObjName(pNode)]);
         }
      }

      cout << Abc_ObjName(_rootNode) << " hash value: " << _structuralHash << endl;
   }
   ~GVAbcCone() {}

   int runTwoStepSim();

   int runFirstStepSim(int pattern);
   int runSecondStepSim(int pattern);

   int genHash();

private:
   Abc_Obj_t *_rootNode;
   vector<Abc_Obj_t *> _TFIlist;
   vector<Abc_Obj_t *> _TFOlist;
   int _structuralHash = 0;
};

class GVAbcConeMgr
{
public:
   GVAbcConeMgr() {}
   GVAbcConeMgr(Abc_Ntk_t *d) : _design(d)
   {
      genConelist();
   }
   ~GVAbcConeMgr()
   {
      for (auto cone : _coneList)
      {
         delete cone.second;
      }
   }

   GVAbcCone *getCone(Abc_Obj_t *k) const { return _coneList.find(k)->second; }

   void genConelist()
   {
      Abc_Obj_t *pNode;
      int i;
      Abc_NtkForEachNode1(_design, pNode, i)
      {
         _coneList[pNode] = new GVAbcCone(pNode);
      }
   }

private:
   Abc_Ntk_t *_design;
   map<Abc_Obj_t *, GVAbcCone *> _coneList;
};

class GVRTLCone
{
public:
   GVRTLCone() {}
   ~GVRTLCone() {}

   void runTfiDfs();
   void runTfoDfs();

   int runTwoStepSim();

   int runFirstStepSim(int pattern);
   int runSecondStepSim(int pattern);

   void setTfiList(RTLIL::Wire *s)
   {
      _TFIlist.push_back(s);
      // keep the vector item in sorted order
      // int temp = _TFIlist.size()-1;
      // for (int i = _TFIlist.size()-2; i>=0 ; --i){
      //    RTLIL::Wire* w = _TFIlist[i];
      //    _TFIlist[i] = _TFIlist[temp];
      //    _TFIlist[temp] = w;
      //    temp = i;
      // }
   }

   void setTfoList(RTLIL::Wire *s)
   {
      _TFOlist.push_back(s);
   }

private:
   RTLIL::Wire *_rootNode;
   vector<RTLIL::Wire *> _TFIlist;
   vector<RTLIL::Wire *> _TFOlist;
   int _structuralHash = 0;
};

class GVRTLConeMgr
{
public:
   GVRTLConeMgr() {}
   GVRTLConeMgr(GVRTLDesign *d) : _design(d)
   {
   }
   ~GVRTLConeMgr() {}

   GVRTLCone *getCone(size_t k) const { return _coneList.find(k)->second; }

   void genConelist();

private:
   GVRTLDesign *_design;
   map<size_t, GVRTLCone *> _coneList;
};

#endif // GV_CONE_h