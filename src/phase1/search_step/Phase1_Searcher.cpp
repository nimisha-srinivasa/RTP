#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <math.h>

#include "Phase1_Searcher.h"

using namespace std;

string rel_path_to_target_dir = "./";
typedef chrono::system_clock Clock;

void Phase1_Searcher::splitFullQuery(){
    short counter = 0;
    query_words_arr.push_back("");
    for(short i=0;i<full_query.length();i++){
        if(full_query[i] == ' '){
            query_words_arr.push_back("");
            counter++;
            i++;
        }
        query_words_arr[counter] += full_query[i];
    }
}

void Phase1_Searcher::setQueryLength(){
    int res = 0;
    if (full_query.length()!=0){
        res=1;
        for(int i=0;i<full_query.length();i++){
            if(full_query[i]==' ')
                res++;
        }
    }
    query_len = res;
}

void Phase1_Searcher::init()
{
    vid_titlelen_hash.clear();
    intersection_hash.clear();
    super_index.clear();
    splitFullQuery();
    setQueryLength();
    duration = 0;
    phase1_duration = 0;
}

void Phase1_Searcher::re_init(){
    intersection_hash.clear();
    search_frag.clear();
    splitFullQuery();
    setQueryLength();
    duration = 0;
    phase1_duration = 0;
}

//query and query_len has to be set already
vector<ScoreResult> Phase1_Searcher::runSearch(string query){

    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    full_query = query;
    init();
    splitFullQuery();
    setQueryLength();
    read_title_len();
    read_search_frag();
    read_superinfo();
    intersection();
    scoring();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Complete Phase1 Search took:" << elapsed_seconds.count() << endl;
    phase1_duration = elapsed_seconds.count();

    return score_result;
}
 
//query and query_len has to be set already
vector<ScoreResult> Phase1_Searcher::runSearchAgain(string query){
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    full_query = query;
    //no need to load super_index and title_len.txt this time
    re_init();
    splitFullQuery();
    setQueryLength();
    read_search_frag();
    intersection();
    scoring();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Complete Phase1 Search took:" << elapsed_seconds.count() << endl;
    phase1_duration = elapsed_seconds.count();

    return score_result;
}

void Phase1_Searcher::read_title_len(){

    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

	ifstream fin;
    fin.open(rel_path_to_target_dir + RTP::TITLE_LEN_FILE_NAME);

    // read frag_reuse_table
    frag_reuse_table.clear();
    int vid;
    int title_len;
    while(fin >> vid)
    {
        if (vid == -99999) // index separator
            break;
        fin >> title_len;
        vid_titlelen_hash[vid] = title_len;

        vector<ReuseTableInfo> v;
        v.clear();
        ReuseTableInfo r;
        r.vid = vid;
        r.offset = 0;
        v.push_back(r);
        frag_reuse_table[vid] = v;
    }
    fin.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase1 Search - read title_len.txt: " << elapsed_seconds.count() << endl;
}

void Phase1_Searcher::validate()
{
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    ofstream fout;
    fout.open(rel_path_to_target_dir + RTP::VALIDATION_FILE_NAME);
    for (int i=0; i<search_frag.size(); i++)
    {
        vector<Fid_Occurence> v = search_frag[i];
        for (int j=0; j<v.size(); j++)
        {
            Fid_Occurence o = v[j];
            fout << "term " << i << " Occ " << j << " fid " << o.fid << " pos ";
            for (int k=0; k<o.v_pos.size(); k++)
                fout << o.v_pos[k] << " ";
            fout << endl;
        }
    }
    fout.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase1 Search - output validate.txt:" << elapsed_seconds.count() << endl;
}

void Phase1_Searcher::read_search_frag()
{
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    ifstream fin;
    string search_frag_path = rel_path_to_target_dir + RTP::SEARCH_FRAGMENT_FILE_NAME;
    fin.open(search_frag_path);
    search_frag.clear();

    int fid, pos, term;
    int previous_term=-1;
    while(fin >> term) // each time deal with an occurence
    {
        if (term != previous_term) // meet a new term
        {
            if (term != previous_term+1)
            {
                cout << " missing frags for term " << previous_term+1 << endl;
                exit(1);
            }
            vector<Fid_Occurence> cur_v;
            cur_v.clear();
            search_frag.push_back(cur_v);
        }
        vector<Fid_Occurence> *v;
        v = &(search_frag[term]);
        Fid_Occurence occ;
        fin >> occ.fid;
        occ.v_pos.clear();
        // push_back all the positions
        while((fin >> pos) && (pos != -1))
        {
            occ.v_pos.push_back(pos);
        }
        // push back an occurence
        v->push_back(occ);
        previous_term = term;
    }
    if (previous_term != query_len-1)
    {
        cout << "missing frags for term " << query_len-1 << endl;
        exit(1);
    }
    //validate();
    fin.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase1 Search - read search_frag.txt:" << elapsed_seconds.count() << endl;
}

