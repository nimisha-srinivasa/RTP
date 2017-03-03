#include <iostream>

#include "SingleQuerySearcher.h"
using namespace std;

int main(int argc, char** argv){

	if(argc < 3){
		cout << "Not enough args. Format: ./single_query <top_k> <query_len> <query> " << endl;
		exit(1);
	}

	SingleQuerySearcher* searcher = new SingleQuerySearcher();
	searcher->num_words_in_query = atoi(argv[2]);
	searcher->top_k = atoi(argv[1]);

	string query = "";
	for(int i=3; i<argc; i++){
		query+=" " + string(argv[i]);
		searcher->query_words_arr.push_back(string(argv[i]));
	}
	
	cout << "The full query is:" << query << endl;
	searcher->full_query=query;

	searcher->runSearch();
	return 0;
}