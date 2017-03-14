#include <fstream>

#include "BitMapReader.h"
#include "../../Constants.h"

using namespace std;

string rel_path_to_target_dir = "./";

//reads from BITMAP_FILE_NAME and creates VID_LIST_FILE_NAME
void BitMapReader::read_bitmap(){
    ifstream fin;
    string filename = rel_path_to_target_dir + RTP::BITMAP_FILE_NAME;
    fin.open(filename.c_str());
    int vidnum,size;
    fin >> vidnum >> size;
    string term;
    unsigned long long tmp;
    unsigned long long *mask=(unsigned long long *) malloc (size*sizeof(unsigned long long));
    for (int i=0;i<size;i++){
        for (int j=0;j<64;j++){
            unsigned long long u = 1;
            u <<= j;
            mask[i]|=u;
        }
    }
    while (fin >> term){
        int flag=0;
        if (query.find(term)!=query.end())
            flag=1;
        for (int i=0;i<size;i++){
            fin>>tmp;
            if (flag)
                mask[i]&=tmp;
        }
    }
    fin.close();

    ofstream fout;
    fout.open(rel_path_to_target_dir + RTP::VID_LIST_FILE_NAME);
    for (int i=0; i<=size; i++){
        for (int j=0; j<64;j++){
            if (i*64+j<vidnum){
                unsigned long long u = 1;
                u <<= j;
                if (mask[i] & u)
                    fout << i*64+j <<" ";
            }
        }
    }
    fout.close();
}