void Phase1_Searcher::read_superinfo()
{
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    ifstream fin;
    int vid, did;
    //read superpage TF
    fin.open(rel_path_to_target_dir + RTP::SUPER_INDEX_FILE_NAME);
    string line, word;
    while(getline(fin, line))
    {
        istringstream iss(line);
        iss >> word;
        double tf;
        char c; // ":"
        unordered_map<int, double> m;
        m.clear();
        while((iss >> did >> c >> tf))
        {
            //cout << did << " " << tf << endl;
            m[did] = tf;
        }
        super_index[word] = m;
    }
    fin.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    //cout << "Phase1 Search - read super_index:" << elapsed_seconds.count() << endl;
}

void Phase1_Searcher::intersection()
{
    // added by Susen
    start = clock();  // start ticking
    // added by Susen

    int term_number = search_frag.size();
    // init our intersection data structure "intersection_hash" for the first term
    vector<Fid_Occurence> v_occ;
    v_occ = search_frag[0];
    int i, j, m;
    for (i=0; i<v_occ.size(); i++) // each time, deal with one fragment occurence
    {
        Fid_Occurence *o = &(v_occ[i]); // the fragment occurence object
        int fid = o->fid; // fid
        vector<ReuseTableInfo> v_reuse = frag_reuse_table[fid]; // fragment reuse entry for fid
        for (j=0; j<v_reuse.size(); j++) // each time deal with a reuse version page for a given fragment
        {
            int vid = v_reuse[j].vid;
            int frag_offset = v_reuse[j].offset; // fragment's offset in the page
            unordered_map<int, vector<set<int>>>::iterator iter = intersection_hash.find(vid);
            vector<set<int>> *p; // pointer of the value of intersection_hash[vid]
            if (iter != intersection_hash.end()) // find vid saved already
            {
                p = &(iter->second);
            }
            else// not found
            {
                vector<set<int>> new_entry; new_entry.clear();
                set<int> t; t.clear();
                new_entry.push_back(t);
                intersection_hash[vid] = new_entry; // insert new_entry
                p = &(intersection_hash[vid]);
            }
            if (p->size()<1) // shouldn't be smaller than 1
            {
                cout << "wrong! p->size()<1, vid=" << vid  << endl;
                exit(1);
            }
            set<int> *pos_set = &((*p)[0]);
            for (m=0; m<(o->v_pos).size(); m++) // loop for each position in the fragment occurence
                pos_set->insert(o->v_pos[m]+frag_offset); // insert into the set; thus if duplicate, will not save
        }
    }
    // do for the following terms: term1, term2, ..., term(n-1)
    int term;
    for (term = 1; term<term_number; term++) // each time, deal with one term
    {
        unordered_map<int, vector<set<int>>> temp_intersection_hash;
        temp_intersection_hash.clear();

        v_occ = search_frag[term];
        for (i=0; i<v_occ.size(); i++) // each time, deal with one fragment occurence
        {
            Fid_Occurence *o = &(v_occ[i]);
            int fid = o->fid;
            vector<ReuseTableInfo> v_reuse = frag_reuse_table[fid];
            for (j=0; j<v_reuse.size(); j++)
            {
                int vid = v_reuse[j].vid;
                int frag_offset = v_reuse[j].offset;
                unordered_map<int, vector<set<int>>>::iterator iter = intersection_hash.find(vid);
                vector<set<int>> *p;
                if (iter != intersection_hash.end()) // find
                {
                    p = &(iter->second);
                }
                else // not found, disregard this vid because a page must contain all the previous terms
                    continue; 
                // check if we have this entry in temp_intersection_hash
                unordered_map<int, vector<set<int>>>::iterator iter_temp = temp_intersection_hash.find(vid);
                if (iter_temp == temp_intersection_hash.end()) // not find, copy the entry from intersection_hash and create a new set for this term
                {
                    vector<set<int>> cp_entry = *p;
                    set<int> t; t.clear();
                    cp_entry.push_back(t);
                    temp_intersection_hash[vid] = cp_entry;
                }
                // if find, don't need to do anything because the new set for this term should be created before
                p = &(temp_intersection_hash[vid]);
                if (p->size()<term+1) // shouldn't be smaller than term+1
                {
                    cout << "wrong! p->size()<term+1, vid=" << vid<< endl;
                    exit(1);
                }
                set<int> *pos_set = &((*p)[term]);
                for (m=0; m<(o->v_pos).size(); m++)
                {
                    pos_set->insert((o->v_pos)[m]+frag_offset);
                }
            }
        }
        intersection_hash = temp_intersection_hash; // update the new "intersection_hash"
    }

    //print_intersection(term_number);
    // added by Susen
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;
    start = clock();
    // added by Susen
}

