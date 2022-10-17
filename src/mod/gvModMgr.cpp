#include "gvModMgr.h"
#include <iostream>
#include <cstring>
#include <string>

GVModMgr* gvModMgr;


/* -------------------------------------------------- *\
 * Class GVModMgr Implementations
\* -------------------------------------------------- */
GVModMgr::GVModMgr(){
    _inputFileExist = false; _property = -1;
    _propertySet = false;
    _inputFileName  = ""; _aig_name = "";
    _gvMode         = GVModType::GV_MOD_TYPE_SETUP;      // default mode   :  Setup
    _gvEng          = GVModEngine::GV_MOD_ENGINE_YOSYS;  // default engine :  yosys
    _vrfMode        = {GV_CMD_TYPE_VERIFY, GV_CMD_TYPE_SIMULATE, GV_CMD_TYPE_COMMON, GV_CMD_TYPE_MOD};
    _setupMode      = {GV_CMD_TYPE_SIMULATE, GV_CMD_TYPE_VERIFY};
    setModPromt();
}

GVModMgr::~GVModMgr(){
}

bool GVModMgr::checkModeType(GVCmdType& currCmdType)
{
    /*for(int i = 0; i < _vrfMode.size(); ++i){
        cout << _vrfMode[i] << endl;
    }*/

    if(getGVMode() == GV_MOD_TYPE_VERIFY){
        for(size_t i = 0; i < _vrfMode.size(); ++i){
            if(currCmdType == _vrfMode[i])
                return true;
        }
        cout << "Please switch to \"SETUP MODE\" !!" << endl;
    }
    else{
        for(size_t i = 0; i < _setupMode.size(); ++i){
            if(currCmdType == _setupMode[i]){
                cout << "Please switch to \"VRF MODE\" !!" << endl;
                return false;
            }
        }
        return true;
        /*if(cmdType == GV_CMD_TYPE_SIMULATE || cmdType == GV_CMD_TYPE_VERIFY){
            gvMsg(GV_MSG_ERR) << "Please switch to \"VRF MODE\" !!" << endl;
            return true;
        }*/
    }

    return false;
}


/* ------------------------- *\
 * GET functions
\* ------------------------- */
bool 
GVModMgr::getInputFileExist(){
    return _inputFileExist;
}

string 
GVModMgr::getInputFileName(){
    return _inputFileName;
}

string 
GVModMgr::getAigFileName(){
    return _aig_name;
}

string 
GVModMgr::getTopModuleName(){
    return _top_module_name;
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

int
GVModMgr::getSafe(){
    return _property;
}

/* ------------------------- *\
 * SET functions
\* ------------------------- */
void 
GVModMgr::setInputFileExist(bool exist){
    _inputFileExist = exist;
}

void
GVModMgr::setInputFileName(string& filename){
    _inputFileName = filename;
}

void
GVModMgr::setAigFileName(string aigFileName){
     _aig_name = aigFileName;
}

void
GVModMgr::setTopModuleName(string topModuleName){
     _top_module_name = topModuleName;
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
void 
GVModMgr::setSafe(int p){
    _property = p;
    _propertySet = true;
}


