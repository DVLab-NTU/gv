/****************************************************************************
  FileName     [ gvNtk.h ]
  PackageName  [ gv/src/ntk ]
  Synopsis     [ GV Network ]
  Author       [ ]
  Copyright    [ ]
>>>>>>> e49dc0e73d18f38a26c685cf95dad078bf400c08
****************************************************************************/

#ifndef GV_NTK_H
#define GV_NTK_H
#include "gvAbcMgr.h"
#include "kernel/sigtools.h" // Sigmap
#include "kernel/utils.h"    // Toposort
#include "kernel/yosys.h"
#include <string>
#include <vector>

USING_YOSYS_NAMESPACE

// GV Ntk defines
#define isGVNetInverted(netId) (netId.cp)
#define getGVNetIndex(netId) (netId.id)

// constant
const unsigned GVNtkUD = UINT_MAX;
// declaration
class GVNtkMgr;
extern GVNtkMgr* gvNtkMgr;

// object types
typedef enum
{
    GV_NTK_OBJ_NONE,   // 0: non-existent object
    GV_NTK_OBJ_CONST0, // 1: constant 0
    GV_NTK_OBJ_CONST1, // 2: constant 1
    GV_NTK_OBJ_PI,     // 3: primary input
    GV_NTK_OBJ_PO,     // 4: primary output
    GV_NTK_OBJ_BUF,    // 5: buffer node
    GV_NTK_OBJ_AND,    // 6: AND node
    GV_NTK_OBJ_RO,     // 7: Register Output
    GV_NTK_OBJ_FF_CS,  // 8: Flip Flop Current State
    GV_NTK_OBJ_FF_NS,  // 9: Flip Flop Next State
    GV_NTK_OBJ_NOT,    // 10: NOT node
    GV_NTK_OBJ_LAST,   // 11: last element of the type
    GV_NTK_OBJ_AIG     // 12: AIG node
} GV_Ntk_Type_t;

typedef enum
{
    GV_NTK_TYPE_V,       // 0: verilog
    GV_NTK_TYPE_AIG,     // 1: aig
    GV_NTK_TYPE_BLIF,    // 2: blif
    GV_NTK_TYPE_BTOR     // 3: btor
} GV_Ntk_Format_t;

/*
                            _ _ _ _ _ _ _ _ _ _ _ _ _ _
                           |                           |
    (PI)        ---------> |                           | --------->        (PO)
    (PPI)   .------------> |   Combinational Network   | ------------->    (PPO)
            |  .---------> |                           | ---------->  |
            |  |           |_ _ _ _ _ _ _ _ _ _ _ _ _ _|           |  |
            |  |                                                   |  |
            |  |                _ _ _ _ _ _ _ _ _ _                |  |
            |  <-------------- |                   | <-------------.  |
            |                  |   Flip Flop (FF)  |                  |
    (RO=Q)  <----------------- |_ _ _ _ _ _ _ _ _ _| <----------------. (RI=D)
            (X: current state)                        (Y: next state)

*/

//----------------------------------------------------------------------
// GV Ntk Defines
//----------------------------------------------------------------------
struct GVNetId {
        unsigned       cp : 1;
        unsigned       id : 31;
        bool           fanin0Cp; // fanin 0 is complement
        bool           fanin1Cp; // fanin 1 is complement
        GV_Ntk_Type_t  type : GV_NTK_OBJ_AIG;
        static GVNetId makeNetId(unsigned i = GVNtkUD, unsigned c = 0, GV_Ntk_Type_t t = GV_NTK_OBJ_AIG,
                                 bool f0cp = false, bool f1cp = false) {
            GVNetId j;
            j.cp       = c;
            j.id       = i;
            j.type     = t;
            j.fanin0Cp = f0cp;
            j.fanin1Cp = f1cp;
            return j;
        }
        GVNetId    operator~() const { return makeNetId(id, cp ^ 1); }
        const bool operator==(const GVNetId& i) const { return cp == i.cp && id == i.id; }
        const bool operator!=(const GVNetId& i) const { return !(*this == i); }
};

// fanout info
struct GVFanout {
    unsigned id;       // the id of the fanout obj
    unsigned fanin;    // record which fanin is the obj
};

