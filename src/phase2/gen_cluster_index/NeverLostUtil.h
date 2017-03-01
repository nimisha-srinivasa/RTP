/*
NeverLostUtil
    // the toolbox of my MS project.
    // include tools for: directory reading, file read/write, sha-1 checksum, 
    // append store read/write etc, compare two directories.
*/

#ifndef _NEVERLOST_UTIL_H_
#define _NEVERLOST_UTIL_H_

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <string.h>

#include "rabin.h"
/*extern "C" {
    #include "rabin.h"
}*/

#include <cstdint>

using namespace std;
class NeverLostUtil {
public:
    static string readFileContent(const char* filename);
    static void writeFileContent(const char* filename, const string& content_str);
    // chunking
    static void chunkContent(const string& content_str, const bool var, const uint32_t chunksize, vector<string> &chunks);
};



#endif /* _NEVERLOST_UTIL_H_ */
