#include <iterator>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <chrono>

#include "../../Constants.h"
#include "Phase2_SearchRunner.h"

using namespace std;

typedef chrono::system_clock Clock;
string rel_path_to_target_dir2 = "./";
int NUM_FINAL_RESULTS = 10;

void Phase2_SearchRunner::init(){
    final_results.clear();
    did_to_vids.clear();
    phase2_ClusterSearcherObj = new Phase2_ClusterSearcher();
    phase2_duration = 0;
}

void Phase2_SearchRunner::re_init(){
    final_results.clear();
    phase2_duration = 0;
}

Phase2_SearchRunner::Phase2_SearchRunner(){
    init();
}

Phase2_SearchRunner::~Phase2_SearchRunner(){
    delete phase2_ClusterSearcherObj;
}

vector<ScoreResult> Phase2_SearchRunner::readResultsFile(string filepath){
	vector<ScoreResult> scoreResults;
	ifstream f_result;
	string line;
	int did;
	double score;
	f_result.open(filepath);
	while(getline(f_result, line)){
		stringstream ss(line); // Insert the string into a stream
    	ss >> did >> score;
    	ScoreResult new_result;
    	new_result.vid = did;
    	new_result.score = score;
    	scoreResults.push_back(new_result);
	}
	f_result.close();
	return scoreResults;
}

unordered_map<int,vector<int>> Phase2_SearchRunner::readConvertTable(string filepath){
	unordered_map<int,vector<int>> did_to_vids;
	ifstream fin;
	string line,buf;
	vector<string> parts;
	int did;
	did_to_vids.clear();
    fin.open(filepath);
    while(getline(fin, line)){
        parts.clear();
    	stringstream ss(line); // Insert the string into a stream
    	while (ss >> buf)
        	parts.push_back(buf);
    	did = stoi(parts[0]);
    	for(int i = 0; i < stoi(parts[1]); i++){
    		did_to_vids[did].push_back(stoi(parts[2+i]));
    	}
    }
    fin.close();
    return did_to_vids;
}

void Phase2_SearchRunner::writeResults(vector<ScoreResult> scoreResults, string filepath, string query){
    ofstream fout;
    fout.open(filepath);
    fout << "Results for: " << query << endl;
    int num_results_to_output = scoreResults.size() < NUM_FINAL_RESULTS ? scoreResults.size(): NUM_FINAL_RESULTS;
    for (int i=0; i< num_results_to_output; i++)
        fout << "vid: " << scoreResults[i].vid << "\tscore: " << scoreResults[i].score<< endl;
    fout.close();
}

void Phase2_SearchRunner::writeResults_again(vector<ScoreResult> scoreResults, string filepath, string query){
	ofstream fout;
	fout.open(filepath, ios_base::app | ios_base::out);
    fout << "Results for: " << query << endl;
    int num_results_to_output = scoreResults.size() < NUM_FINAL_RESULTS ? scoreResults.size(): NUM_FINAL_RESULTS;
    for (int i=0; i< num_results_to_output; i++)
        fout << "vid: " << scoreResults[i].vid << "\tscore: " << scoreResults[i].score<< endl;
    fout.close();
}

void Phase2_SearchRunner::runSearchInCluster(int curr_did, string query){

    //for timing
    /*chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now(); */ // start ticking

    vector<ScoreResult> phase2_cluster_results = phase2_ClusterSearcherObj->searchCluster(query, curr_did);

    /*end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: " << elapsed_seconds.count() << endl;*/

    //store it in final results
    for(int i=0; i < phase2_cluster_results.size(); i++ ){
        ScoreResult new_result;
        new_result.vid = did_to_vids[curr_did][phase2_cluster_results[i].vid];
        new_result.score = phase2_cluster_results[i].score;
        final_results.push_back(new_result);
    }
}

void Phase2_SearchRunner::run_search(int top_k, string full_query, vector<ScoreResult> phase1_results){
    init();
    //for timing
    chrono::time_point<Clock> start, end, start1, end1;
    chrono::duration<double> elapsed_seconds;

	// process results file
	int num_lines = phase1_results.size();

	if(num_lines < top_k){
    	cout << "not enough lines, k less than result length, set k to be " << num_lines << endl;
    	top_k = num_lines;
    }

    start = Clock::now();  // start ticking

    //read data into did_to_vids from convert table
	did_to_vids = readConvertTable(rel_path_to_target_dir2 + RTP::CONVERT_TABLE); // stores the data from CONVERT_TABLE
	
    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase2 Search - reading ./target/convert_table.txt: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
    start1 = Clock::now(); 

    //search on each of the top_k clusters
    int curr_did;
    for(int i=0; i<top_k; i++){
        //start = Clock::now(); 

    	curr_did = phase1_results[i].vid;
        runSearchInCluster(curr_did, full_query);

        /*end = Clock::now();
        elapsed_seconds = end - start;
        cout << "Phase2 Search - Complete search for cluster " << curr_did << ": " << elapsed_seconds.count() << endl;
        start = Clock::now();*/
    }
    end1 = Clock::now();
    elapsed_seconds = end1 - start1;
    //cout << "Phase2 Search - Complete search for " << top_k << " clusters: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
    start = Clock::now();

    //sort final result
    sort(final_results.begin(), final_results.end(), ScoreResult::compare);

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase2 Search - sorting final results: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
    start = Clock::now();

    //write final result to file
    writeResults(final_results, rel_path_to_target_dir2 + RTP::FINAL_RESULTS_FILE_NAME, full_query);

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase2 Search - writing to final_results.txt: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
}

void Phase2_SearchRunner::run_search_again(int top_k, string full_query, vector<ScoreResult> phase1_results){
    re_init();

    //for timing
    chrono::time_point<Clock> start, end, start1, end1;
    chrono::duration<double> elapsed_seconds;

    // process results
    int num_lines = phase1_results.size();

    if(num_lines < top_k){
        cout << "not enough lines, k less than result length, set k to be " << num_lines << endl;
        top_k = num_lines;
    }

    //search on each of the top_k clusters
    int curr_did;
    start1 = Clock::now(); 
    for(int i=0; i<top_k; i++){
        //start = Clock::now(); 

        curr_did = phase1_results[i].vid;
        runSearchInCluster(curr_did, full_query);

        //end = Clock::now();
        //elapsed_seconds = end - start;
        //cout << "Phase2 Search - Complete search for cluster " << curr_did << ": " << elapsed_seconds.count() << endl;
        //start = Clock::now();
    }
    end1 = Clock::now();
    elapsed_seconds = end1 - start1;
    //cout << "Phase2 Search - Complete search for " << top_k << " clusters: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
    start = Clock::now();

    //sort final result
    sort(final_results.begin(), final_results.end(), ScoreResult::compare);

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase2 Search - Time for sorting final_results: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
    start = Clock::now();

    //write final result to file
    writeResults_again(final_results, rel_path_to_target_dir2 + RTP::FINAL_RESULTS_FILE_NAME, full_query);

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase2 Search - time for writing results to final_results.txt: " << elapsed_seconds.count() << endl;
    phase2_duration += elapsed_seconds.count();
}

