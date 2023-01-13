#include "gvModMgr.h"
#include "gvCmdMgr.h"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

GVModMgr* gvModMgr;

/* -------------------------------------------------- *\
 * Class GVModMgr Implementations
\* -------------------------------------------------- */
GVModMgr::GVModMgr() {
    _inputFileExist = false;
    _property       = -1;
    _propertySet    = false;
    _inputFileName  = "";
    _aig_name       = "";
    _gvMode         = GVModType::GV_MOD_TYPE_SETUP; // default mode   :  Setup
    _gvEng = GVModEngine::GV_MOD_ENGINE_YOSYS;      // default engine :  yosys
    setModPromt();
}

GVModMgr::~GVModMgr() {}

/* ------------------------- *\
 * GET functions
\* ------------------------- */
bool
GVModMgr::getInputFileExist() {
    return _inputFileExist;
}

string
GVModMgr::getInputFileName() {
    return _inputFileName;
}

string
GVModMgr::getAigFileName() {
    return _aig_name;
}

string
GVModMgr::getTopModuleName() {
    return _top_module_name;
}

GVModType
GVModMgr::getGVMode() {
    return _gvMode;
}

GVModEngine
GVModMgr::getGVEngine() {
    return _gvEng;
}

string
GVModMgr::getModPrompt() {
    setModPromt(); // update mode prompt
    return _modPrompt;
}

int
GVModMgr::getSafe() {
    return _property;
}

void
GVModMgr::printWizardPrompt(int promptStart, int promptLength) {
    if (promptStart < 0) {
        if (promptStart == -1) {
            cout << "[CORRECT COMMAND] !! ";
            cout << "(press Enter to continue) ...";
        } else if (promptStart == -2) {
            // cout << "[ERROR COMMAND] !! ";
            cout << "(press Enter to type the command again) ...";
        }
        cin.get();
        return;
    }

    int idx = 0;
    while (idx++ < promptLength) cout << _wizardContent[promptStart++] << "\n";
}

void
GVModMgr::printWizardProgress(int pos, int promptNum) {
    float float_percent = (static_cast<float>(pos) / (promptNum - 1)) * 100;
    int   int_percent   = float_percent;
    int   idx           = 0;
    cout << "progress : [";
    while (idx++ < pos) cout << "=";
    cout << ">";
    idx = 0;
    while (idx++ < promptNum - pos - 1) cout << " ";
    cout << "] ";

    cout << int_percent << "%\n\n";
}

/* ------------------------- *\
 * SET functions
\* ------------------------- */
void
GVModMgr::setInputFileExist(bool exist) {
    _inputFileExist = exist;
}

void
GVModMgr::setInputFileName(string& filename) {
    _inputFileName = filename;
}

void
GVModMgr::setAigFileName(string aigFileName) {
    _aig_name = aigFileName;
}

void
GVModMgr::setTopModuleName(string topModuleName) {
    _top_module_name = topModuleName;
}

void
GVModMgr::setGVMode(GVModType mode) {
    _gvMode = mode;
    setModPromt();
}
void
GVModMgr::setGVEngine(GVModEngine engine) {
    _gvEng = engine;
    setModPromt();
}
void
GVModMgr::setModPromt() {
    if (_gvMode == GVModType::GV_MOD_TYPE_SETUP)
        _modPrompt = GVEngineString[_gvEng] + GVModTypeString[_gvMode];
    else _modPrompt = GVModTypeString[_gvMode];
    gvCmdMgr->updateModPrompt(_modPrompt);
}
void
GVModMgr::setSafe(int p) {
    _property    = p;
    _propertySet = true;
}

void
GVModMgr::setWizardContent(string prompt) {
    _wizardContent.push_back(prompt);
}
