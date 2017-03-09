#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <math.h>
#include <sys/time.h>


#include "Phase2_ClusterSearcher.h"

using namespace std;

string rel_path_to_target_dir1 = "./";
typedef chrono::system_clock Clock;

vector<ScoreResult> Phase2_ClusterSearcher::searchCluster(string full_query_words, int cluster_id){
    init(); 

    cout << "************************Searching cluster " << cluster_id << " ********************************" << endl;
	full_query = full_query_words;
    setQueryLength();
    setQueryWords();

	rel_path_to_cluster = rel_path_to_target_dir1 + "cluster/" + to_string(cluster_id) + "/";
	string index_path = rel_path_to_cluster + "index";

	computeSearchFrag(index_path);
	computeVidList();

	// read reuse table: fid -> a list of vids for Option A
    read_index();
    
    // read forward reuse table: vid -> a list of fids for Option B
    read_forward();

    // calculate the positional information for each vid
    get_positional_info();

    //the result is writen to RESULT_FILE_NAME
    scoring();

    cout << "******************** Done Searching cluster " << cluster_id << " ********************************" << endl;
    return score_result;
}

void Phase2_ClusterSearcher::init(){
    num_words_in_query = 0;
    query_words.clear();
    frag_reuse_table.clear();
    vid_titlelen_hash.clear(); 
    search_frag.clear(); 
    vid_list.clear(); 
    doc_posting.clear(); 
    intersection_hash.clear(); 
    score_result.clear();
}

void Phase2_ClusterSearcher::setQueryLength(){
    int res = 0;
    if (full_query.length()!=0){
        res=1;
        for(int i=0;i<full_query.length();i++){
            if(full_query[i]==' ')
                res++;
        }
    }
    num_words_in_query = res;
}

void Phase2_ClusterSearcher::setQueryWords(){
    short counter = 0;
    query_words.clear();
    string word = "";
    for(short i=0;i<full_query.length();i++){
        if(full_query[i] == ' '){
            if(word.length() > 0){
                query_words.insert(word);
                word = "";
            }
        }else{
            word = word + full_query[i];
        }
    }
    //insert last word
    if(word.length() > 0){
        query_words.insert(word);
    }
}

void Phase2_ClusterSearcher::read_index(){

    //for timing
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

	ifstream fin;
    fin.open(rel_path_to_cluster + RTP::INDEX_FILE_NAME);

    // read frag_reuse_table
    frag_reuse_table.clear();
    int fid, size, vid, offset;

    while(fin >> fid) // deal with one line
    {
        if (fid == -99999) // index separator
            break;
        fin >> size;
        vector<ReuseTableInfo> v;
        v.clear();
        for (int i=0; i<size; i++)
        {
            fin >> vid >> offset;
            ReuseTableInfo r;
            r.vid = vid;
            r.offset = offset;
            v.push_back(r);
        }
        frag_reuse_table[fid] = v; // create one entry
    }
    int title_len;
    while(fin >> vid)
    {
        if (vid == -99999) // index separator
            break;
        fin >> title_len;
        vid_titlelen_hash[vid] = title_len;
    }
    fin.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: Reading index.txt=" << elapsed_seconds.count() << endl;
}

// read forward reuse table: vid -> a list of fids for Option B
void Phase2_ClusterSearcher::read_forward(){
    //for timing
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

	ifstream fin;
    fin.open(rel_path_to_cluster + RTP::FORWARD_FILE_NAME);
    int vid, size, fid, offset;
    int count=0;
    while(fin >> vid) // deal with one line
    {
        count++;
        fin >> size;
        vector<ForwardTableInfo> v;
        v.clear();
        for (int i=0; i<size; i++)
        {
            fin >> fid >> offset;
            ForwardTableInfo r;
            r.fid = fid;
            r.offset = offset;
            v.push_back(r);
        }
        forward_table[vid] = v; // create one entry for vid
    }
    fin.close();

    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: Reading forward.txt=" << elapsed_seconds.count() << endl;
}

