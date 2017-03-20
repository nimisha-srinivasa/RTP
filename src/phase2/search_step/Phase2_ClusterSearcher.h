/* This program is used to generate the phase2_search_results from 
 * INDEX_FILE, FORWARD_FILE within a particular cluster
 * Author: Nimisha Srinivasa
 */


#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "../../data_structures/ReuseTableInfo.h"
#include "../../data_structures/ForwardTableInfo.h"
#include "../../data_structures/Fid_Occurence.h"
#include "../../data_structures/Vid_Occurence.h"
#include "../../data_structures/ScoreResult.h"
#include "../../utils/Serializer.h"
#include "../../Constants.h"
#include "Phase2_IndexSearcher.h"

class Phase2_ClusterSearcher{
	
public:
	vector<ScoreResult> searchCluster(string full_query, int cluster_id);

private:

	string full_query;
	int num_words_in_query;
	int cluster_id;
	set<string> query_words;
	string rel_path_to_cluster;

	//data structures used
	vector<vector<Fid_Occurence>> search_frag; // <<fID, <pos>>>
	vector<int> vid_list; // vid list
	unordered_map<int, vector<ReuseTableInfo>> frag_reuse_table;
	unordered_map<int ,int> vid_titlelen_hash; // vid to title length
	vector<ForwardTableInfo> forward_table[RTP::MAX_VID];
	vector<vector<Vid_Occurence>> doc_posting; // each term, a list of vid_occurence, result of step 1 is saved in this.
	unordered_map<int, vector<set<int>>> intersection_hash; // this keeps the intersection results
	vector<ScoreResult> score_result; // this vector keeps the final scoring results

	//member functions
	void init();
	void read_index();
	void computeSearchFrag(string index_path);
	void setQueryLength();
	void setQueryWords();
	bool makeChoice(int k);
	void get_positional_info();
	void scoring();
	void score_page(int vid, vector<set<int>> &occur_terms);
	void cal_min_span_title(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number, int title_len);
	void cal_min_span(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number);
	int body_tf(int vid, vector<set<int>> &occur_terms, int term_number);
	int find_index(vector<Fid_Occurence> *term_posting, int fid, int start, int end);
	void clean_text(char *buffer);
	uint64_t compute_term_id(string term);
	vector<vector<Fid_Occurence>> findSearchFrags(string index_path, string full_query);	

};