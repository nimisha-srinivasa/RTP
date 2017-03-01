#include "Vid_Occurence.h"

bool Vid_Occurence::compare(const Vid_Occurence &obj1, const Vid_Occurence &obj2)
{
    return obj1.vid < obj2.vid;
}