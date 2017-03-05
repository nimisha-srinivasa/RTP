#include <iostream>

#include "Phase2_ClusterSearcher.h"

using namespace std;


int main(int argc, char** argv)
{
    if (argc !=2)
    {
        cout << "need one parameter for query_len" << endl;
        exit(1);
    }

    Phase2_ClusterSearcher* phase2_ClusterSearcherObj = new Phase2_ClusterSearcher();
    phase2_ClusterSearcherObj->query_len=atoi(argv[1]);
    // read reuse table: fid -> a list of vids for Option A
    phase2_ClusterSearcherObj->read_index();
    // read postings
    phase2_ClusterSearcherObj->read_search_frag();
    // read vids for Option C
    phase2_ClusterSearcherObj->read_vid();
    // read forward reuse table: vid -> a list of fids for Option C
    phase2_ClusterSearcherObj->read_forward();
    // calculate the positional information for each vid
    phase2_ClusterSearcherObj->get_positional_info();
    //the result is writen to RESULT_FILE_NAME
    phase2_ClusterSearcherObj->scoring();
    return 0;
}

