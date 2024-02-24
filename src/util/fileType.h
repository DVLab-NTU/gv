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
#endif