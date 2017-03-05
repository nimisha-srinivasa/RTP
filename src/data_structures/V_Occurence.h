#ifndef _V_OCCURENCE_H_
#define _V_OCCURENCE_H_

#include <set>

using namespace std;
// vid occurence
class V_Occurence
{
public:
    int vid;
    set<int> pos;

    static bool compare(const V_Occurence &obj1, const V_Occurence &obj2);
};

#endif