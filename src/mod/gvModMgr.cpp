#include "gvModMgr.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "gvCmdMgr.h"

GVModMgr* gvModMgr;

// --------------------------------------------------
// Class GVModMgr Implementations
// --------------------------------------------------
GVModMgr::GVModMgr() {
    _property      = -1;
    _propertySet   = false;
    _inputFileName = "";
    _aig_name      = "";
    _gvMode        = GVModType::GV_MOD_TYPE_SETUP;  // default mode   :  Setup
    setModPromt();
}

void GVModMgr::reset() {
    _property      = -1;
    _propertySet   = false;
    _inputFileName = "";
    _aig_name      = "";
    setModPromt();
}

void GVModMgr::setGVMode(GVModType mode) {
    _gvMode = mode;
    setModPromt();
}

void GVModMgr::setModPromt() {
    _modPrompt = GVModTypeString[_gvMode];
    gvCmdMgr->updateModPrompt(_modPrompt);
}

void GVModMgr::setSafe(int p) {
    _property    = p;
    _propertySet = true;
}
