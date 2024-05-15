//
//  utility.hpp
//  crv_01
//
//  Created by Es1chUb on 2022/8/1.
//  Copyright © 2022 dhgir.abien. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include "crpg.hpp"

class RandomGen{
public:
    RandomGen(bool isDebug)
    {
        unsigned seed;
        
        if (isDebug) {
            seed = 84133409;
        } else {
            seed = rd();
        }
        
        std::cout << "Random Seed = " << seed << std::endl;
        ranEngine = std::default_random_engine(seed);
    }
    
    unsigned getRandNum(unsigned lower, unsigned upper){
        std::uniform_int_distribution<unsigned> _uni64(lower, upper);
        return _uni64(ranEngine);
    }
    
    std::random_device rd;
    std::default_random_engine ranEngine;
};
RandomGen rGen(false);

class Timer{
public:
    Timer(){}
    
    void start(void){
        start_timestamp = clock();
    }
        
    double getTime(void){
        return (clock()-start_timestamp)/(double)(CLOCKS_PER_SEC);
    }
    
    clock_t start_timestamp;
};
Timer timer;

#endif /* utility_h */
