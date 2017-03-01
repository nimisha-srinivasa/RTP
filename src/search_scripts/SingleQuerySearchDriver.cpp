#include <iostream>

#include "SingleQuerySearcher.h"
using namespace std;

int main(int argc, char** argv){

	if(argc < 3){
		cout << "Not enough args. Format: ./single_query <query> <query_len> <top_k>" << endl;
		exit(1);
	}
	string query = "";
	for(int i=1; i<argc-2; i++){
		query+=" " + string(argv[i]);
	}

	SingleQuerySearcher* searcher = new SingleQuerySearcher();
	searcher->query=query;
	searcher->num_words_in_query = atoi(argv[argc-2]);
	searcher->top_k = atoi(argv[argc-1]);

	searcher->pre_process_query();
	searcher->run_phase1_lucene_jar();

	searcher->generate_phase1_results();
	searcher->run_phase2_search();
}