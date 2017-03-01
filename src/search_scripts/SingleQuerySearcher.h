#include <string>

using namespace std;

class SingleQuerySearcher{
public:
	string query;
	int num_words_in_query;
	int top_k;

	void pre_process_query();
	void run_phase1_lucene_jar();
	void generate_phase1_results();
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