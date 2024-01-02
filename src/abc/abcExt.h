#ifndef ABC_EXT_H
#define ABC_EXT_H

extern void initCirMgr(const int& piNum, const int& poNum, const int& regNum, const int& totNum);

/**
 * @brief Extern functions under the circuit manager
 */
extern bool inputIsPi(const int& gateId);
extern void parseInput(const int& idx, const int& gateId);
extern void parseOutput(const int& idx, const int& gateId, const int& in0Id, const int& inv, string poName);
extern void parseAig(const int& gateId, const int& in0Id, const int& in0Inv, const int& in1Id, const int& in1Inv);
extern void parseRo(const int& idx, const int& gateId);
extern void parseRi(const int& idx, const int& gateId, const int& in0Id, const int& inv);
extern void parseRiRo(const int& riGid, const int& roGid);

#endif
