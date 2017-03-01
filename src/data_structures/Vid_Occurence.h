#include <vector>

using namespace std;

class Vid_Occurence
{
public:
    int vid;
    vector<int> pos;
    static bool compare(const Vid_Occurence &obj1, const Vid_Occurence &obj2);
};