class GVNtkMgr
{
    public:
        // -------------------------------------------------------------------------
        //                               Information
        // -------------------------------------------------------------------------
        // Constructors for BV Network
        GVNtkMgr() { reset(); };
        ~GVNtkMgr() { reset(); }
        // get size
        inline const uint32_t getNetSize() const { return _id2GVNetId.size(); }
        inline const unsigned getInputSize() const { return _InputList.size(); }
        inline const unsigned getOutputSize() const { return _OutputList.size(); }
        inline const unsigned getInoutSize() const { return _InoutList.size(); }
        inline const uint32_t getFFSize() const { return _FFList.size(); }
        inline const uint32_t getConstSize() const { return _ConstList.size(); }
        inline const uint32_t getFFConst0Size() const { return _FFConst0List.size(); }
        // access function
        inline const GVNetId& getInput(const unsigned& i) const {
            assert(i < getInputSize());
            return _InputList[i];
        }
        inline const GVNetId& getOutput(const unsigned& i) const {
            assert(i < getOutputSize());
            return _OutputList[i];
        }
        inline const GVNetId& getInout(const unsigned& i) const {
            assert(i < getInoutSize());
            return _InoutList[i];
        }
        inline const GVNetId& getFF(const unsigned& i) const {
            assert(i < getFFSize());
            return _FFList[i];
        }
        inline const GVNetId& getConst(const unsigned& i) const {
            assert(i < getConstSize());
            return _ConstList[i];
        }
        inline const GVNetId& getFFConst0(const unsigned& i) const {
            assert(i < getFFConst0Size());
            return _FFConst0List[i];
        }
        // GV net id
        inline const GVNetId&          getGVNetId(const unsigned& i) const { return _id2GVNetId.at(i); }
        // GV gate type
        inline const GV_Ntk_Type_t     getGateType(const GVNetId& id) { return id.type; }
        inline GV_Ntk_Type_t&          getTypeFromId(const unsigned& i) { return _id2Type[i]; }
        // fanin
        inline const vector<unsigned>& getFaninId(const unsigned& i) const { return _id2FaninId.at(i); }
        inline const bool              hasFanout(const unsigned& i)  const { return _id2Fanout.count(i); }
        inline const vector<GVFanout>& getFanout(const unsigned& i)  const { return _id2Fanout.at(i); }
        inline const GVNetId&          getInputNetId(const GVNetId&, const uint32_t&) const;
        // flag
        inline void                    newMiscData() {
            if (getNetSize() > _miscList.size()) {
                _miscList.resize(getNetSize());
            }
            ++_globalMisc;
        }
        inline bool     isLatestMiscData(const GVNetId& id) const { return _globalMisc == _miscList[id.id]; }
        inline void     setLatestMiscData(const GVNetId& id) { _miscList[id.id] = _globalMisc; }
        // mapping (get id)
        inline unsigned getNetIdFromName(string name) { return _netName2Id[name]; }
        inline unsigned getPpiIdFromRoId(unsigned id) { return _idRo2Ppi[id]; }
        inline unsigned getRiIdFromRoId(unsigned id) { return _idRo2Ri[id]; }
        inline unsigned getRoIdFromRiId(unsigned id) { return _idRi2Ro[id]; }
        // mapping (get name)
        inline string   getNetNameFromId(unsigned id) { return _netId2Name[id]; }

        // -------------------------------------------------------------------------
        //                                Network
        // -------------------------------------------------------------------------
        // construct ntk
        void       createNet(const GVNetId& id, const int net_type);
        void       createNetFromAbc(char*);
        void       parseAigMapping(Gia_Man_t* pGia);
        void       setFileType(unsigned type) { _fileType = type; };
        unsigned   getFileType()              { return _fileType; };
        // print ntk
        void       print_rec(Gia_Man_t* pGia, Gia_Obj_t* pObj);
        // print functions
        void       printPi();      // print the information of all PI's
        void       printPo();      // print the information of all PO's
        void       printRi();      // print the information of all RI's
        void       printSummary(); // print the information of all Obj in the aig ntk
        // generate net
        GVNetId    createNet();
        bool       createGVAndGate(GVNetId&, GVNetId, GVNetId);
        // -------------------------------------------------------------------------
        //                                  BDD
        // -------------------------------------------------------------------------
        // build the BDD
        const bool setBddOrder(const bool&);
        void       buildNtkBdd();
        void       buildBdd(const GVNetId& netId);
        // DFS tranversal
        void       dfsOrder(const GVNetId&, vector<GVNetId>&);

    protected:
        // info
        vector<GVNetId>                 _InputList;    // GVNetId of PI's
        vector<GVNetId>                 _OutputList;   // GVNetId of PO's
        vector<GVNetId>                 _InoutList;    // GVNetId of Inout's
        vector<GVNetId>                 _FFList;       // GVNetId of Flip Flops (current state)
        vector<GVNetId>                 _ConstList;    // GVNetId of Constants (const0)
        vector<GVNetId>                 _FFConst0List; // GVNetId of FF that stores const0
        // map
        map<unsigned, vector<unsigned>> _id2FaninId;
        map<unsigned, GVNetId>          _id2GVNetId;
        map<unsigned, string>           _netId2Name;
        map<string, unsigned>           _netName2Id;
        map<unsigned, unsigned>         _idRo2Ppi; // PPI: pseudo PI
        map<unsigned, unsigned>         _idRo2Ri;  // RO: register output (Q)
        map<unsigned, unsigned>         _idRi2Ro;  // RI: register input (D)
        map<unsigned, GV_Ntk_Type_t>    _id2Type;
        map<unsigned, vector<GVFanout>> _id2Fanout;
        // flag
        vector<unsigned>                _miscList;   // global misc date list
        unsigned                        _globalMisc; // global misc data for GVNetId in network
        // file type
        unsigned                        _fileType;
    private:
        void reset();
};

// Inline function implementation
inline const GVNetId&
GVNtkMgr::getInputNetId(const GVNetId& id, const uint32_t& i) const {
    unsigned faninId = getFaninId(id.id)[i];
    return getGVNetId(faninId);
}

//----------------------------------------------------------------------
// Forward Declarations
//----------------------------------------------------------------------
class GVRTLDesign;

//----------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------
extern GVRTLDesign* gvRTLDesign;

//----------------------------------------------------------------------
// Data Structure : yosys
//----------------------------------------------------------------------
class GVRTLDesign
{
    public:
        // Constructors for GV RTL Design Instance
        GVRTLDesign() {}
        ~GVRTLDesign();

        // Ntk Reference Functions
        RTLIL::Design* getDesign() { return yosys_design; }

    private:
};

//----------------------------------------------------------------------
// Inline Functions
//----------------------------------------------------------------------

#endif