void Phase2_ClusterSearcher::computeSearchFrag(string index_path){
	search_frag = findSearchFrags(index_path, full_query);
}

void Phase2_ClusterSearcher::computeVidList(){
    //for timing
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

	//generate vid from BITMAP_FILE_NAME
	ifstream fin;
    string filename = rel_path_to_cluster + RTP::BITMAP_FILE_NAME;
    fin.open(filename.c_str());
    int vidnum,size;
    fin >> vidnum >> size;
    string term;
    unsigned long long tmp;
    unsigned long long *mask=(unsigned long long *) malloc (size*sizeof(unsigned long long));
    for (int i=0;i<size;i++){
        for (int j=0;j<64;j++){
            mask[i]|=1<<j;
        }
    }
    while (fin >> term){
        int flag=0;
        if (query_words.find(term)!=query_words.end())
            flag=1;
        for (int i=0;i<size;i++){
            fin>>tmp;
            if (flag)
                mask[i]&=tmp;
        }
    }
    fin.close();
    vid_list.clear();
    for (int i=0; i<=size; i++){
        for (int j=0; j<64;j++){
            if (i*64+j<vidnum){
                unsigned long long u = 1;
                u <<= j;
                if (mask[i] & u)
                    vid_list.push_back(i*64+j);
            }
        }
    }
    
    free(mask);
    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: Generating vid_list from bitmap.txt=" << elapsed_seconds.count() << endl;
}

bool Phase2_ClusterSearcher::makeChoice(int k)
{
    vector<Fid_Occurence> v = search_frag[k];
    int f = v.size();
    int doc_sum = 0;
    int pos_sum = 0;
    for(int i=0;i<f;i++)
    {
        Fid_Occurence o = v[i];
        doc_sum += frag_reuse_table[o.fid].size();
        pos_sum = pos_sum + o.v_pos.size();
    }
    int u = doc_sum/f;
    double p = pos_sum*1.0/f;
    int m = vid_list.size();
    int frag_sum = 0;
    for(int i=0;i<m;i++)
    {
        frag_sum += forward_table[vid_list[i]].size();
        //frag_sum += forward_table[vid_list[i]].size() + f;
    }
    int pi = frag_sum/m;
    bool res = doc_sum*50<frag_sum*(1+(int)log2(f));
    //bool res = doc_sum*50<frag_sum;
    return res;
}

