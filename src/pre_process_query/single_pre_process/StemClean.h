#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>       /* for isupper, islower, tolower */

#include "../Stem.h"

#define TRUE 1
#define FALSE 0

#define LETTER(ch) (isupper(ch) || islower(ch))
static char * s;         /* buffer for words tobe stemmed */
#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */

char* appendChar(char* str, char c){
  size_t len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
    return str;
}

char* stemstring(struct stemmer * z, char* string1)
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
        result = appendChar(result,char_to_append);
      } 
   }
}

char* doStem(char* str){
  struct stemmer * z = create_stemmer();

   s = (char *) malloc(i_max + 1);
   char* result = stemstring(z, str);
   free(s);

   free_stemmer(z);

   return result;
}

bool isSpecialChar(char c){
  if((c>='a' && c<='z') || (c >='A' && c<='Z') || (c>='0' && c<='9'))
    return false;
  else 
    return true;
  
}

char* doClean(char* str){
  for(int i=0; i< strlen(str); i++){
    if(isSpecialChar(str[i]))
      str[i]=' ';
  }
  return str;
}


char* doStemClean(char* str)
{

  char* res = doStem(str);
  res = doClean(res);
  return res;
}