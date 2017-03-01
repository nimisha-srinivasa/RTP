#include <vector>

#include "../../Constants.h"
#include "../../data_structures/ForwardPosting.h"

using namespace std;

class Convert{
public:
	char *did; // which cluster

	void init();
    void read_index();
    void output_forward_table();

private:
	static const int MAX_VID=400000; // need to be >= max vid in "index.txt"
	vector<ForwardPosting> v[MAX_VID]; // this saves the forward posting for each page
};