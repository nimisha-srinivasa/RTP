/** This program is used to run the complete phase 1 search
*  Author: Nimisha Srinivasa
**/

#include <vector>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <ctime>

#include "../../Constants.h"
#include "../../data_structures/ReuseTableInfo.h"
#include "../../data_structures/Fid_Occurence.h"
#include "../../data_structures/Vid_Occurence.h"
#include "../../data_structures/Result_Occurence.h"
#include "../../data_structures/ScoreResult.h"

using namespace std;
 
class Phase1_Searcher{

public:
	double duration;  // global variable added to calculate non-I/O time

	void runSearch(string query);
	void runSearchAgain(string query);

private:
	string full_query;
	vector<string> query_words_arr;
	int query_len;
	
	unordered_map<int, vector<ReuseTableInfo>> frag_reuse_table;
	vector<vector<Fid_Occurence>> search_frag; // <<fID, <pos>>>
	unordered_map<int ,int> vid_titlelen_hash; // vid to title length
	unordered_map<int, vector<set<int>>> intersection_hash; // this keeps the intersection results
	vector<ScoreResult> score_result; // this vector keeps the final scoring results
	unordered_map<string, unordered_map<int, double>> super_index;
	clock_t start;  // global variable added to mark the starting point of time statistics

	void splitFullQuery();
	void setQueryLength();
	void init();
	void re_init();
	void read_title_len();
	void read_search_frag();
	void read_superinfo();
	void intersection();
	void scoring();

	//helper methods
	void validate();

	//int body_tf(int vid, vector<set<int>> &occur_terms, int term_number)
	double body_tf_avg(int vid, int term_number);
	void score_page(int vid, vector<set<int>> &occur_terms);

	// Print the information after intersection step. Term_number specified how many terms in the query.
	void print_intersection(int term_number);

	// calculate the min span in title: sliding window algorithm O(n)
	void cal_min_span_title(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number, int title_len);

	// calculate the min span in body: sliding window algorithm O(n)
	void cal_min_span(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number);

};