// calculate the positional information for each vid
// each vid: vector<set<int>>, each term corresponds to a set of positions
void Phase2_ClusterSearcher::get_positional_info(){
    //for timing
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    // begin timing
    struct timeval dwstart, dwend, dwstart_sort, dwend_sort, dwstartA, dwendA, dwstartC, dwendC;
    double dwtime = 0, dwtime_sort=0, dwtimeA=0, dwtimeC=0;
    int vid, fid, frag_offset;

    // init Option B's data structures
    vector<vector<int>> *e = new vector<vector<int>>[vid_list.size()];
    for (int i=0; i<vid_list.size(); i++)
        e[i].resize(num_words_in_query);

    // init Option A's data structures
    int term_number = num_words_in_query;
    doc_posting.clear();
    doc_posting.resize(term_number);

    // begin optionTest algorithm 
    double ratio;
    int y; 
    //cout << "Option C:" << endl;
    for (int k=0; k < num_words_in_query; k++) // each time, deal with one term
    {
        dwtime_sort = 0.0;
        vector<Vid_Occurence> current_posting; // doc posting for current term
        current_posting.clear();
		bool aa = makeChoice(k);

		//timing 
		gettimeofday(&dwstart, NULL);
		//timing 

        int f = search_frag[k].size();
        if(aa)
        {
            cout << "Chose option A" << endl;
            // Option A:
            // Step 1: Convert the fragment posting to document posting.
            //vector<Vid_Occurence> current_posting; // doc posting for current term
            //current_posting.clear();

            // algorithm for step 1: insert -> sort -> merge, this can merge the same vids
            // insert: insert each posting, the problem is that for different fragments, they might match the same vid, so that there will be multiple entries for the same vid. This duplication will be merged later.
            vector<Fid_Occurence> *vector_occ = &(search_frag[k]);
            for (int j=0; j<vector_occ->size(); j++) // each time, deal with one fragment
            {
                Fid_Occurence *o = &((*vector_occ)[j]); // the fragment occurence object
                int fid = o->fid; // fid
                vector<ReuseTableInfo> *vector_reuse = &(frag_reuse_table[fid]); // fragment reuse entry for fid
                for (int m=0; m<vector_reuse->size(); m++) // each time deal with a reuse version page for a given fragment
                {
                    int vid = (*vector_reuse)[m].vid;
                    int frag_offset = (*vector_reuse)[m].offset; // fragment's offset in the page
                    Vid_Occurence element;
                    current_posting.push_back(element); // firstly push back an empty element
                    Vid_Occurence *v_occur = &(current_posting[current_posting.size()-1]); // get the element just pushed in
                    v_occur->vid = vid;
                    v_occur->pos.clear();
                    for (int l=0; l<o->v_pos.size(); l++)
                        v_occur->pos.push_back(frag_offset+o->v_pos[l]);
                }
            }
            //timing
            gettimeofday(&dwstart_sort, NULL);
            //timing

            // sort: sort current_posting by its element's vid entry
            sort(current_posting.begin(), current_posting.end(), Vid_Occurence::compare);
            //gettimeofday(&dwend_sort, NULL);
            //dwtime_sort = 1000.0*(dwend_sort.tv_sec-dwstart_sort.tv_sec)+(dwend_sort.tv_usec-dwstart_sort.tv_usec)/1000.0;
            
            // merge: After sorting, elements with same vid must be adjacent. Merge them.
            vector<Vid_Occurence> *merge_result = &(doc_posting[k]);
            merge_result->clear();
            merge_result->push_back(current_posting[0]); // insert first element
            int pre_index = 0;
            for (int j=1; j<current_posting.size(); j++)
            {   //cout << "current_posting[j].vid=" << current_posting[j].vid << endl;
                if (current_posting[j].vid != (*merge_result)[pre_index].vid){
                    merge_result->push_back(current_posting[j]);
                    pre_index++;
                }
                else // merge
                {
                    vector<int>::iterator iter;
                    //cout << "current_posting[j] size=" << current_posting[j].pos.size() << endl;
                    //cout << "doc_posting[" << k << "][" << pre_index << "].pos size=" << doc_posting[k][pre_index].pos.size() << endl;
                    for (iter = current_posting[j].pos.begin(); iter != current_posting[j].pos.end(); iter++)
                        (*merge_result)[pre_index].pos.push_back(*iter);
                }
            }
            vector<Vid_Occurence> *current = &(doc_posting[k]);
            for (int j=0; j<current->size(); j++)
            {
                int v = (*current)[j].vid;
                //check if vid is present in vid_list before creating an entry in doc_posting for the vid
                vector<int>::iterator vid_iter;
                vid_iter = find (vid_list.begin(), vid_list.end(), v);
                if(vid_iter == vid_list.end())
                {    continue;   }

                set<int> *tmp = new set<int>;
                tmp->clear();
                if (intersection_hash.find(v)==intersection_hash.end()){
                    vector<set<int>> new_entry; new_entry.clear();
                    intersection_hash[v]=new_entry;
                }
                intersection_hash[v].push_back(*tmp);

                vector<int> *p = &((*current)[j].pos);
                vector<int>::iterator iter;
                for (iter = p->begin(); iter != p->end(); iter++){
                    intersection_hash[v][k].insert(*iter);
                }
            }
            //timing
	        gettimeofday(&dwend_sort, NULL);
            dwtime_sort = 1000.0*(dwend_sort.tv_sec-dwstart_sort.tv_sec)+(dwend_sort.tv_usec-dwstart_sort.tv_usec)/1000.0;
        	//timing
        }
        else
        {
            cout << "Chose option B" << endl;
            // Option B:
            vector<Fid_Occurence> *term_posting = &(search_frag[k]);
            //y=0; // avg number of frags in vid_list
            for (int i=0; i<vid_list.size(); i++) // each time, deal with one vid
            {
                vid = vid_list[i];
	
                //y += forward_table[vid].size();
                //vector<vector<int>> *e = new vector<vector<int>>(query_len);
                for (int j=0; j<forward_table[vid].size(); j++) // each time, deal with one fid in the vid
                {
                    fid = forward_table[vid][j].fid;
                    frag_offset = forward_table[vid][j].offset;
                    // for each fid, lookup the sorted term-fid postings
                    // binary search fid in term_posting O(log(n))
                    int index = find_index(term_posting, fid, 0, term_posting->size()-1);
                    if (index != -1){
                        vector<int>* vpos = &((*term_posting)[index].v_pos);
                        // for each position, insert it into vid_posting
                        for (int l=0; l<vpos->size(); l++)
                            (e[i])[k].push_back((*vpos)[l]+frag_offset);
                    }           
                }
 
                if (intersection_hash.find(vid)==intersection_hash.end()){
                    vector<set<int>> new_entry; new_entry.clear();
                    intersection_hash[vid]=new_entry;
                }

                set<int> *tmp = new set<int>;
                tmp->clear();
                intersection_hash[vid].push_back(*tmp);
                for (int x = 0; x < e[i][k].size(); x++){
                    intersection_hash[vid][k].insert(e[i][k][x]);
                }
                delete tmp;
            }
            //y /= vid_list.size();
        }

        //timing
        gettimeofday(&dwend, NULL);
        dwtime += 1000.0*(dwend.tv_sec-dwstart.tv_sec)+(dwend.tv_usec-dwstart.tv_usec)/1000.0 -dwtime_sort;
        //timing
    }
    cout << "Using clock_t, Finding positional info took: " << dwtime << " ms" << endl;
    delete [] e;
    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: Finding positional info took: " << elapsed_seconds.count() << endl;
}

