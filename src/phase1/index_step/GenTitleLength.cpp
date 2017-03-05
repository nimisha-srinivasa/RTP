/** This project is used to generate number of words in the titles of documents and store it in TITLE_LENGTH_FILE
*  Author: Jiyu
**/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <stdlib.h>

#include "../../Constants.h"
using namespace std;

string rel_path_to_target_dir = "./";

//generates TITLE_LENGTH_FILE_NAME from the representative document
int main(int argc, char** argv){
	if (argc != 2){
		printf("Need file name!");
		return 0;
	}

	string filename = string(argv[1]);
    ifstream fin;
    fin.open(filename.c_str());
	
	ofstream title_length;
    string title_len = rel_path_to_target_dir + RTP::TITLE_LENGTH_FILE_NAME;
    title_length.open(title_len.c_str(), ios::ate);

    string line; // each line of the input file is a document
    int vid=0;

    while(getline(fin, line))
    {
        size_t found = line.find_first_of("\t");
    	size_t len = found + 1;
    	string title;
    	title = line.substr(0, len);
    	int tlen = 1;
    	for (int i = 0; i < title.length(); i++){
    		if (title[i]== ' ')
    			tlen++;
    	}
    	title_length << vid << " " << tlen << endl;
        vid++;
    }
    title_length << -99999 << endl;
    fin.close();
    title_length.close();
    return 0;
}