/****************************************************************************
  FileName     [ GVNtk.h ]
  PackageName  [ GV/src/ntk ]
  Synopsis     [ GV Network. ]
  Author       [ Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2012-2014 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_NTK_H
#define GV_NTK_H

#include "gvType.h"
#include "cassert"
#include "gvAbcMgr.h"

// GV Ntk Defines
#define getGVInvertNet(netId)     (~netId)
#define getGVNetIndex(netId)      (netId.id)
#define isGVNetInverted(netId)    (netId.cp)
#define isGVBlackBoxed(module)    (!module->getRef())
#define isGVConstType(type)       (AIG_FALSE == type || BV_CONST == type)
#define isGVReducedType(type)     (BV_RED_AND <= type && BV_RED_XOR >= type)
#define isGVPairType(type)        (BV_AND <= type && BV_GEQ >= type)
#define isGVExtendType(type)      (BV_OR <= type && BV_LESS >= type)
#define isGVExtendSwapIn(type)    (BV_GREATER == type || BV_LEQ == type)
#define isGVExtendInvIn(type)     (BV_OR == type || BV_NOR == type)
#define isGVExtendInvOut(type)    \
   (BV_OR == type || BV_NAND == type || BV_XNOR == type || BV_NEQ == type || BV_GREATER == type || BV_LESS == type)
#define isGVExchangableType(type) \
   (AIG_NODE == type || BV_AND == type || BV_XOR == type || BV_ADD == type || BV_MULT == type || BV_EQUALITY == type)
#define getGVExtendNormal(type)   \
   ((BV_OR <= type && BV_NOR >= type) ? BV_AND : (BV_GREATER <= type && BV_LESS >= type) ? BV_GEQ : \
    (BV_XNOR == type) ? BV_XOR : (BV_NEQ == type) ? BV_EQUALITY : GV_GATE_TOTAL)

typedef GVVec<GVNetId   >::Vec   GVNetVec;
typedef GVVec<uint32_t  >::Vec   GVUI32Vec;
typedef GVVec<GVNetType    >::Vec   GVInputVec;


// Defines for Tables
typedef GVVec<GVInputVec   >::Vec   GVInputTable;
typedef GVVec<GVNetVec     >::Vec   GVNetTable;

// Forward Declarations
class GVNtkHandler;
class GVNtkModule;

// Constants
const GVNetId        GVNetUD = GVNetId::makeNetId();

// GVNtkModule : GV Ntk Reference (Ntk Instance)
class GVNtkModule
{
   public : 
      // Constructors for GV Module Instance
      GVNtkModule(const GVNetVec& i, const GVNetVec& o) { _ref = 0; _inputs = i; _outputs = o; }
      ~GVNtkModule();
      // Ntk Reference Functions
      inline GVNtkHandler* const getNtkRef() const { return (GVNtkHandler*)(_ref & ~1ul); }
      inline const bool isNtkRefBlackBoxed() const { return (_ref & 1ul); }
      inline void updateNtkRef(const GVNtkHandler* const h, const bool& b = false) {
         assert (h); _ref = (b) ? ((size_t)h | 1ul) : (size_t)(h); }
      // Ntk I/O Functions
      inline const GVNetVec& getInputList() const { return _inputs; }
      inline const GVNetVec& getOutputList() const { return _outputs; }
      inline void updateInput(const uint32_t& i, const GVNetId& id) { assert (i < _inputs.size()); _inputs[i] = id; }
      inline void updateOutput(const uint32_t& i, const GVNetId& id) { assert (i < _outputs.size()); _outputs[i] = id; }
   private : 
      size_t      _ref;
      GVNetVec    _inputs;
      GVNetVec    _outputs;
};

typedef GVVec<GVNtkModule* >::Vec   GVNtkModuleVec;

class GVNtk
{
    public :
        // Constructor for gvNtk
        GVNtk();
        virtual ~GVNtk();
        // Ntk Construction Functions
        virtual void initialize();
        virtual const GVNetId createNet(const uint32_t& = 1);
        void createModule(GVNtkModule* const);
        void createInput(const GVNetId&);
        void createOutput(const GVNetId&);
        void createInout(const GVNetId&);
        void createLatch(const GVNetId&);
        void createConst(const GVNetId&);
        void createClock(const GVNetId&);
        //const bool extendNetId(const GVNetId&);
        void setInput(const GVNetId&, const GVInputVec&);
        void createGate(const GVGateType&, const GVNetId&);
        // Ntk Reconstruction Functions
        // void replaceFanin(const GVRepIdHash&);
        void replaceOutput(const uint32_t&, const GVNetId&);
        // Ntk Destructive Functions  (NOT Recommended)
        inline void freeNetId(const GVNetId&);
        inline void removeLastInput() { _IOList[0].pop_back(); }
        inline void removeLastOutput() { _IOList[1].pop_back(); }
        inline void removeLastInout() { _IOList[2].pop_back(); }
        inline void removeLastLatch() { _FFList.pop_back(); }
        // Ntk Structure Functions
        virtual const uint32_t getNetWidth(const GVNetId&) const = 0;
        inline const GVGateType getGateType(const GVNetId&) const;
        inline const uint32_t getNetSize() const { return _inputData.size(); }
        // inline const uint32_t getModuleSize() const { return _ntkModule.size(); }
        inline const uint32_t getInputSize() const { return _IOList[0].size(); }
        inline const uint32_t getOutputSize() const { return _IOList[1].size(); }
        inline const uint32_t getInoutSize() const { return _IOList[2].size(); }
        inline const uint32_t getLatchSize() const { return _FFList.size(); }
        inline const uint32_t getConstSize() const { return _ConstList.size(); }
        inline const GVNetId& getInput(const uint32_t& i) const { assert (i < getInputSize()); return _IOList[0][i]; }
        inline const GVNetId& getOutput(const uint32_t& i) const { assert (i < getOutputSize()); return _IOList[1][i]; }
        inline const GVNetId& getInout(const uint32_t& i) const { assert (i < getInoutSize()); return _IOList[2][i]; }
        inline const GVNetId& getLatch(const uint32_t& i) const { assert (i < getLatchSize()); return _FFList[i]; }
        inline const GVNetId& getConst(const uint32_t& i) const { assert (i < getConstSize()); return _ConstList[i]; }
        inline const GVNetId& getClock() const { return _globalClk; }
        inline GVNtkModule* const getModule(const uint32_t&) const;
        inline GVNtkModule* const getModule(const GVNetId&) const;
        // Ntk Traversal Functions
        inline const uint32_t getInputNetSize(const GVNetId&) const;
        inline const GVNetId& getInputNetId(const GVNetId&, const uint32_t&) const;
        // Ntk Construction Validation Report Functions
      inline const bool validNetId(const GVNetId& id) const { return (id.id) < getNetSize(); }
      const bool reportInvertingNet(const GVNetId&) const;
      const bool reportMultipleDrivenNet(const GVGateType&, const GVNetId&) const;
      const bool reportUnexpectedFaninSize(const GVGateType&, const GVNetId&) const;
      const bool reportUnexpectedNetWidth(const GVNetId&, const uint32_t&, const string&) const;
      const bool reportNetWidthInconsistency(const GVNetId&, const GVNetId&, const string&) const;
    protected :
        // gvNetwork provate members
        GVNetVec    _IOList[3];     // gvNetId of PI / PO / PIO
        GVNetVec    _FFList;       // GVNetId of FF
        GVNetVec    _ConstList;    // GVNetId of BV_CONST (AIG_FALSE)
        GVInputTable   _inputData;    // Fanin Table for GVNetId   (GVNetId, GVBVXId, GVBusId)
        // GV Special Handling Members
        GVNetId        _globalClk;    // Global Clock Signal (Specified in RTL)
        GVNtkModuleVec _ntkModule;    // Module Instance for Hierarchical Ntk
        AbcMgr*        gvAbcMgr;
};

// GVBvNtk : GV BV Network
class GVBvNtk : public GVNtk
{
   public : 
      // Constructors for BV Network
      GVBvNtk();
      GVBvNtk(const GVBvNtk&);
      ~GVBvNtk();
      // Ntk Construction Functions
      void initialize();
      const GVNetId createNet(const uint32_t& = 1);
      // Ntk Structure Functions
      const uint32_t getNetWidth(const GVNetId&) const;
      // Ntk for BV Gate Functions
      const uint32_t getInputSliceBit(const GVNetId&, const bool&) const;
      // Ntk Reconstruction Functions
      void resetNetWidth(const GVNetId&, const uint32_t&);
   private : 
      // BV Ntk Extended Data Member
      GVUI32Vec            _netWidth;     // Width of Bit-Vector GVNetId
};



#endif

