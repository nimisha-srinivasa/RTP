#include <stdlib.h>
#include <iostream>

#include "SingleQuerySearcher.h"
#include "../utils/Stemmer.h"

#define TRUE 1
#define FALSE 0

#define LETTER(ch) (isupper(ch) || islower(ch))
static char * s;         /* buffer for words tobe stemmed */
#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */

string rel_path_to_target = "./";

using namespace std;

SingleQuerySearcher::SingleQuerySearcher(){
  total_phase1_time_per_query = 0;
  total_phase2_time_per_query = 0;
  phase1_results.clear();
}

SingleQuerySearcher::~SingleQuerySearcher(){
  delete phase1_searcher;
  delete phase2_searcher;
}

void SingleQuerySearcher::pre_process_query(){
	full_query = doStemClean(full_query);
}

void SingleQuerySearcher::run_phase1_lucene_jar(){
  cout << "Phase1 Search..." << endl;
  string command = "rm -rf " + rel_path_to_target + "search_frag.txt";
	system(command.c_str());

	//generate SEARCH_FRAGMENT_FILE_NAME
	command = "java -jar " + rel_path_to_target + "lucene_search.jar " + full_query;
	system(command.c_str());
}


void SingleQuerySearcher::generate_phase1_results(){
  phase1_searcher = new Phase1_Searcher();
  phase1_results.clear();
  phase1_results = phase1_searcher->runSearch(full_query);
  cout << "Time taken for Phase1 Search without I/O:" << phase1_searcher->duration << endl;
  total_phase1_time_per_query = phase1_searcher->phase1_duration;
}

void SingleQuerySearcher::generate_phase1_results_again(){
  phase1_results.clear();
  phase1_results = phase1_searcher->runSearchAgain(full_query);
  cout << "Time taken for Phase1 Search without I/O:" << phase1_searcher->duration << endl;
  total_phase1_time_per_query = phase1_searcher->phase1_duration;
}

int SingleQuerySearcher::setQueryLength(){
    int res = 0;
    if (full_query.length()!=0){
        res=1;
        for(int i=0;i<full_query.length();i++){
            if(full_query[i]==' ')
                res++;
        }
    }
    return res;
}

void SingleQuerySearcher::run_phase2_search(){
  phase2_searcher = new Phase2_SearchRunner();
  cout << "Phase2 Search..." << endl;
  int num_words = setQueryLength();
  phase2_searcher->run_search(top_k, full_query, phase1_results);
  cout << "Complete Phase 2 Search took: " << phase2_searcher->phase2_duration << endl;
  total_phase2_time_per_query = phase2_searcher->phase2_duration;
}

void SingleQuerySearcher::run_phase2_search_again(){
  cout << "Phase2 Search..." << endl;
  int num_words = setQueryLength();
  phase2_searcher->run_search_again(top_k, full_query, phase1_results);
  cout << "Complete Phase 2 Search took: " << phase2_searcher->phase2_duration << endl;
  total_phase2_time_per_query = phase2_searcher->phase2_duration;
}

string SingleQuerySearcher::stemstring(struct stemmer * z, string str_to_stem)
{
  int j=0;
  string result="";
  //char* result=(char*) malloc(strlen(string1)*sizeof(char));  //assuming stemmed string is same or lesser in size than orig string
  while(TRUE)
   {  int ch = str_to_stem[j++];
      if (ch == 0) return result;
      if (LETTER(ch))
      {  int i = 0;
         while(TRUE)
         {  if (i == i_max)
            {  i_max += INC;
               s = (char*)realloc(s, i_max + 1);
            }
            ch = tolower(ch); //forces lower case

            s[i] = ch; i++;
            ch = str_to_stem[j++];
            if (!LETTER(ch)) {j--; break; }
         }
         s[Stemmer::stem(z, s, i - 1) + 1] = 0;
         /* the previous line calls the stemmer and uses its result to
            zero-terminate the string in s */
         result.append(s);
      }

      else{
        char char_to_append = (char) ch;
        result.push_back(char_to_append);
      } 
   }
}

string SingleQuerySearcher::doStem(string str){
    struct stemmer * z = Stemmer::create_stemmer();
    s = (char *) malloc(i_max + 1);
    string result = stemstring(z, str);
    free(s);
    Stemmer::free_stemmer(z);
    return result;
}

bool SingleQuerySearcher::isSpecialChar(char c){
  if((c>='a' && c<='z') || (c >='A' && c<='Z') || (c>='0' && c<='9'))
    return false;
  else 
    return true;
  
}

string SingleQuerySearcher::doClean(string str){
  for(int i=0; i< str.length(); i++){
    if(isSpecialChar(str[i]))
      str[i]=' ';
  }
  return str;
}


string SingleQuerySearcher::doStemClean(string str)
{

  string res = doStem(str);
  res = doClean(res);
  return res;
}

void SingleQuerySearcher::runSearch_without_preprocess(){
    run_phase1_lucene_jar();
    generate_phase1_results();
    run_phase2_search();
}

void SingleQuerySearcher::searchAgain_without_preprocess(){
    run_phase1_lucene_jar();
    generate_phase1_results_again();
    run_phase2_search_again();
}

void SingleQuerySearcher::runSearch(){
    pre_process_query();
    run_phase1_lucene_jar();
    generate_phase1_results();
    run_phase2_search();
}