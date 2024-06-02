#include "vcdMgr.h"

#include "simMgr.h"

//! Convert a VCDBit to a single char
static char VCDBit2Char(VCDBit b) {
    switch (b) {
        case (VCD_0):
            return '0';
        case (VCD_1):
            return '1';
        case (VCD_Z):
            return 'Z';
        case (VCD_X):
        default:
            return 'X';
    }
}

void VCDMgr::readVCDFile() {
    std::string fileName = simMgr->getVcdFileName();
    _trace = _vcdParser.parse_file(fileName);
    if (_trace == nullptr) {
        std::cout << "ERROR: VCD Parsing Error !!" << std::endl;
        // Something went wrong.
    } else {
        for (VCDScope* scope : *_trace->get_scopes()) {
            std::cout << "Scope: " << scope->name << std::endl;

            for (VCDSignal* signal : scope->signals) {
                std::cout << "\t" << signal->hash << "\t" << signal->reference;
                if (signal->size > 1)
                    std::cout << " [" << signal->size << ":0]";
                std::cout << std::endl;
            }
        }
    }
}

void VCDMgr::printVCDFile() {
    // Get the first signal we fancy.
    VCDSignal* mysignal = _trace->get_scope("adder")->signals[1];
    // VCDSignalValues* vals = _trace->get_signal_values(mysignal->hash);
    // std::cout << vals->front()->time;

    // Print the value of this signal at every time step.
    for (VCDTime time : *_trace->get_timestamps()) {
        VCDValue* val = _trace->get_signal_value_at(mysignal->hash, time);
        std::cout << "t = " << time << ", " << mysignal->reference << " = ";
        // Assumes val is not nullptr!
        switch (val->get_type()) {
            case (VCD_SCALAR): {
                std::cout << VCDBit2Char(val->get_value_bit());
                break;
            }
            case (VCD_VECTOR): {
                VCDBitVector* vecval = val->get_value_vector();
                for (auto it = vecval->begin(); it != vecval->end(); ++it) {
                    std::cout << VCDBit2Char(*it);
                }
                break;
            }
            case (VCD_REAL): {
                std::cout << val->get_value_real();
            }
            default: {
                break;
            }
        }

        std::cout << std::endl;
    }
}