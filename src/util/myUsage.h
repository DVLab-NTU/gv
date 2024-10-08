/****************************************************************************
  FileName     [ myUsage.h ]
  PackageName  [ util ]
  Synopsis     [ Report the run time and memory usage ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <sys/resource.h>
#include <sys/times.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>

using namespace std;

#undef MYCLK_TCK
#define MYCLK_TCK sysconf(_SC_CLK_TCK)

class MyUsage {
public:
    MyUsage() { reset(); }

    void reset() {
        _initMem        = checkMem();
        _currentTick    = checkTick();
        _periodUsedTime = _totalUsedTime = 0.0;
    }

    void report(bool repTime, bool repMem) {
        if (repTime) {
            setTimeUsage();
            cout << "Period time used : " << setprecision(4)
                 << _periodUsedTime << " seconds" << endl;
            cout << "Total time used  : " << setprecision(4)
                 << _totalUsedTime << " seconds" << endl;
        }
        if (repMem) {
            setMemUsage();
            cout << "Total memory used: " << setprecision(4) << _currentMem
                 << " M Bytes" << endl;
        }
    }

private:
    // for Memory usage (in MB)
    double _initMem;
    double _currentMem;

    // for CPU time usage
    double _currentTick;
    double _periodUsedTime;
    double _totalUsedTime;

    // private functions
    double checkMem() const {
        struct rusage usage;
        if (0 == getrusage(RUSAGE_SELF, &usage))
#ifdef __APPLE__
            return usage.ru_maxrss / double(1 << 20);  // bytes
#else
            return usage.ru_maxrss / double(1 << 10);  // KBytes
#endif

        else return 0;
    }
    double checkTick() const {
        tms tBuffer;
        times(&tBuffer);
        return tBuffer.tms_utime;
    }
    void setMemUsage() { _currentMem = checkMem() - _initMem; }
    void setTimeUsage() {
        double thisTick = checkTick();
        _periodUsedTime = (thisTick - _currentTick) / double(MYCLK_TCK);
        _totalUsedTime += _periodUsedTime;
        _currentTick = thisTick;
    }
};
