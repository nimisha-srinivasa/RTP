/* This program is used to run a batch query
 * Author: Nimisha Srinivasa
 */

#include <iostream>
#include <fstream>
#include <vector>

#include "SingleQuerySearcher.h"

using namespace std;

string rel_path_to_target_dir3 = "./";

// NOTE: the query_file contains the cleaned, stemmed data
int main(int argc, char** argv){
	if(argc<=2){
		cout << "Illegal num of args. Format: ./batch_query_search <query_file> <top_k>" << endl;
		exit(1);
	}
	string query_file_name = string(argv[1]);
	int top_k = atoi(argv[2]);

	ifstream fin;
	string line;
	bool first_time = true;
	int curr_query_num=1;
	
    SingleQuerySearcher* single_searcher = new SingleQuerySearcher();
	single_searcher->top_k = top_k;

    fin.open(rel_path_to_target_dir3 + query_file_name);
    
    while (std::getline(fin, line)){
    	cout << "####################Processing query number: " << curr_query_num << "####################" << endl;
        cout << "Results for: " << line << endl;
        single_searcher->full_query = line;
        cout << "Processing query: " << line << endl;
    	if(first_time){
    		single_searcher->runSearch_without_preprocess();
    		first_time = false;

    	}else{
    		single_searcher->searchAgain_without_preprocess();
    	}

    	cout << "#################### Done with query number: " << curr_query_num << "####################" << endl;
    	curr_query_num ++;
    }
    delete single_searcher;
    curr_query_num--;
}