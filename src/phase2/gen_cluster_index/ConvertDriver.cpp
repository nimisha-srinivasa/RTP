#include <iostream>

#include "Convert.h"

// convert the traditional reuse table to a forward reuse table for a particular did
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "need one argument" << endl;
        return 1;
    }
    Convert * convertObj = new Convert();
    convertObj->did = argv[1];
    convertObj->init();
    convertObj->read_index();
    convertObj->output_forward_table();
    return 0;
}