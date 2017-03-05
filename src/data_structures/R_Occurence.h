#ifndef _R_OCCURENCE_INFO_H_
#define _R_OCCURENCE_INFO_H_

#include <vector>
#include <set>
// result occurence
class R_Occurence
{
public:
    int vid;
    vector<set<int>> pos;
};

#endif