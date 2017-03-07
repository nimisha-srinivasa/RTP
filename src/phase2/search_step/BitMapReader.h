/* This program is used to generate the VID_LIST_FILE from BITMAP_FILE within a particular cluster
 * Author: Nimisha Srinivasa
 */

#include <set>
#include <string>

using namespace std;

class BitMapReader{
public:
	set<string> query;

	BitMapReader();
	~BitMapReader();
	void read_bitmap();
private:
	void reset_all_data_structures();
}; 