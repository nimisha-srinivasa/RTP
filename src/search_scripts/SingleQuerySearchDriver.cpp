#include <iostream>

#include "SingleQuerySearcher.h"
using namespace std;

int main(int argc, char** argv){

	if(argc < 3){
		cout << "Not enough args. Format: ./single_query <top_k> <query> " << endl;
		exit(1);
	}

	SingleQuerySearcher* searcher = new SingleQuerySearcher();
	searcher->top_k = atoi(argv[1]);

	string query = string(argv[2]);
	for(int i=3; i<argc; i++){
		query+=" " + string(argv[i]);
	}
	
	cout << "The full query is:" << query << endl;
	searcher->full_query=query;

	searcher->runSearch();
	return 0;
}