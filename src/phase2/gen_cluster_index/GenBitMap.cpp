/** This program is used to generate the BITMAP_FILE from the ALL_VERSIONS_FILE for a cluster
 *  Author: Jiyu
 **/
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>

#include "../../Constants.h"

using namespace std;

unordered_map<string,set<int>> term_to_v;
int maxvid,num_of_bitmap;

int init(){
    term_to_v.clear();
    return 0;
}

int read(){
    ifstream fin;
    string filename = RTP::ALL_VERSIONS_FILE_NAME;
    fin.open(filename.c_str());
    string line;
    int vid = 0;
    while (getline(fin,line)){
        int las=0;
        int i=line.find_first_of(" \t\n",las);
        while (i != string::npos){
            if (i>las){
                string term = line.substr(las,i-las);
                if (term_to_v.find(term)==term_to_v.end()){
                    set<int> *tmp = new set<int>;
                    tmp->clear();
                    term_to_v[term]=*tmp;
                }
                term_to_v[term].insert(vid);
            }
            las = i+1;
            i=line.find_first_of(" \t\n",las);
        }
        vid++;
    }
    fin.close();
    maxvid=vid;
    return 0;
}

int write(){
    ofstream fout;
    string filename;
    filename = RTP::BITMAP_FILE_NAME;
    fout.open(filename.c_str());

    num_of_bitmap = (maxvid-1)/64+1;

    fout << maxvid << " " << num_of_bitmap << endl;

    int number_of_term = term_to_v.size();
    unsigned long long *bitmap = (unsigned long long *) malloc (num_of_bitmap * sizeof(unsigned long long));

    for (auto iter = term_to_v.begin();iter != term_to_v.end();iter++){
        for (int i=0;i<num_of_bitmap;i++)
            bitmap[i]=0;
        fout << iter->first << endl;
        for (auto it = iter->second.begin(); it != iter->second.end(); it++){
                unsigned long long u = 1;
                u <<= ((*it)%64);
                bitmap[(*it)/64]|=u;
        }
        fout << bitmap[0];
        for (int i=1; i<num_of_bitmap; i++){
            fout << " " << bitmap[i];
        }
        fout << endl;
    }

    fout.close();
    return 0;

}

//create BITMAP_FILE_NAME from ALL_VERSIONS_FILE_NAME
int main(int argc, char **argv)
{
    init();
    read();
    write();
    return 0;
}
