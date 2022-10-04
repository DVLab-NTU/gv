
#ifndef GV_NTK_C
#define GV_NTK_C

#include "gvNtk.h"
#include "gvMsg.h"
#include "gvStrUtil.h"
#include "cassert"

extern AbcMgr* abcMgr;

/* -------------------------------------------------- *\
 * Class GVNtk Implementations
\* -------------------------------------------------- */
// Constuctor and Destructor
GVNtk::GVNtk() {
   for (uint32_t i = 0; i < 3; ++i) _IOList[i].clear();
   _FFList.clear(); _ConstList.clear();
   _globalClk = GVNetUD;
   gvAbcMgr = abcMgr;
}

GVNtk::~GVNtk() {
}

// Ntk Construction Functions
void
GVNtk::initialize() {
   assert (!_inputData.size());
   // Create Constant AIG_FALSE
   const GVNetId id = createNet(1); assert (!id.id); createConst(id);
}

const GVNetId
GVNtk::createNet(const uint32_t& width) {
   // Validation Check
   if (width != 1) {
      gvMsg(GV_MSG_ERR) << "Unexpected Net width = " << width << " in Base / AIG Network !!" << endl;
      return GVNetUD;
   }
//    assert (_inputData.size() == _typeMisc.size());
   // Create New GVNetId
   GVNetId id = GVNetId::makeNetId(_inputData.size()); // assert (!isGVNetInverted(id));
//    _typeMisc.push_back(GVMiscType()); _inputData.push_back(GVInputVec());
//    assert (GV_PI == _typeMisc.back().type); assert (!_typeMisc.back().misc);
   assert (!_inputData.back().size()); 
   return id;
}

void
GVNtk::createModule(GVNtkModule* const module) {
   assert (module); assert (module->getNtkRef());
   _ntkModule.push_back(module);
}

void
GVNtk::createInput(const GVNetId& id) {
   assert (validNetId(id)); assert (!isGVNetInverted(id));
   assert (!reportMultipleDrivenNet(GV_PI, id));
   _IOList[0].push_back(id);
}

void
GVNtk::createOutput(const GVNetId& id) {
   assert (validNetId(id)); _IOList[1].push_back(id);
}

void
GVNtk::createInout(const GVNetId& id) {
   assert (validNetId(id)); assert (!isGVNetInverted(id));
   createGate(GV_PIO, id); _IOList[2].push_back(id);
}

void
GVNtk::createLatch(const GVNetId& id) {
   assert (validNetId(id)); assert (!isGVNetInverted(id));
   createGate(GV_FF, id); _FFList.push_back(id);
}

void
GVNtk::createConst(const GVNetId& id) {
   assert (validNetId(id)); assert (!isGVNetInverted(id));
   createGate(dynamic_cast<const GVBvNtk*>(this) ? BV_CONST : AIG_FALSE, id); _ConstList.push_back(id);
}

void
GVNtk::createClock(const GVNetId& id) {
   assert (validNetId(id)); assert (GV_PI == getGateType(id));
   assert (GVNetUD == _globalClk); _globalClk = id;
}

void
GVNtk::setInput(const GVNetId& id, const GVInputVec& inputs) {
   assert (validNetId(id)); assert (GV_PI == getGateType(id));
   GVInputVec& fanInVec = _inputData[id.id]; assert (!fanInVec.size());
   for (uint32_t i = 0; i < inputs.size(); ++i) fanInVec.push_back(inputs[i]);
}

void
GVNtk::createGate(const GVGateType& type, const GVNetId& id) {
   // Check Validation
   assert (validNetId(id)); assert (type < GV_XD); assert (type > GV_PI);
   assert (dynamic_cast<const GVBvNtk*>(this) || type <= AIG_FALSE);
   assert (!(dynamic_cast<const GVBvNtk*>(this)) || (type <= GV_MODULE || type > AIG_FALSE));
   assert (!reportMultipleDrivenNet(type, id)); assert (!reportUnexpectedFaninSize(type, id));
   // Set Gate Type
//    _typeMisc[id.id].type = type;
}


// Constructors for BV Network Gates
GVBvNtk::GVBvNtk() : GVNtk() {
   _netWidth.clear();
}

GVBvNtk::GVBvNtk(const GVBvNtk& ntk) : GVNtk(ntk) {
   _netWidth = ntk._netWidth;
}

GVBvNtk::~GVBvNtk() {
};

// Ntk Construction Functions
void
GVBvNtk::initialize() {
   assert (!_inputData.size());
   // Create Constant BV_CONST = 1'b0 for Sync with AIG_FALSE
   const GVNetId id = createNet(1); assert (!id.id);
   _inputData.back().push_back(0); createConst(id);
}

const GVNetId
GVBvNtk::createNet(const uint32_t& width) {
   // Validation Check
   if (!width) {
      gvMsg(GV_MSG_ERR) << "Unexpected Net width = " << width << " in BV Network !!" << endl;
      return GVNetUD;
   }
   assert (_inputData.size() == _netWidth.size()); _netWidth.push_back(width);
//    assert (_inputData.size() == _typeMisc.size());
   // Create New GVNetId
   GVNetId id = GVNetId::makeNetId(_inputData.size()); assert (!isGVNetInverted(id));
//    _typeMisc.push_back(GVMiscType()); 
   _inputData.push_back(GVInputVec());
//    assert (GV_PI == _typeMisc.back().type); assert (!_typeMisc.back().misc);
   assert (!_inputData.back().size()); return id;
}

// Ntk Structure Functions
const uint32_t
GVBvNtk::getNetWidth(const GVNetId& id) const {
   assert (validNetId(id)); return _netWidth[id.id];
}


#endif