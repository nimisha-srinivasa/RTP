/* This program is used to generate the RESULTS_FILE from 
 * INDEX_FILE, SEARCH_FRAG_FILE, VID_LIST_FILE, FORWARD_FILE within a particular cluster
 * Author: Nimisha Srinivasa
 */

#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <math.h>

#include "../../data_structures/ReuseTableInfo.h"
#include "../../data_structures/ForwardTableInfo.h"
#include "../../data_structures/Fid_Occurence.h"
#include "../../data_structures/Vid_Occurence.h"
#include "../../data_structures/ScoreResult.h"
 
class Phase2_ClusterSearcher{

public:
	int query_len;
	double duration;

	Phase2_ClusterSearcher();
	~Phase2_ClusterSearcher();

	void runSearch();

private:
	static const int MAX_VID = 5500000;//max number of VID
	static const int y = 13; // average number of fragments in one page: 13 wiki, 22 web, 17 git

	unordered_map<int, vector<ReuseTableInfo>> frag_reuse_table;
	unordered_map<int ,int> vid_titlelen_hash; // vid to title length
	vector<ForwardTableInfo> forward_table[MAX_VID];
	vector<vector<Fid_Occurence>> search_frag; // <<fID, <pos>>>
	vector<int> vid_list; // vid list
	unordered_map<int, vector<vector<int>>*> vid_posting; // vid_posting, for each vid, there will be a posting
	vector<vector<Vid_Occurence>> doc_posting; // each term, a list of vid_occurence, result of step 1 is saved in this.
	unordered_map<int, vector<set<int>>> intersection_hash; // this keeps the intersection results
	vector<ScoreResult> score_result; // this vector keeps the final scoring results

	// read reuse table: fid -> a list of vids for Option A
	void read_index();
    // read postings
    void read_search_frag();
    // read vids for Option C
    void read_vid();
    // read forward reuse table: vid -> a list of fids for Option C
    void read_forward();
    // calculate the positional information for each vid
    void get_positional_info();
    void scoring();

	// print search_frag in txt format
	void validate();
	void score_page(int vid, vector<set<int>> &occur_terms);
	// calculate the min span in title: sliding window algorithm O(n)
	void cal_min_span_title(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number, int title_len);	
	// calculate the min span in body: sliding window algorithm O(n)
	void cal_min_span(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number);
	int body_tf(int vid, vector<set<int>> &occur_terms, int term_number);
	int find_index(vector<Fid_Occurence> *term_posting, int fid, int start, int end);
	// binary search within [start, end] using recursion
	int find_index_old(vector<Fid_Occurence> *term_posting, int fid, int start, int end);
	bool MakeChoice(int k);
	void print_vid_postingA();
	void print_vid_postingC();

	void reset_all_data_structures();
};