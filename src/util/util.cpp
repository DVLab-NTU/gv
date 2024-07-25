/****************************************************************************
  FileName     [ util.cpp ]
  PackageName  [ util ]
  Synopsis     [ Define global utility functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyright(c) 2023-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

#include "fmt/core.h"
#include "gvType.h"
#include "myUsage.h"
#include "rnGen.h"

using namespace std;

//----------------------------------------------------------------------
//    Global variables in util
//----------------------------------------------------------------------

RandomNumGen rnGen(0);  // use random seed = 0
MyUsage myUsage;

//----------------------------------------------------------------------
//    Global functions in util
//----------------------------------------------------------------------
//
// List all the file names under "dir" with prefix "prefix"
// Ignore "." and ".."
//
int listDir(vector<string>& files, const string& prefix, const string& dir = ".") {
    DIR* dp;
    dirent* dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << "): failed to open " << dir << "!!\n";
        return errno;
    }

    const char* pp = prefix.size() ? prefix.c_str() : 0;
    while ((dirp = readdir(dp)) != NULL) {
        if (string(dirp->d_name) == "." ||
            string(dirp->d_name) == "..") continue;
        if (!pp || strncmp(dirp->d_name, pp, prefix.size()) == 0)
            files.push_back(string(dirp->d_name));
    }
    sort(files.begin(), files.end());
    closedir(dp);
    return 0;
}

bool systemCmd(const string& cmd, const bool& verbose) {
    std::string execCmd = (!verbose) ? fmt::format("{0} {1}", cmd, "> /dev/null 2>&1") : cmd;
    if (system(execCmd.c_str()) != 0) return false;
    return true;
}

size_t getHashSize(size_t s) {
    if (s < 8) return 7;
    if (s < 16) return 13;
    if (s < 32) return 31;
    if (s < 64) return 61;
    if (s < 128) return 127;
    if (s < 512) return 509;
    if (s < 2048) return 1499;
    if (s < 8192) return 4999;
    if (s < 32768) return 13999;
    if (s < 131072) return 59999;
    if (s < 524288) return 100019;
    if (s < 2097152) return 300007;
    if (s < 8388608) return 900001;
    if (s < 33554432) return 1000003;
    if (s < 134217728) return 3000017;
    if (s < 536870912) return 5000011;
    return 7000003;
}

const std::string fileTypeStr[4] = {
    "VERILOG",
    "AIG",
    "AAG",
    "BLIF"};

void printBanner() {
    ifstream infile("banner.txt");
    std::string line = "";
    while (infile) {
        // infile >> line;
        getline(infile, line);
        std::cout << line << "\n";
    }
    infile.close();
}
