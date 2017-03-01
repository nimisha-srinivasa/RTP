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
    vector<string> query_words_arr;

    if (argc-2 != searcher->query_len)
    {
        cout << "query_len is not equal to query length" << endl;
        exit(1);
    }
    for (int i=0; i<searcher->query_len; i++)
        searcher->query.push_back(argv[2+i]);
    
    searcher->runSearch();

    // added by Susen
    cout << "Phase1 Search: Duration without I/O: " << searcher->duration << endl;
    // added by Susen
    return 0;
}
