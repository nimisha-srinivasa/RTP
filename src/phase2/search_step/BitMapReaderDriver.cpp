#include <iostream>
#include "BitMapReader.h"

using namespace std;
//format: <program_exec> <conjunctive_query>
int main(int argc, char** argv){
    if (argc <=2){
        cout << "need at least 2 terms" << endl;
        exit(1);
    }

    BitMapReader* reader = new BitMapReader();
    for (int i=1; i<argc; i++){
        string term = argv[i];
        reader->query.insert(term);
    }

    reader->read_bitmap();
    return 0;
}