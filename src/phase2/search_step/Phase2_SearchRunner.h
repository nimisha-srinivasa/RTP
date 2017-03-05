/* This program is used to run the complete phase 2 search 
 * Author: Nimisha Srinivasa
 */

#include <vector>
#include <unordered_map>

#include "../../data_structures/ScoreResult.h"

using namespace std;

class Phase2_SearchRunner {
public:
	Phase2_SearchRunner();
	void run_search(int top_k, int query_len, string query);
	void run_search_again(int top_k, int query_len, string query);


private:
	vector<ScoreResult> final_results;
	unordered_map<int,vector<int>> did_to_vids;

	void init();
	void re_init();
	vector<ScoreResult> readResultsFile(string filepath);
	unordered_map<int,vector<int>> readConvertTable(string filepath);
	void writeResults(vector<ScoreResult> scoreResults, string filepath);
	void runSearchInCluster(int curr_did, string phase2_exec_name, string curr_rel_path_to_target_dir, string query, int query_len);
};