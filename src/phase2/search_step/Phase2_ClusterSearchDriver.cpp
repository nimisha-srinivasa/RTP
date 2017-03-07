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
    phase2_ClusterSearcherObj->runSearch();
    delete phase2_ClusterSearcherObj;
    return 0;
}

