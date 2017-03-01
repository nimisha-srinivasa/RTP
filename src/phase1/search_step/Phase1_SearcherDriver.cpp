#include <stdlib.h>
#include <iostream>
#include <vector>

#include "Phase1_Searcher.h"

using namespace std;

int main(int argc, char** argv)
{

    if (argc < 3)
    {
        cout << "need at least two parameters for query_len and query" << endl;
        exit(1);
    }

    Phase1_Searcher* searcher = new Phase1_Searcher();
    searcher->query_len=atoi(argv[1]);

    if (argc-2 != searcher->query_len)
    {
        cout << "query_len is not equal to query length" << endl;
        exit(1);
    }
    for (int i=0; i<searcher->query_len; i++)
        searcher->query[i].assign(argv[2+i]);
    
    searcher->init();
    searcher->read_index();
    searcher->read_search_frag();
    searcher->read_superinfo();
    searcher->intersection();
    searcher->scoring();

    // added by Susen
    cout << "Phase1 Search: Duration without I/O: " << searcher->duration << endl;
    // added by Susen
    return 0;
}
