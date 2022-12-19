#ifndef simulator_h
#define simulator_h

#include <set>
#include <list>
#include <ctime>
#include <cmath>
#include <bitset>
#include <random>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include <verilated.h>

//#include "svdpi.h"
#include "Vtestf.h"
#include "Vtestf___024root.h"
#include "interface.hpp"
//#include "Vtestf__Dpi.h"

//#include "NumCpp.hpp"

using namespace std;

class Simulator{
public:
    Simulator():
        contextp(make_unique<VerilatedContext>()),
        duv(make_unique<Vtestf>()),
        count(0)
    {
        contextp->debug(0);
        contextp->randReset(2);
        signalMap = new Interface( duv->rootp );
    }
    
    ~Simulator(){
        duv->final();
    }
    
    unique_ptr<VerilatedContext> contextp;
    unique_ptr<Vtestf> duv;
    Interface *signalMap;
    long long unsigned count;
    
    void resetDUV(){
        setCLK(0); setRST(1); duv->eval();
        setCLK(1); setRST(1); duv->eval();
        setCLK(0); setRST(1); duv->eval();
        setCLK(1); setRST(1); duv->eval();
        setCLK(0); setRST(0); duv->eval();
    }
    
    unsigned getPiNum(void){ return signalMap->pi.size(); }
    unsigned getPoNum(void){ return signalMap->po.size(); }
    unsigned getRegNum(void){ return signalMap->reg.size(); }
    
    
    Signal* getPiSignal(unsigned index){ return signalMap->pi[index]; }
    Signal* getPoSignal(unsigned index){ return signalMap->po[index]; }
    Signal* getRegSignal(unsigned index){ return signalMap->reg[index]; }
    
    unsigned getPiWidth(unsigned index){ return signalMap->pi[index]->width; }
    unsigned getPoWidth(unsigned index){ return signalMap->po[index]->width; }
    unsigned getRegWidth(unsigned index){ return signalMap->reg[index]->width; }
    
    unsigned getPiUpper(unsigned index){ return (unsigned)pow(2, getPiWidth(index))-1; }
    unsigned getPoUpper(unsigned index){ return (unsigned)pow(2, getPoWidth(index))-1; }
    unsigned getRegUpper(unsigned index){ return (unsigned)pow(2, getRegWidth(index))-1; }
    
    unsigned getPi(unsigned index){
        if(signalMap->pi[index]->getType() == 'C')
            return *(CData*)(signalMap->pi[index]->value);
        else if(signalMap->pi[index]->getType() == 'I')
            return *(IData*)(signalMap->pi[index]->value);
        else if(signalMap->pi[index]->getType() == 'Q')
            return *(QData*)(signalMap->pi[index]->value);
        return *(SData*)(signalMap->pi[index]->value);
    }

    string getPiName(unsigned index){
        return signalMap->pi[index]->name;
    }

    unsigned getPo(unsigned index){
        if(signalMap->po[index]->getType() == 'C')
            return *(CData*)(signalMap->po[index]->value);
        else if(signalMap->po[index]->getType() == 'I')
            return *(IData*)(signalMap->po[index]->value);
        else if(signalMap->po[index]->getType() == 'Q')
            return *(QData*)(signalMap->po[index]->value);
        return *(SData*)(signalMap->po[index]->value);
    }

    string getPoName(unsigned index){
        return signalMap->po[index]->name;
    }

    unsigned getReg(unsigned index){
        if(signalMap->reg[index]->getType() == 'C')
            return *(CData*)(signalMap->reg[index]->value);
        else if(signalMap->reg[index]->getType() == 'I')
            return *(IData*)(signalMap->reg[index]->value);
        else if(signalMap->reg[index]->getType() == 'Q')
            return *(QData*)(signalMap->reg[index]->value);
        return *(SData*)(signalMap->reg[index]->value);
    }

    string getRegName(unsigned index){
        return signalMap->reg[index]->name;
    }