void Phase2_ClusterSearcher::scoring(){
    //for timing
    chrono::time_point<Clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = Clock::now();  // start ticking

    clock_t start1 = clock();  // start ticking

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
    duration = (clock() - start1) / (double) CLOCKS_PER_SEC;  // added
    // added by Susen

    cout << "Using clock_t, Time taken for scoring the score_results without I/O: " << duration << endl;

    end = Clock::now();
    elapsed_seconds = end - start;
    cout << "Phase2 Cluster Search: scoring took:" << elapsed_seconds.count() << endl;
}

void Phase2_ClusterSearcher::score_page(int vid, vector<set<int>> &occur_terms)
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
    
    // old text_appear_score = 1
    //text_appear_score = 1; // text_appear_score=1 because in body we ensure that all text appears
    
    // new text_appear_score = log2(TF+1), TF is the min of all TFs.
    text_appear_score = log2(1.0+body_tf(vid, occur_terms, term_number));

    min_span=9999;
    cal_min_span(min_span, vid, occur_terms, term_number);
    proximity_score = (double)term_number/min_span; 
    body_score = 1.0/6.0*text_appear_score+5.0/6.0*proximity_score;

    score = 5.0/6.0*title_score + 1.0/6.0*body_score;
    new_entry.score = score;

    score_result.push_back(new_entry);
}

void Phase2_ClusterSearcher::cal_min_span_title(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number, int title_len)
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

// calculate the min span in body: sliding window algorithm O(n)
void Phase2_ClusterSearcher::cal_min_span(int &min_span, int vid, vector<set<int>> &occur_terms, int term_number)
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

