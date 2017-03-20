/* This program is used to run a single query
 * Author: Nimisha Srinivasa
 */

#include <string>
#include <vector>

#include "../phase1/search_step/Phase1_Searcher.h"
#include "../phase2/search_step/Phase2_SearchRunner.h"

using namespace std;

class SingleQuerySearcher{
public:

	string full_query;
	int top_k;
	Phase1_Searcher* phase1_searcher;
	Phase2_SearchRunner* phase2_searcher;

	SingleQuerySearcher();
	~SingleQuerySearcher();
	void runSearch_without_preprocess();
	void searchAgain_without_preprocess();
	void runSearch();

	
private:
	vector<ScoreResult> phase1_results;
	
	void pre_process_query();
	void run_phase1_lucene_jar();
	void generate_phase1_results();
	void generate_phase1_results_again();
	void run_phase2_search();
	void run_phase2_search_again();
	
	//helper methods
	char* appendChar(char* str, char c);
	string stemstring(struct stemmer * z, string string1);
	string doStem(string str);
	bool isSpecialChar(char c);
	string doClean(string str);
	string doStemClean(string str);
	int setQueryLength();

};