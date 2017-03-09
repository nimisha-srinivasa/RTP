#include <iostream>
#include <chrono>

#include "SingleQuerySearcher.h"
using namespace std;

typedef chrono::system_clock Clock;

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

	chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

	searcher->runSearch();

	end = Clock::now();
    elapsed_seconds = end - start;
    cout << "The query " <<  query <<" took: " << elapsed_seconds.count() << endl;

	delete searcher;
	return 0;
}