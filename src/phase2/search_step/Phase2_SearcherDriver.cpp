#include <iostream>

#include "Phase2_Searcher.h"

using namespace std;


int main(int argc, char** argv)
{
    if (argc !=2)
    {
        cout << "need one parameter for query_len" << endl;
        exit(1);
    }

    Phase2_Searcher* Phase2_SearcherObj = new Phase2_Searcher();
    Phase2_SearcherObj->query_len=atoi(argv[1]);
    // read reuse table: fid -> a list of vids for Option A
    Phase2_SearcherObj->read_index();
    // read postings
    Phase2_SearcherObj->read_search_frag();
    // read vids for Option C
    Phase2_SearcherObj->read_vid();
    // read forward reuse table: vid -> a list of fids for Option C
    Phase2_SearcherObj->read_forward();
    // calculate the positional information for each vid
    Phase2_SearcherObj->get_positional_info();
    //the result is writen to RESULT_FILE_NAME
    Phase2_SearcherObj->scoring();
    return 0;
}

