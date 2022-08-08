#include "gvModMgr.h"
#include <iostream>
#include <cstring>
#include <string>

GVModMgr* gvModMgr;

/* -------------------------------------------------- *\
 * Class GVModMgr Implementations
\* -------------------------------------------------- */
GVModMgr::GVModMgr(){
    _inputFileExist = false;
    _gvMode = GVModType::GV_MOD_TYPE_SETUP;      // default mode   :  Setup
    _gvEng  = GVModEngine::GV_MOD_ENGINE_YOSYS;  // default engine :  yosys
    setModPromt();
}

GVModMgr::~GVModMgr(){
}

/* ------------------------- *\
 * GET functions
\* ------------------------- */
bool 
GVModMgr::getInputFileExist(){
    return _inputFileExist;
}

GVModType    
GVModMgr::getGVMode(){
    return _gvMode;
}

GVModEngine  
GVModMgr::getGVEngine(){
    return _gvEng;
}

string  
GVModMgr::getModPrompt(){
    setModPromt(); // update mod prompt
    return _modPrompt;
}

/* ------------------------- *\
 * SET functions
\* ------------------------- */
void 
GVModMgr::setInputFileExist(bool exist){
    _inputFileExist = exist;
}

void 
GVModMgr::setGVMode(GVModType mode){
    _gvMode = mode;
}
void 
GVModMgr::setGVEngine(GVModEngine engine){
    _gvEng = engine;
}
void 
GVModMgr::setModPromt(){
    if(_gvMode == GVModType::GV_MOD_TYPE_SETUP)
        _modPrompt = GVEngineString[_gvEng] + GVModTypeString[_gvMode];
    else
        _modPrompt = GVModTypeString[_gvMode];
}



