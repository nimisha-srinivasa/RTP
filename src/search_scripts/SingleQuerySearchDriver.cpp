#include <iostream>

#include "SingleQuerySearcher.h"
using namespace std;

int main(int argc, char** argv){

	if(argc < 3){
		cout << "Not enough args. Format: ./single_query <query> <query_len> <top_k>" << endl;
		exit(1);
	}

	SingleQuerySearcher* searcher = new SingleQuerySearcher();

	string query = "";
	for(int i=1; i<argc-2; i++){
		query+=" " + string(argv[i]);
		searcher->query_words_arr[i-1].assign(string(argv[i]));
	}
	
	searcher->full_query=query;
	searcher->num_words_in_query = atoi(argv[argc-2]);
	searcher->top_k = atoi(argv[argc-1]);
	
	searcher->pre_process_query();
	searcher->run_phase1_lucene_jar();

	//query_words_arr as it is required for the generate_phase1_results()
	for(int i=1; i<argc-2; i++){
		searcher->query_words_arr.push_back(string(argv[i]));
	}

	searcher->generate_phase1_results();
	searcher->run_phase2_search();
}