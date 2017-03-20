/* This program is used to convert from traditional reuse table to forward reuse table
 * Author: Xin Jin
 * Dec. 10. 2014
 */

#include <vector>

#include "../../Constants.h"
#include "../../data_structures/ForwardTableInfo.h"

using namespace std;

class Convert{
public:
	char *did; // which cluster

	void init();
    void read_index();
    void output_forward_table();

private:
	static const int MAX_VID=400000; // need to be >= max vid in "index.txt"
	vector<ForwardTableInfo> v[MAX_VID]; // this saves the forward posting for each page
};