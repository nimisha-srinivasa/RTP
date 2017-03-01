/** This project is used to generate clusters; fragment data and index data within each cluster.
 *  Author: Nimisha
 **/
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>

#include "CreateCluster.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3)
    {
        cout << "format: ./gen_cluster <stem.clean.data_file_path> <choice_of_representative>" << endl;
        exit(1);
    }
    string data_file_path = string(argv[1]);
    ifstream is(data_file_path.c_str());

    if (!is){
      cout << ("Could not open file " + data_file_path);
      exit(1);
    }

    istringstream iss( argv[2] );
    int val;
    RepresentativeChoice choice;
    if ((iss >> val) && iss.eof() && (val <= 2)) // Check eofbit
    {
        choice = (RepresentativeChoice)(val-1); //enum values start from 0
    }
    else{
        cout << "invalid <choice_of_representative>: should be between 1 and 2" << endl;
        exit(1);
    }

    CreateCluster* cluster_creator=new CreateCluster();
    cluster_creator->choice = choice;

    string line; // each line of the input file is a document
    int vid=0;
    cluster_creator->init();
    while(getline(is, line))
    {
        // deal with each document
        cluster_creator->deal_with_ver(line, vid);
        vid++;
        if (vid%10000==0)
            printf("done vid = %d\n",vid);
    }
    is.close();

    cluster_creator->gen_index_for_all_cluster();
    cluster_creator->output_convert_table();

    return 0;
}


