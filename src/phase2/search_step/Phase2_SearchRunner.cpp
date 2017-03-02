#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#include "../../Constants.h"
#include "../../data_structures/ScoreResult.h"

#define DEBUG 1

using namespace std;

string rel_path_to_target_dir2 = "./";
string rel_path_to_source_dir = "../src/";
bool show_output = false;

//global vars for the program
vector<ScoreResult> final_results;
unordered_map<int,vector<int>> did_to_vids;

vector<ScoreResult> readResultsFile(string filepath){
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

unordered_map<int,vector<int>> readConvertTable(string filepath){
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

void writeResults(vector<ScoreResult> scoreResults, string filepath){
	ofstream fout;
	fout.open(filepath);

    for (int i=0; i<scoreResults.size(); i++)
        fout << scoreResults[i].vid << " " << scoreResults[i].score<< endl;
    fout.close();
}

void runSearchInCluster(int curr_did, string phase2_exec_name, string curr_rel_path_to_target_dir, string query, int query_len){
    string command, cd_command;

    //move to the directory
    cd_command = "cd "+ rel_path_to_target_dir2+ "cluster/" + to_string(curr_did) + "; ";
    cout << "**********enter cluster " << curr_did << "**********" << endl;
    system(command.c_str());

    //lucene search to generate "search_frag.txt"
    command = cd_command + "rm -rf ";
    command += RTP::SEARCH_FRAGMENT_FILE_NAME;
    system(command.c_str());
    command = cd_command + "cp "+ curr_rel_path_to_target_dir + phase2_exec_name + " ./";
    system(command.c_str());
    command = cd_command + "./" + phase2_exec_name + " --index index --no-features --postings " + query + " &> /dev/null" ;
    cout << "Generate search_frag.txt" << endl;
    system(command.c_str());
    command = cd_command + "rm -rf " + phase2_exec_name;
    system(command.c_str());

    //generate vidlist.txt from bitmap.txt
    command = cd_command + curr_rel_path_to_target_dir + "phase2_read_bitmap "+query;
    cout << "generate vidlist.txt" << endl;
    system(command.c_str());

    //generate result.txt
    command = cd_command + curr_rel_path_to_target_dir + "phase2_cluster_search "+ to_string(query_len);
    cout << "run ./phase2_cluster_search. Generate result.txt" << endl;
    system(command.c_str());
    
    //read results of this dir into final results
    vector<ScoreResult> phase2_cluster_results = readResultsFile("./cluster/" + to_string(curr_did) + "/" + RTP::RESULT_FILE_NAME);

    //store it in final results
    for(int i=0; i < phase2_cluster_results.size(); i++ ){
        ScoreResult new_result;
        new_result.vid = did_to_vids[curr_did][phase2_cluster_results[i].vid];
        new_result.score = phase2_cluster_results[i].score;
        final_results.push_back(new_result);
    }
}

int main(int argc, char** argv){

	if(argc<4){
		cout << "Illegal num of args: <exec_name> <top_k> <query_len> <query_word1> <query_word2> .. <query_wordn>" << endl;
		exit(1);
	}

	int top_k = stoi(argv[1]);
	int query_len = stoi(argv[2]);

	string query = string(argv[3]);
	for(int i=4; i< argc; i++){
		query += " " + string(argv[i]);
	}

	// process result file
	vector<ScoreResult> phase1_results = readResultsFile(rel_path_to_target_dir2 + RTP::RESULT_FILE_NAME);
	int num_lines = phase1_results.size();

	if(num_lines < top_k){
    	cout << "not enough lines, k<result length, set k to be " << num_lines << endl;
    	top_k = num_lines;
    }

    //read data into did_to_vids from convert table
	did_to_vids = readConvertTable(rel_path_to_target_dir2 + RTP::CONVERT_TABLE); // stores the data from CONVERT_TABLE
	
    
    string phase2_exec_name = "phase2_index_search";
    string curr_rel_path_to_target_dir = "../../"; //from the cluster/x dir

    int curr_did;
    final_results.clear();
    //search on each of the top_k clusters
    for(int i=0; i<top_k; i++){
    	curr_did = phase1_results[i].vid;
        runSearchInCluster(curr_did, phase2_exec_name, curr_rel_path_to_target_dir, query, query_len);
    }

    //sort final result
    sort(final_results.begin(), final_results.end(), ScoreResult::compare);

    //write final result to file
    writeResults(final_results, rel_path_to_target_dir2 + RTP::FINAL_RESULTS_FILE_NAME);
}