    void getAllPattern(vector<unsigned> &AllPattern){
        getPiPattern(AllPattern);
        getRegPattern(AllPattern);
        getPoPattern(AllPattern);
    }

    void getRegPattern(vector<unsigned> &regPattern){
        for(unsigned i=0; i<signalMap->reg.size(); i++){
            if(signalMap->reg[i]->getType() == 'C')
                regPattern.push_back(*(CData*)(signalMap->reg[i]->value));
            else if(signalMap->reg[i]->getType() == 'I')
                regPattern.push_back(*(IData*)(signalMap->reg[i]->value));
            else if(signalMap->reg[i]->getType() == 'Q')
                regPattern.push_back(*(QData*)(signalMap->reg[i]->value));
            else
                regPattern.push_back(*(SData*)(signalMap->reg[i]->value));
        }
    };
    void getRegPattern(vector<double> &regPattern){
        for(unsigned i=0; i<signalMap->reg.size(); i++){
            if(signalMap->reg[i]->getType() == 'C')
                regPattern.push_back(*(CData*)(signalMap->reg[i]->value));
            else if(signalMap->reg[i]->getType() == 'I')
                regPattern.push_back(*(IData*)(signalMap->reg[i]->value));
            else if(signalMap->reg[i]->getType() == 'Q')
                regPattern.push_back(*(QData*)(signalMap->reg[i]->value));
            else
                regPattern.push_back(*(SData*)(signalMap->reg[i]->value));
        }
    };
    void getPiPattern(vector<unsigned> &piPattern){
        for(unsigned i=0; i<signalMap->pi.size(); i++){
            if(signalMap->pi[i]->getType() == 'C')
                piPattern.push_back(*(CData*)(signalMap->pi[i]->value));
            else if(signalMap->pi[i]->getType() == 'I')
                piPattern.push_back(*(IData*)(signalMap->pi[i]->value));
            else if(signalMap->pi[i]->getType() == 'Q')
                piPattern.push_back(*(QData*)(signalMap->pi[i]->value));
            else
                piPattern.push_back(*(SData*)(signalMap->pi[i]->value));
        }
    };

    void getPoPattern(vector<unsigned> &poPattern){
        for(unsigned i=0; i<signalMap->po.size(); i++){
            if(signalMap->po[i]->getType() == 'C')
                poPattern.push_back(*(CData*)(signalMap->po[i]->value));
            else if(signalMap->po[i]->getType() == 'I')
                poPattern.push_back(*(IData*)(signalMap->po[i]->value));
            else if(signalMap->po[i]->getType() == 'Q')
                poPattern.push_back(*(QData*)(signalMap->po[i]->value));
            else
                poPattern.push_back(*(SData*)(signalMap->po[i]->value));
        }
    };
    
    void setCLK(unsigned newValue){ *(CData*)(signalMap->clk[0]->value) = newValue; };
    void setRST(unsigned newValue){ *(CData*)(signalMap->rst[0]->value) = newValue; };
    void setPI(unsigned index, unsigned newValue){
        if(signalMap->pi[index]->getType() == 'C')
            *(CData*)(signalMap->pi[index]->value) = newValue;
        else if(signalMap->pi[index]->getType() == 'I')
            *(IData*)(signalMap->pi[index]->value) = newValue;
        else if(signalMap->pi[index]->getType() == 'Q')
            *(QData*)(signalMap->pi[index]->value) = newValue;
        else
            *(SData*)(signalMap->pi[index]->value) = newValue;
    };
    void setPO(unsigned index, unsigned newValue){
        if(signalMap->po[index]->getType() == 'C')
            *(CData*)(signalMap->po[index]->value) = newValue;
        else if(signalMap->po[index]->getType() == 'I')
            *(IData*)(signalMap->po[index]->value) = newValue;
        else if(signalMap->po[index]->getType() == 'Q')
            *(QData*)(signalMap->po[index]->value) = newValue;
        else
            *(SData*)(signalMap->po[index]->value) = newValue;
    };
    void setREG(unsigned index, unsigned newValue){
        if(signalMap->reg[index]->getType() == 'C')
            *(CData*)(signalMap->reg[index]->value) = newValue;
        else if(signalMap->reg[index]->getType() == 'I')
            *(IData*)(signalMap->reg[index]->value) = newValue;
        else if(signalMap->reg[index]->getType() == 'Q')
            *(QData*)(signalMap->reg[index]->value) = newValue;
        else
            *(SData*)(signalMap->reg[index]->value) = newValue;
    };
    
