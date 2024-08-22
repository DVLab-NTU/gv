#include <fmt/core.h>

#include "vcdMgr.h"

/**
 * @brief
 *
 */
void testVCDMgr() {
    VCDMgr* myVCDMgr = new VCDMgr();
    myVCDMgr->readVCDFile("debug.vcd");
    myVCDMgr->printAllSignals();

    VCDSignal* signal    = myVCDMgr->getRefenceSignal("sum");
    VCDSignalValues vals = *myVCDMgr->getSignalDeltaValue(signal->hash);
    for (int i = 0; i < vals.size(); ++i) {
        std::string val = myVCDMgr->getHexValue(vals[i]->value);
        fmt::println("Time: {0}  Val: {1}", vals[i]->time, val);
    }
}

/**
 * @brief
 *
 */
void testGlucose() {

}