int Phase2_ClusterSearcher::body_tf(int vid, vector<set<int>> &occur_terms, int term_number)
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
    delete [] tf;
    return min_tf;
}

int Phase2_ClusterSearcher::find_index(vector<Fid_Occurence> *term_posting, int fid, int start, int end)
{
    while(start<=end){
        int mid = (start+end)/2;
        if ((*term_posting)[mid].fid == fid)
            return mid;
        else if (fid < (*term_posting)[mid].fid)
            end = mid - 1;
        else
            start = mid + 1;
    }
    return -1;
}

void Phase2_ClusterSearcher::clean_text(char *buffer) {
  for(char *p = buffer;*p;++p) {
    char c = *p;
    if (!isalnum(c)) *p = ' '; //TODO: decide whether to include '-'
  }
  //stem_str(z,buffer); //also does to-lower, but leaves symbols in-place
}

uint64_t Phase2_ClusterSearcher::compute_term_id(std::string term) {
  const char *buf = term.c_str();
  const size_t buflen = term.size();

  return serializer::get_SHA1(buf,buflen);
}

vector<vector<Fid_Occurence>> Phase2_ClusterSearcher::findSearchFrags(string index_path, string full_query){
    
  Phase2_IndexSearcher searcher(index_path);
  vector<vector<Fid_Occurence>> search_frag;
  search_frag.clear();
  if (!searcher.is_open()) {
    cerr << "failed to open index '" << index_path << "'\n";
  }

  size_t linelen = 1024;
  char *line = new char[linelen];
  char *tok;
  std::string strline;
  int qid = 0;
  const char *qid_str = NULL;
  int i=0;
  
  strline = full_query;
  if (strline.size() < 1){cout << "strline size < 1 " << endl; return search_frag;}
  if (strline.size() >= linelen) {
  delete[] line;
  while (strline.size() >= linelen) {
    linelen *= 1.5;
  }

  line = new char[linelen];
  }

  strcpy(line,strline.c_str());
  

  strline = std::string(line);
  clean_text(line); //stem and remove non-alphanumerics
  
  std::vector<std::string> base_terms;
  std::vector<std::string> title_terms;
  std::vector<std::string> body_pair_terms;
  std::vector<std::string> title_pair_terms;

  for(tok = strtok(line, " \t\n"); tok; tok = strtok(NULL, " \t\n")) {
    base_terms.push_back(std::string(tok));
  }

  //first lookup base terms
  int termid = 0;
  int curTerm = 0;
  int prevTerm=-1;
  std::ofstream fout;
  
  fout.open(rel_path_to_cluster + "search_frag.txt", std::ofstream::app);
  for(std::vector<std::string>::const_iterator it = base_terms.begin(); it != base_terms.end(); ++it) 
  {
    std::string term = *it;
    curTerm = termid++;
    std::vector<std::pair<uint64_t,std::vector<uint32_t> > > posting;
    std::vector<std::pair<uint64_t,double> > features;
    if (!searcher.lookup_term(compute_term_id(term), posting)) {
      continue;
    }
    vector<Fid_Occurence> cur_v;
    cur_v.clear();
    search_frag.push_back(cur_v);
    
    for(std::vector<std::pair<uint64_t,std::vector<uint32_t> > >::const_iterator it2 = posting.begin(); it2 != posting.end(); ++it2) 
    {    
        vector<Fid_Occurence> *v;
        v = &(search_frag[curTerm]);
        Fid_Occurence occ;
        occ.fid = it2->first;
        
        fout << curTerm << " " << it2->first << " ";
        for(std::vector<uint32_t>::const_iterator it3 = it2->second.begin(); it3 != it2->second.end(); ++it3) {
          //std::cout << *it3 << " ";
          occ.v_pos.push_back(*it3);
          fout << *it3 << " ";
        }
        //std::cout << -1;
        fout << -1;
        //std::cout << "\n";
        fout << "\n";
        v->push_back(occ);
    }
    
  }
  return search_frag;
}