    void setRegPattern(vector<unsigned> &regPattern){
        for(unsigned i=0; i<regPattern.size(); i++){
            if(signalMap->reg[i]->getType() == 'C')
                *(CData*)(signalMap->reg[i]->value) = regPattern[i];
            else if(signalMap->reg[i]->getType() == 'I')
                *(IData*)(signalMap->reg[i]->value) = regPattern[i];
            else if(signalMap->reg[i]->getType() == 'Q')
                *(QData*)(signalMap->reg[i]->value) = regPattern[i];
            else
                *(SData*)(signalMap->reg[i]->value) = regPattern[i];
        }
    };
    
    void setPiPattern(vector<unsigned> &piPattern){
        for(unsigned i=0; i<piPattern.size(); i++){
            if(signalMap->pi[i]->getType() == 'C')
                *(CData*)(signalMap->pi[i]->value) = piPattern[i];
            else if(signalMap->pi[i]->getType() == 'I')
                *(IData*)(signalMap->pi[i]->value) = piPattern[i];
            else if(signalMap->pi[i]->getType() == 'Q')
                *(QData*)(signalMap->pi[i]->value) = piPattern[i];
            else
                *(SData*)(signalMap->pi[i]->value) = piPattern[i];
        }
    };
    
    void setAllPattern(vector<unsigned> &AllPattern){
        cout << "[INFO] Verilator - setAllPattern (size=" << AllPattern.size() << ")" << endl;
        unsigned i, j=0;
        for(i=0; i<getPiNum(); i++){
            if(signalMap->pi[i]->getType() == 'C')
                *(CData*)(signalMap->pi[i]->value) = AllPattern[j];
            else if(signalMap->pi[i]->getType() == 'I')
                *(IData*)(signalMap->pi[i]->value) = AllPattern[j];
            else if(signalMap->pi[i]->getType() == 'Q')
                *(QData*)(signalMap->pi[i]->value) = AllPattern[j];
            else
                *(SData*)(signalMap->pi[i]->value) = AllPattern[j];
            j++;
        }
        for(i=0; i<getRegNum(); i++){
            if(signalMap->reg[i]->getType() == 'C')
                *(CData*)(signalMap->reg[i]->value) = AllPattern[j];
            else if(signalMap->reg[i]->getType() == 'I')
                *(IData*)(signalMap->reg[i]->value) = AllPattern[j];
            else if(signalMap->reg[i]->getType() == 'Q')
                *(QData*)(signalMap->reg[i]->value) = AllPattern[j];
            else
                *(SData*)(signalMap->reg[i]->value) = AllPattern[j];
            j++;
        }
        for(i=0; i<getPoNum(); i++){
            if(signalMap->po[i]->getType() == 'C')
                *(CData*)(signalMap->po[i]->value) = AllPattern[j];
            else if(signalMap->po[i]->getType() == 'I')
                *(IData*)(signalMap->po[i]->value) = AllPattern[j];
            else if(signalMap->po[i]->getType() == 'Q')
                *(QData*)(signalMap->po[i]->value) = AllPattern[j];
            else
                *(SData*)(signalMap->po[i]->value) = AllPattern[j];
            j++;
        }
    };
    
    void printPI(unsigned i){
        unsigned value;
        if(signalMap->pi[i]->getType() == 'C')
            value = *(CData*)(signalMap->pi[i]->value);
        else if(signalMap->pi[i]->getType() == 'I')
            value = *(IData*)(signalMap->pi[i]->value);
        else if(signalMap->pi[i]->getType() == 'Q')
            value = *(QData*)(signalMap->pi[i]->value);
        else
            value = *(SData*)(signalMap->pi[i]->value);
        
        cout << signalMap->pi[i]->name << " " << signalMap->pi[i]->width << " " << value << endl;
    }

