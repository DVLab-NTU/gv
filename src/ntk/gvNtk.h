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
    GV_NTK_OBJ_PPI,    // 7: Pseudo Primary Input
    GV_NTK_OBJ_FF_CS,  // 8: Flip Flop Current State
    GV_NTK_OBJ_FF_NS,  // 9: Flip Flop Next State
    GV_NTK_OBJ_NOT,    // 10: NOT node
    GV_NTK_OBJ_LAST,   // 11: last element of the type
    GV_NTK_OBJ_AIG     // 12: AIG node
} GV_Ntk_Type_t;

//----------------------------------------------------------------------
// GV Ntk Defines
//----------------------------------------------------------------------
struct GVNetId {
        unsigned       cp : 1;
        unsigned       id : 31;
        bool           fanin0Cp;
        bool           fanin1Cp;
        GV_Ntk_Type_t  type : GV_NTK_OBJ_AND;
        static GVNetId makeNetId(unsigned i = GVNtkUD, unsigned c = 0,
                                 GV_Ntk_Type_t t = GV_NTK_OBJ_AND,
                                 bool f0cp = false, bool f1cp = false) {
            GVNetId j;
            j.cp       = c;
            j.id       = i;
            j.type     = t;
            j.fanin0Cp = f0cp;
            j.fanin1Cp = f1cp;
            return j;
        }
        // void setComplement(int fanin, bool& cp){if(fanin){fanin1Cp =
        // cp;}else{fanin0Cp = cp;}}
        GVNetId    operator~() const { return makeNetId(id, cp ^ 1); }
        const bool operator==(const GVNetId& i) const {
            return cp == i.cp && id == i.id;
        }
        const bool operator!=(const GVNetId& i) const { return !(*this == i); }
};

class GVNtkMgr
{
    public:
        // Constructors for BV Network
        GVNtkMgr() { reset(); };
        ~GVNtkMgr() { reset(); }
        inline const GV_Ntk_Type_t getGateType(const GVNetId& id) {
            return id.type;
        }
        inline const uint32_t getNetSize() const { return _id2GVNetId.size(); }
        inline const unsigned getInputSize() const {
            return _InputList.size();
        } // get the # of Inputs (PI + Latch's output)
        inline const unsigned getOutputSize() const {
            return _OutputList.size();
        } // get the # of Outsputs (Co = PO + Latch's input)
        inline const unsigned getInoutSize() const {
            return _InoutList.size();
        } // get the # of Inout's
        inline const uint32_t getFFSize() const {
            return _FFList.size();
        } // get the # of FF's
        inline const uint32_t getConstSize() const {
            return _ConstList.size();
        }
        inline const uint32_t getFFConst0Size() const {
            return _FFConst0List.size();
        }
        inline const GVNetId& getInput(const unsigned& i) const {
            assert(i < getInputSize());
            return _InputList[i];
        } // get the i'th input
        inline const GVNetId& getOutput(const unsigned& i) const {
            assert(i < getOutputSize());
            return _OutputList[i];
        } // get the i'th output
        inline const GVNetId& getInout(const unsigned& i) const {
            assert(i < getInoutSize());
            return _InoutList[i];
        } // get the i'th inout
        inline const GVNetId& getLatch(const unsigned& i) const {
            assert(i < getFFSize());
            return _FFList[i];
        } // get the i'th FF
        inline const GVNetId& getFFConst0(const unsigned& i) const {
            assert(i < getFFConst0Size());
            return _FFConst0List[i];
        }
        inline const GVNetId& getConst(const unsigned& i) const {
            assert(i < getConstSize());
            return _ConstList[i];
        }
        inline const vector<unsigned>& getfaninId(const unsigned& i) const {
            return _id2faninId.at(i);
        }
        inline const GVNetId& getGVNetId(const unsigned& i) const {
            return _id2GVNetId.at(i);
        }

        // ntk traversal functions
        inline const GVNetId& getInputNetId(const GVNetId&,
                                            const uint32_t&) const;
        // Ntk Misc Data Functions
        inline void           newMiscData() {
            if (getNetSize() > _miscList.size()) {
                _miscList.resize(getNetSize());
            }
            ++_globalMisc;
        }
        inline bool isLatestMiscData(const GVNetId& id) const {
            return _globalMisc == _miscList[id.id];
        }
        inline void setLatestMiscData(const GVNetId& id) {
            _miscList[id.id] = _globalMisc;
        }

        // net id/name mapping
        inline unsigned getNetIdFromName(string name) {
            return _netName2Id[name];
        } // get the net id from its name
        inline string getNetNameFromId(unsigned id) {
            return _netId2Name[id];
        } // get the net name from its id
        inline unsigned getPPIidFromROid(unsigned id) { return _idRO2PPI[id]; }
        inline unsigned getRIidFromROid(unsigned id) { return _idRO2RI[id]; }
        inline unsigned getROidFromRIid(unsigned id) { return _idRI2RO[id]; }
        void            parseAigMapping(Gia_Man_t* pGia);

        // construct ntk
        void createNet(const GVNetId& id, const int net_type);
        void createNetFromAbc(char*);

        // print ntk
        void print_rec(Gia_Man_t* pGia, Gia_Obj_t* pObj);

        // build the BDD
        const bool setBddOrder(const bool&);
        void       buildNtkBdd();
        void       buildBdd(const GVNetId& netId);

        // DFS tranversal
        void dfsOrder(const GVNetId&, vector<GVNetId>&);

        // print functions
        void printPi();      // print the information of all PI's
        void printPo();      // print the information of all PO's
        void printRi();      // print the information of all RI's
        void printSummary(); // print the information of all Obj in the aig ntk

    protected:
        // GV
        vector<GVNetId> _InputList;    // GVNetId of PI's
        vector<GVNetId> _OutputList;   // GVNetId of PO's
        vector<GVNetId> _InoutList;    // GVNetId of Inout's
        vector<GVNetId> _FFList;       // GVNetId of Flip Flops (current state)
        vector<GVNetId> _ConstList;    // GVNetId of Constants
        vector<GVNetId> _FFConst0List; // GVNetId of Constant0 stored by FF
        // map
        map<unsigned, vector<unsigned>> _id2faninId; // use id to get its fanin
        map<unsigned, GVNetId>  _id2GVNetId; // use id to get its net struct
        map<unsigned, string>   _netId2Name; // use the net id to get its name
        map<string, unsigned>   _netName2Id; // use the net name to get its id
        map<unsigned, unsigned> _idRO2PPI;   // map register Q id to PPI id
        map<unsigned, unsigned> _idRO2RI; // map register Q id to register D id
        map<unsigned, unsigned> _idRI2RO; // map register output id to PPI id
        // flag
        vector<unsigned>        _miscList; // Global Misc Date List
        unsigned _globalMisc; // Global Misc Data for GVNetId in Ntk
    private:
        void reset();
};

// Inline function implementation
inline const GVNetId&
GVNtkMgr::getInputNetId(const GVNetId& id, const uint32_t& i) const {
    unsigned faninId = getfaninId(id.id)[i];
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

        // Ntk I/O Functions

    private:
};

//----------------------------------------------------------------------
// Inline Functions
//----------------------------------------------------------------------

#endif
