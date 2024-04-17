#include "modMgr.h"

#include <string>

#include "gvCmdMgr.h"

ModMgr* modeMgr;

ModType getMode() { return modeMgr->getCurrMode(); };
/**
 * @brief Constructs a ModMgr object with default values.
 *
 */
ModMgr::ModMgr() {
    _gvMode = ModType::MOD_TYPE_SETUP;  // default mode   :  Setup
    setModPromt();
}

/**
 * @brief Resets the Mode Manager object to its initial state.
 *
 */
void ModMgr::reset() {
    setModPromt();
}

/**
 * @brief Sets the Mode Type (setup/vrf)
 *
 * This function sets the current mode type.
 * Default type is the SETUP mode.
 *
 * @param mode MOD_TYPE_SETUP or MOD_TYPE_VRF
 */
void ModMgr::setGVMode(const ModType& mode) {
    _gvMode = mode;
    setModPromt();
}

/**
 * @brief Sets the module prompt (string) based on the current mode.
 *
 * This function sets the module prompt based on the current ModType mode (_gvMode).
 */
void ModMgr::setModPromt() {
    _modPrompt = ModTypeString[_gvMode];
    gvCmdMgr->updateModPrompt(_modPrompt);
}
