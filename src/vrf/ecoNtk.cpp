#ifndef ECO_NTK_CPP
#define ECO_NTK_CPP

#include "ecoNtk.h"
#include "base/abc/abc.h"
#include "proof/fraig/fraig.h"
#include <iostream>
#include <string>

namespace gv {
namespace cir {
// print the gate type name
string
EcoGate::getGateTypeName() {
  switch (_gateType) {
    case ECO_CONST_0_GATE:
      return "CONST_0_GATE";
    case ECO_CONST_1_GATE:
      return "CONST_1_GATE";
    case ECO_AND_GATE:
      return "AND_GATE";
    case ECO_OR_GATE:
      return "OR_GATE";
    case ECO_NAND_GATE:
      return "NAND_GATE";
    case ECO_NOR_GATE:
      return "NOR_GATE";
    case ECO_XOR_GATE:
      return "XOR_GATE";
    case ECO_XNOR_GATE:
      return "XNOR_GATE";
    case ECO_BUF_GATE:
      return "BUF_GATE";
    case ECO_NOT_GATE:
      return "NOT_GATE";
    default:
      return "NONE_GATE";
  }
}

// split the line by white spaces
vector<string> splitLine(const string& line) {
  vector<string> ret;
  string buf;
  for(size_t i = 0, n = line.size(); i < n; i++) {
    if(line[i] != ' ')
      buf.push_back(line[i]);
    if(line[i] == ' ' || i == n - 1) {
      if(!buf.empty()) {
        ret.push_back(buf);
        buf.clear();
      }
    }
  }
  return ret;
}

// get the net names in the primitive nets
vector<string> getGateNets(const string& line) {
  vector<string> ret;
  string buf;
  bool flag = false;
  for(size_t i = 0, n = line.size(); i < n; i++) {
    if(line[i] == '(')
      flag = true;
    
    if(flag && line[i] != ' ' && line[i] != ',' && line[i] != '(' && line[i] != ')')
      buf.push_back(line[i]);
    else if(flag) {
      if(!buf.empty()) {
        ret.push_back(buf);
        buf.clear();
      }
    }
    if(line[i] == ')')
      break;
  }
  return ret;
}

vector<string> getWires(const string& line) {
  vector<string> ret;
  string buf;
  int lsb = -1, msb = -1;
  bool flag = false;
  for(size_t i = 0, n = line.size(); i < n; i++) {
    if(line[i] == ' ')
      flag = true;
    
    if(flag && line[i] != ' ' && line[i] != ',' && line[i] != ';')
      buf.push_back(line[i]);
    else if(flag) {
      if(!buf.empty()) {
        if(buf[0] == '[') {
          string lsbStr, msbStr;
          bool afterColon = false;
          for(size_t j=0; j<buf.size(); j++) {
            if(buf[j] == ':') {
              afterColon = true;
              continue;
            }
            else if(buf[j] == '[' || buf[j] == ']')
              continue;
            if(!afterColon)
              lsbStr.push_back(buf[j]);
            else
              msbStr.push_back(buf[j]);
          }
          cout << lsbStr << " " << msbStr << endl;
          lsb = stoi(lsbStr);
          msb = stoi(msbStr);
          if(lsb > msb)
            swap(lsb, msb);
        }
        else if(buf != "wire"){
          if(lsb >= 0) {
            for(int i=lsb; i<=msb; i++) {
              ret.push_back(buf + "[" + to_string(i) + "]");
            }
          }
          else
            ret.push_back(buf);
        }
        buf.clear();
      }
    }
  }
  return ret;
}

void
EcoNtk::parseNtkFile(const string& dir) {
  // rewrite the design file (handle capital chars and gate name stuff)
    ifstream file(dir);
    assert(file.is_open());
    ofstream fout("/C/Users/User/Documents/gv/tmp.v");
    string buf;
    unordered_set<string> gateTypeStrings = {"and", "or", "nand", "nor", "not", "buf", "xor", "xnor"};
    unordered_set<string> wires;
    while (getline(file, buf))
    {
      // splitting the items in the line by white space
      vector<string> items = splitLine(buf);
      
      // if the line is not empty
      if(!items.empty()) {
        string firstTok = items.at(0);
        // assure that the first token is not capital
        if(firstTok.at(0) >= 'A' && firstTok.at(0) <= 'Z') {
          for(size_t i=0; i<firstTok.size(); i++)
            firstTok[i] = static_cast<char>(firstTok[i] + ('a' - 'A'));
        }
        // if it is a primitive
        if(gateTypeStrings.count(firstTok)) {
          assert(items.size() > 1);
          vector<string> nets = getGateNets(buf);
          for(const auto& net : nets) {
            if(!wires.count(net)) {
              fout << "wire " << net << ";" << endl;
              wires.insert(net);
            }
          }
          fout << firstTok << " ";
          // ignore gate name
          string gateNameItem = items.at(1);
          bool flag = false;
          for(size_t i=0; i<gateNameItem.size(); i++) {
            if(gateNameItem.at(i) == '(')
              flag = true;
            if(!flag) continue;
            fout << gateNameItem.at(i);
          }
          fout << " ";
          // print the remaining items
          for(size_t i=2; i<items.size(); i++) {
            fout << items.at(i) << " ";
          }
          fout << endl;
        }
        else {
          if(firstTok == "wire") {
            vector<string> lineWires = getWires(buf);
            for(const auto& w : lineWires)
              wires.insert(w);
          }
          fout << buf << endl;
        }
      }
    }

  // abc read file parameters
  Fraig_Params_t Params, * pParams = &Params;
  int fAllNodes = 1;
  int fExdc = 0;
  Abc_Obj_t * pObj;
  Abc_Obj_t * pNode;
  int i;
  memset( pParams, 0, sizeof(Fraig_Params_t) );
  pParams->nPatsRand  = 2048; // the number of words of random simulation info
  pParams->nPatsDyna  = 2048; // the number of words of dynamic simulation info
  pParams->nBTLimit   =  100; // the max number of backtracks to perform
  pParams->fFuncRed   =    1; // performs only one level hashing
  pParams->fFeedBack  =    1; // enables solver feedback
  pParams->fDist1Pats =    1; // enables distance-1 patterns
  pParams->fDoSparse  =    1; // performs equiv tests for sparse functions
  pParams->fChoicing  =    0; // enables recording structural choices
  pParams->fTryProve  =    0; // tries to solve the final miter
  pParams->fVerbose   =    0; // the verbosiness flag
  pParams->fVerboseP  =    0; // the verbosiness flag

  Abc_Ntk_t* pNtk = Io_Read( "./tmp.v", IO_FILE_VERILOG, 0, 0 );
  // cout << "ntk name " << Abc_NtkName(pNtk) << endl;

  cirV->readCirFromAbcNtk(pNtk);
}

// bool
// newFaninComp(Abc_Obj_t *pObj, int inIdx) {
//     assert(inIdx == 0 || inIdx == 1);
//     bool ret = false;
//     if(inIdx == 0) {
//         ret = Abc_ObjFaninC0(pObj);
//         if(Abc_AigNodeIsConst(Abc_ObjFanin0(pObj))) {
//             ret ^= true;
//         }
//     }
//     else {
//         ret = Abc_ObjFaninC1(pObj);
//         if(Abc_AigNodeIsConst(Abc_ObjFanin1(pObj))) {
//             ret ^= true;
//         }
//     }
//     return ret;
// }

// const bool
// EcoCir::readCirFromAbcNtk(Abc_Ntk_t* pNtk) {
//     // TODO : Convert abc ntk to gv aig ntk
//     CirGateV gateV;
//     // Abc_Ntk_t* pNtk = NULL;            // the gia pointer of abc
//     Abc_Obj_t *pObj, *pObjRi, *pObjRo; // the obj element of gia
//     unsigned iPi = 0, iPo = 0, iRi = 0, iRo = 0;
//     int i;

// cout << "pppppppsp " << Abc_NtkPiNum(pNtk) << endl;
//     // initialize the size of the containers
//     initCir(Abc_NtkPiNum(pNtk), Abc_NtkPoNum(pNtk), Abc_NtkLatchNum(pNtk), Abc_NtkObjNumMax(pNtk));

//     // increment the global travel id for circuit traversing usage
//     // Abc_NtkIncrementTravId(pNtk);

//     // since we don't want to traverse the constant node, set the TravId of the
//     // constant node to be as the global one
//     // Abc_NodeSetTravIdCurrent(pGia, Gia_ManConst0(pGia));
//     _totGateList.push_back(_const0);
//     cout << "pppppppppppppppppppppppppp" << endl;
    
//     // set the const node
//     _totGateList[0] = _const0;
//     // traverse the obj's in topological order
//     Abc_NtkForEachObj( pNtk, pObj, i) {
//         char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
//         if(Abc_ObjIsPi(pObj)) {
//             // cout<< "pi " << Abc_ObjId(pObj) << endl;
            
//             CirPiGate* gate = new CirPiGate(Abc_ObjId(pObj), 0);
            
//             _piList[iPi++] = gate;
//             _totGateList[Abc_ObjId(pObj)] = gate;
//             gate->setName(name);
//         }
//         else if(Abc_ObjIsPo(pObj)) {
//         //  
//         // cout<< "po " << Abc_ObjId(pObj) << endl;
//         }
//         else if(Abc_ObjIsNode(pObj)) {
//             // cout<< "node " << Abc_ObjId(pObj) << " / " << Abc_NtkObjNumMax(pNtk) << endl;
//             CirAigGate *gate = new CirAigGate(Abc_ObjId(pObj), 0);
//             _totGateList[Abc_ObjId(pObj)] = gate;
//             gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), newFaninComp(pObj, 0));
//             gate->setIn1(getGate(Abc_ObjId(Abc_ObjFanin1(pObj))), newFaninComp(pObj, 1));
//             // char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
//             gate->setName(name);
//         }
//         // TODO : handle latches

//         // else if(Abc_ObjIsBo(pObj)) {
//         //     CirRoGate* gate = new CirRoGate(Abc_ObjId(pObj), 0);
//         //     _roList[iRo++] = gate;
//         //     _totGateList[Abc_ObjId(pObj)] = gate;
//         // }
//         // else if(Abc_ObjIsBi(pObj)) {
//         //     CirRiGate *gate = new CirRiGate(Abc_ObjId(pObj), 0, Abc_ObjId(Abc_ObjFanin0(pObj)));
//         //     gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), newFaninComp(pObj, 0));
//         //     _riList[iPo++] = gate;
//         //     _totGateList[Abc_ObjId(pObj)] = gate;
//         // }
//         else if(Abc_AigNodeIsConst(pObj)) {
//             // cout << "I am const1 " << Abc_ObjId(pObj) <<  endl;
//         }
//         else {
//             // cout << "not defined gate type" << endl;
//         }
//     }
//     // handle the po's
//     Abc_NtkForEachPo(pNtk, pObj, i) {
//         char *name = new char[strlen(Abc_ObjName(pObj))+1]; strcpy(name, Abc_ObjName(pObj));
//         CirPoGate *gate = new CirPoGate(Abc_ObjId(pObj), 0, Abc_ObjId(Abc_ObjFanin0(pObj)));
//         gate->setIn0(getGate(Abc_ObjId(Abc_ObjFanin0(pObj))), newFaninComp(pObj, 0));
//         _poList[iPo++] = gate;
//         _totGateList[Abc_ObjId( pObj)] = gate;
//         gate->setName(name);
//     }
    
//     // add the fanout information
//     Abc_NtkForEachObj( pNtk, pObj, i) {
//         CirGate* gate = getGate(Abc_ObjId(pObj));
//         if(gate->getType() == PI_GATE || gate->getType() == AIG_GATE) {
//           for(int j=0; j<Abc_ObjFanoutNum(pObj); ++j)
//             addFanout(gate, getGate(Abc_ObjId(Abc_ObjFanout(pObj, j))));
//             // if(gate->getType() == PI_GATE) {
//             //     for(int j=0; j<Abc_ObjFanoutNum(pObj); ++j) {
//             //         static_cast<CirPiGate*>(gate)->addOut(getGate(Abc_ObjId(Abc_ObjFanout(pObj, j))));
//             //     }
//             // }
//             // else {
//             //     for(int j=0; j<Abc_ObjFanoutNum(pObj); ++j) {
//             //         static_cast<CirAigGate*>(gate)->addOut(getGate(Abc_ObjId(Abc_ObjFanout(pObj, j))));
//             //     }
//             // }
//         }
//     }

//     genDfsList();
//     printNetlist();
//     // checkFloatList();
//     // checkUnusedList();
// }

}
}
#endif