    void printPiPattern(){
        cout << "[INFO] Verilator - PrintPiPattern" << endl;
        for (unsigned i=0; i<getPiNum() ; i++){
            cout << "\t" << getPiName(i) << " : " << getPi(i) << endl;
        }
    }

    void printRegPattern(){
        cout << "[INFO] Verilator - PrintRegPattern" << endl;
        for (unsigned i=0; i<getRegNum() ; i++){
            cout << "\t" << getRegName(i) << " : " << getReg(i) << endl;
        }
    }

    void printPoPattern(){
        cout << "[INFO] Verilator - PrintPoPattern" << endl;
        for (unsigned i=0; i<getPoNum() ; i++){
            cout << "\t" << getPoName(i) << " : " << getPo(i) << endl;
        }
    }

    void printPO(unsigned i){
        unsigned value;
        cout << "Type: ";
        cout << signalMap->po[i]->getType() << endl;
        if(signalMap->po[i]->getType() == 'C')
            value = *(CData*)(signalMap->po[i]->value);
        else if(signalMap->po[i]->getType() == 'I')
            value = *(IData*)(signalMap->po[i]->value);
        else if(signalMap->po[i]->getType() == 'Q')
            value = *(QData*)(signalMap->po[i]->value);
        else
            value = *(SData*)(signalMap->po[i]->value);
        cout << signalMap->po[i]->name << " " << signalMap->po[i]->width << " " << value << endl;
    }
    void printREG(unsigned i){
        unsigned value;
        if(signalMap->pi[i]->getType() == 'C')
            value = *(CData*)(signalMap->pi[i]->value);
        else if(signalMap->pi[i]->getType() == 'I')
            value = *(IData*)(signalMap->pi[i]->value);
        else if(signalMap->pi[i]->getType() == 'Q')
            value = *(QData*)(signalMap->pi[i]->value);
        else
            value = *(SData*)(signalMap->pi[i]->value);
        cout << signalMap->reg[i]->name << " " << signalMap->reg[i]->width << " " << value << endl;
    }
    
    void eval(void){
        duv->eval();
        count++;
    }
    
    void evalOneClock(void){
        duv->eval();
        setCLK(1);
        duv->eval();
        setCLK(0);
        duv->eval();
        count+=2;
    }
    
    string pattern2str(vector<unsigned> pattern){
        stringstream ss;
        for (auto it = pattern.begin(); it != pattern.end(); it++) {
            if (it != pattern.begin()) {
                ss << ",";
            }
            ss << *it;
        }
        string pattern_str = ss.str();
        return pattern_str;
    }

    const vector<unsigned> split(const string &str, const char &delimiter) {
        vector<unsigned> result;
        stringstream ss(str);
        string tok;

        while (getline(ss, tok, delimiter)) {
            // cout << "[Warning] Verilator - convert string to integer instead of unsigned" << endl;
            result.push_back(atoi(tok.c_str()));
        }
        return result;
    }

    vector<unsigned> str2pattern(string pattern_str){
        vector<unsigned> pattern = split(pattern_str, ',');
        return pattern;
    }

    string getSequence(){
        vector<string> allPattern;
        stringstream ss;
        for (unsigned i=0; i<getPiNum() ; i++){
            allPattern.push_back(getPiName(i));
        }
        for (unsigned i=0; i<getRegNum() ; i++){
            allPattern.push_back(getRegName(i));
        }
        for (unsigned i=0; i<getPoNum() ; i++){
            allPattern.push_back(getPoName(i));
        }
        for (auto it = allPattern.begin(); it != allPattern.end(); it++) {
            if (it != allPattern.begin()) {
                ss << ",";
            }
            ss << *it;
        }
        string seq = ss.str();
        return seq;
    }

};


#endif /* simulator_h */
