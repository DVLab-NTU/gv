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

class Simulator{
public:
    Simulator():
        contextp(std::make_unique<VerilatedContext>()),
        duv(std::make_unique<Vtestf>()),
        count(0)
    {
        contextp->debug(0);
        contextp->randReset(2);
        signalMap = new Interface( duv->rootp );
    }
    
    ~Simulator(){
        duv->final();
    }
    
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<Vtestf> duv;
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
    
    unsigned getPI(unsigned index){
        if(signalMap->pi[index]->getType() == 'C')
            return *(CData*)(signalMap->pi[index]->value);
        else if(signalMap->pi[index]->getType() == 'I')
            return *(IData*)(signalMap->pi[index]->value);
        else if(signalMap->pi[index]->getType() == 'Q')
            return *(QData*)(signalMap->pi[index]->value);
        return *(SData*)(signalMap->pi[index]->value);
    }
    unsigned getPO(unsigned index){
        if(signalMap->po[index]->getType() == 'C')
            return *(CData*)(signalMap->po[index]->value);
        else if(signalMap->po[index]->getType() == 'I')
            return *(IData*)(signalMap->po[index]->value);
        else if(signalMap->po[index]->getType() == 'Q')
            return *(QData*)(signalMap->po[index]->value);
        return *(SData*)(signalMap->po[index]->value);
    }
    unsigned getREG(unsigned index){
        if(signalMap->reg[index]->getType() == 'C')
            return *(CData*)(signalMap->reg[index]->value);
        else if(signalMap->reg[index]->getType() == 'I')
            return *(IData*)(signalMap->reg[index]->value);
        else if(signalMap->reg[index]->getType() == 'Q')
            return *(QData*)(signalMap->reg[index]->value);
        return *(SData*)(signalMap->reg[index]->value);
    }
    void getRegPattern(std::vector<unsigned> &regPattern){
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
    void getRegPattern(std::vector<double> &regPattern){
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
    void getPiPattern(std::vector<unsigned> &piPattern){
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

    void getPoPattern(std::vector<unsigned> &poPattern){
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
    
    void setRegPattern(std::vector<unsigned> &regPattern){
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
    
    void setPiPattern(std::vector<unsigned> &piPattern){
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
        
        std::cout << signalMap->pi[i]->name << " " << signalMap->pi[i]->width << " " << value << std::endl;
    }
    void printPO(unsigned i){
        unsigned value;
        std::cout << "Type: ";
        std::cout << signalMap->po[i]->getType() << std::endl;
        if(signalMap->po[i]->getType() == 'C')
            value = *(CData*)(signalMap->po[i]->value);
        else if(signalMap->po[i]->getType() == 'I')
            value = *(IData*)(signalMap->po[i]->value);
        else if(signalMap->po[i]->getType() == 'Q')
            value = *(QData*)(signalMap->po[i]->value);
        else
            value = *(SData*)(signalMap->po[i]->value);
        std::cout << signalMap->po[i]->name << " " << signalMap->po[i]->width << " " << value << std::endl;
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
        std::cout << signalMap->reg[i]->name << " " << signalMap->reg[i]->width << " " << value << std::endl;
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
    
};


#endif /* simulator_h */
