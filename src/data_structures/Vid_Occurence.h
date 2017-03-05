#ifndef _VID_OCCURENCE_H_
#define _VID_OCCURENCE_H_

#include <vector>

using namespace std;

class Vid_Occurence
{
public:
    int vid;
    vector<int> pos;
    static bool compare(const Vid_Occurence &obj1, const Vid_Occurence &obj2);
};

#endif