// Print the information after intersection step. 
// Term_number specified how many terms in the query.
void Phase1_Searcher::print_intersection(int term_number)
{
    ofstream fout;
    fout.open(rel_path_to_target_dir + "output2.txt");
    //fout << intersection_hash.size() << endl;
    unordered_map<int, vector<set<int>>>::iterator iter;
    for (iter = intersection_hash.begin(); iter != intersection_hash.end(); iter++)
    {
        fout << "VID" << iter->first;
        for (int i=0; i<term_number; i++)
        {
            fout << " term" << i;
            set<int> s = (iter->second)[i];
            set<int>::iterator iter2;
            for (iter2 = s.begin(); iter2 != s.end(); iter2++)
                fout << " " << *iter2;
        }
        fout << endl;
    }
    fout.close();
}

void Phase1_Searcher::scoring()
{
    // go through the intersection_hash and score each entry
    score_result.clear();
    unordered_map<int, vector<set<int>>>::iterator iter;
    for (iter = intersection_hash.begin(); iter != intersection_hash.end(); iter++)
    {
        score_page(iter->first, iter->second);
    }
    // sort score_result
    sort(score_result.begin(), score_result.end(), ScoreResult::compare);
    // added by Susen
    duration += (clock() - start) / (double) CLOCKS_PER_SEC;  // added
    // added by Susen

    /*chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    ofstream fout;
    fout.open(rel_path_to_target_dir + RTP::RESULT_FILE_NAME);
    for (int i=0; i<score_result.size(); i++)
        fout << score_result[i].vid << " " << score_result[i].score<< endl;
    fout.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase1 Search - write result.txt:" << elapsed_seconds.count() << endl;*/
}

void Phase1_Searcher::score_page(int vid, vector<set<int>> &occur_terms)
{
    ScoreResult new_entry;
    new_entry.vid = vid;
    double score;
    // now we need to calculate the score for vid
    double title_score, body_score;
    double text_appear_score, proximity_score;
    int term_number = search_frag.size();
    
    // calculate for title:
    int title_len = vid_titlelen_hash[vid];
    int appear=0;
    int min_span=9999;
    for (int i=0; i<term_number; i++)
    {
        bool if_cal_appear = false; // flag
        set<int>::iterator iter_set;
        for (iter_set = occur_terms[i].begin(); iter_set != occur_terms[i].end(); iter_set++)
        {
            if (*iter_set < title_len && if_cal_appear == false)
            {
                appear++;
                if_cal_appear = true;
            }
        }
    }
    text_appear_score = (double)appear / term_number;
    if (appear != term_number) // if not all terms occur in title, no proximity_score
        proximity_score = 0;
    else
    {
        min_span=9999;
        cal_min_span_title(min_span, vid, occur_terms, term_number, title_len);
        if (min_span < term_number)
        {
            cout << "min_span < term_number for title" << " something is wrong for cal_min_span_title" << endl;
            exit(1);
        }
        proximity_score = (double)term_number/min_span;
    }
    title_score = 1.0/6.0*text_appear_score + 5.0/6.0*proximity_score;
    
    // calculate for body:
    
    // old old text_appear_score = 1
    //text_appear_score = 1; // text_appear_score=1 because in body we ensure that all text appears
    
    // old text_appear_score = log2(TF+1), TF is the min of all TFs.
    //text_appear_score = log2(1.0+body_tf(vid, occur_terms, term_number));
    
    // new super-version text_appear_score = log2(tf_avg+1), tf_avg is the min of all tf_avgs.
    text_appear_score = log2(1.0+body_tf_avg(vid, term_number));
    
    min_span=9999;
    cal_min_span(min_span, vid, occur_terms, term_number);
    proximity_score = (double)term_number/min_span; 
    body_score = 1.0/6.0*text_appear_score+5.0/6.0*proximity_score;

    score = 5.0/6.0*title_score + 1.0/6.0*body_score;
    new_entry.score = score;

    score_result.push_back(new_entry);
}

