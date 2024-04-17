#ifndef FILE_TYPE_H
#define FILE_TYPE_H
#include <string>
/**
 * @brief Define the circuit file type
 *
 */
enum FileType {
    VERILOG,
    AIGER,
    AAG,
    BLIF
};
extern const std::string fileTypeStr[4];

/**
 * @brief Define the mode type
 *
 */
enum ModType {
    MOD_TYPE_NONE   = 0,
    MOD_TYPE_SETUP  = 1,
    MOD_TYPE_VERIFY = 2,
    MOD_TYPE_APP    = 3
};
#endif