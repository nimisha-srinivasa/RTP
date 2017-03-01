#include <string>
#include <vector>

#include "../phase1/search_step/Phase1_Searcher.h"

using namespace std;

class SingleQuerySearcher{
public:
	vector<string> query_words_arr;
	string full_query;
	int num_words_in_query;
	int top_k;
	Phase1_Searcher* phase1_searcher;

	void runSearch_without_preprocess();
	void searchAgain_without_preprocess();
	void runSearch();
	
	void pre_process_query();
	void run_phase1_lucene_jar();
	void generate_phase1_results();
	void generate_phase1_results_again();
	void run_phase2_search();
	
private:
	//helper methods
	char* appendChar(char* str, char c);
	string stemstring(struct stemmer * z, string string1);
	string doStem(string str);
	bool isSpecialChar(char c);
	string doClean(string str);
	string doStemClean(string str);

};