/* ------- HELPER METHODS FOR SCORING ---------------------- */
// calculate the min span in title: sliding window algorithm O(n)
void Phase1_Searcher::cal_min_span_title(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number, int title_len)
{
    // cur_pos saves the current focus of terms' positions in the algorithm
    int *cur_pos = new int[term_number];
    int *cur_index = new int[term_number];
    if (term_number != occur_terms.size())
    {
        cout << "error in cal_min_span_title: term_number != occur_terms.size()" << endl;
        exit(1);
    }
    // copy occur_terms to vec_occ because set cannot be sorted
    vector<vector<int>> vec_occ(term_number);
    for (int i=0; i<term_number; i++)
    {
        vec_occ[i].clear();
        copy(occur_terms[i].begin(), occur_terms[i].end(), back_inserter(vec_occ[i]));
        sort(vec_occ[i].begin(), vec_occ[i].end()); // sort each term's positions from small to large
        cur_pos[i] = vec_occ[i][0]; // let cur_pos focus on the first occurence of each term
        cur_index[i] = 0;
    }
    // each time check if need to update min_span, move min_pos to next to check the next loop
    while(1)
    {
        int min, max, min_index, max_index;
        min = 99999, max = -99999;
        // every time, min is the smallest position among all current term position focus, max is the largest one
        for (int i=0; i<term_number; i++)
        {
            if (min > cur_pos[i]) {min=cur_pos[i]; min_index=i;}
            if (max < cur_pos[i]) {max=cur_pos[i]; max_index=i;}
        }
        // if max is not in title, terminate the loop
        if (max >= title_len)
            break;
        if (max - min + 1 < term_number) // double check: If true, something is wrong with our index.
        {
            cout << "error in cal_min_span: max-min+1<term_number" << endl;
            cout << "vid=" << vid << " min=" << min << " max=" << max << endl;
            exit(1);
        }

        if (min_span > max - min + 1) // check if we need to update min_span
            min_span = max - min + 1;
        // termination condition is cur_index[min_index] is the last one
        if (cur_index[min_index] == vec_occ[min_index].size() - 1)
            break;
        // otherwise, find the next one
        cur_index[min_index]++;
        cur_pos[min_index] = vec_occ[min_index][cur_index[min_index]];
    }
    delete []cur_pos;
    delete []cur_index;
}

/*
int Phase1_Searcher::body_tf(int vid, vector<set<int>> &occur_terms, int term_number)
{
    if (term_number != occur_terms.size())
    {
        cout << "error in body_tf: term_number != occur_terms.size()" << endl;
        exit(1);
    }
    int *tf = new int[term_number];
    int min_tf = 99999;
    for (int i=0; i<term_number; i++)
    {
        tf[i] = occur_terms[i].size();
        if (min_tf > tf[i])
            min_tf = tf[i];
    }
    return min_tf;
}
*/

// calculate the min span in body: sliding window algorithm O(n)
void Phase1_Searcher::cal_min_span(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number)
{
    // cur_pos saves the current focus of terms' positions in the algorithm
    int *cur_pos = new int[term_number];
    int *cur_index = new int[term_number];
    if (term_number != occur_terms.size())
    {
        cout << "error in cal_min_span: term_number != occur_terms.size()" << endl;
        exit(1);
    }
    // copy occur_terms to vec_occ because set cannot be sorted
    vector<vector<int>> vec_occ(term_number);
    for (int i=0; i<term_number; i++)
    {
        vec_occ[i].clear();
        copy(occur_terms[i].begin(), occur_terms[i].end(), back_inserter(vec_occ[i]));
        sort(vec_occ[i].begin(), vec_occ[i].end()); // sort each term's positions from small to large
        cur_pos[i] = vec_occ[i][0]; // let cur_pos focus on the first occurence of each term
        cur_index[i] = 0;
    }
    // each time check if need to update min_span, move min_pos to next to check the next loop
    while(1)
    {
        int min, max, min_index, max_index;
        min = 99999, max = -99999;
        for (int i=0; i<term_number; i++)
        {
            if (min > cur_pos[i]) {min=cur_pos[i]; min_index=i;}
            if (max < cur_pos[i]) {max=cur_pos[i]; max_index=i;}
        }
        if (max - min + 1 < term_number) // double check: If true, something is wrong with our index.
        {
            cout << "error in cal_min_span: max-min+1<term_number" << endl;
            cout << "vid=" << vid << " min=" << min << " max=" << max << endl;
            exit(1);
        }

        if (min_span > max - min + 1) // check if we need to update min_span
            min_span = max - min + 1;
        // termination condition is cur_index[min_index] is the last one
        if (cur_index[min_index] == vec_occ[min_index].size() - 1)
            break;
        // otherwise, find the next one
        cur_index[min_index]++;
        cur_pos[min_index] = vec_occ[min_index][cur_index[min_index]];
    }
    delete []cur_pos;
    delete []cur_index;
}

double Phase1_Searcher::body_tf_avg(int vid, int term_number)
{
    int did = vid;
    double *tf_avg = new double[term_number];
    double min_tf = 99999;
    for (int i=0; i<term_number; i++)
    {
        tf_avg[i] = super_index[query_words_arr[i]][did];
        //cout << vid+3532308 << " " << did << " "<< i << " " << tf_avg[i] << endl;
        if (min_tf > tf_avg[i])
            min_tf = tf_avg[i];
    }
    delete [] tf_avg;
    return min_tf;
}