/* This program is used to generate the VID_LIST_FILE from BITMAP_FILE within a particular cluster
 * Author: Nimisha Srinivasa
 */

#include <set>
#include <string>

using namespace std;

class BitMapReader{
public:
	set<string> query;
	void read_bitmap();
};