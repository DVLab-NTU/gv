/****************************************************************************
  FileName     [ util.h ]
  PackageName  [ util ]
  Synopsis     [ Define the prototypes of the exported utility functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#pragma once


#include <cassert>
#include <istream>
#include <vector>

#include "gvType.h"
#include "myUsage.h"
#include "rnGen.h"

using namespace std;

// Extern global variable defined in util.cpp
extern RandomNumGen rnGen;
extern MyUsage myUsage;

// In myString.cpp
extern int myStrNCmp(const string& s1, const string& s2, unsigned n);
extern size_t myStrGetTok(const string& str, string& tok, size_t pos = 0,
                          const char del = ' ');
extern bool myStr2Int(const string& str, int& num);
extern bool isValidVarName(const string& str);

// In myGetChar.cpp
extern char myGetChar(istream&);
extern char myGetChar();

// In util.cpp
extern int listDir(vector<string>&, const string&, const string&);
extern size_t getHashSize(size_t s);
extern bool systemCmd(const string&, const bool&);
extern void printBanner();

// Other utility template functions
template <class T>
void clearList(T& l) {
    T tmp;
    l.swap(tmp);
}

template <class T, class D>
void removeData(T& l, const D& d) {
    size_t des = 0;
    for (size_t i = 0, n = l.size(); i < n; ++i) {
        if (l[i] != d) {  // l[i] will be kept, so des should ++
            if (i != des) l[des] = l[i];
            ++des;
        }
        // else l[i] == d; to be removed, so des won't ++
    }
    l.resize(des);
}


