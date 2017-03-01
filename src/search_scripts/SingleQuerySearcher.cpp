#include <stdlib.h>
#include <iostream>

#include "SingleQuerySearcher.h"
#include "../pre_process_query/Stem.h"

#define TRUE 1
#define FALSE 0

#define LETTER(ch) (isupper(ch) || islower(ch))
static char * s;         /* buffer for words tobe stemmed */
#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */


using namespace std;

void SingleQuerySearcher::pre_process_query(){
	this->query = doStemClean(query);
}

void SingleQuerySearcher::run_phase1_lucene_jar(){
	system("rm -rf ./target/search_frag.txt");

	//generate SEARCH_FRAGMENT_FILE_NAME
	string command = "java -jar ../src/phase1/search_step/lucene_search.jar " + query;
  cout << "command is:" << command << endl;
	system(command.c_str());
}


void SingleQuerySearcher::generate_phase1_results(){
	string command = "./phase1_search " + to_string(num_words_in_query) + " " + this->query ;
	system(command.c_str());
}

void SingleQuerySearcher::run_phase2_search(){
	string command = "python ../src/phase2/search_step/run_phase2_search.py \"" + this->query + "\" " + to_string(this->num_words_in_query) + " " + to_string(this->top_k);
	system(command.c_str()); 
}

char* SingleQuerySearcher::appendChar(char* str, char c){
  size_t len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
    return str;
}

char* stemstring1(struct stemmer * z, char* string1)
{
  int j=0;
  char* result=(char*) malloc(strlen(string1)*sizeof(char));  //assuming stemmed string is same or lesser in size than orig string
  strcat(result, "");
  while(TRUE)
   {  int ch = string1[j++];
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
            ch = string1[j++];
            if (!LETTER(ch)) {j--; break; }
         }
         s[stem(z, s, i - 1) + 1] = 0;
         /* the previous line calls the stemmer and uses its result to
            zero-terminate the string in s */
         strcat(result,s);
      }

      else{
        char char_to_append = (char) ch;
        //result = appendChar(result,char_to_append);
      } 
   }
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
         s[stem(z, s, i - 1) + 1] = 0;
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
    struct stemmer * z = create_stemmer();
    s = (char *) malloc(i_max + 1);
    string result = stemstring(z, str);
    free(s);
    free_stemmer(z);
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