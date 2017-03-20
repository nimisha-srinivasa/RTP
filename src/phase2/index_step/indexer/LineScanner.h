#ifndef _LINE_SCANNER_H_
#define _LINE_SCANNER_H_
#include <string>
#include "CorpusScanner.h"
#include "DocumentScanner.h"
#include "../../../utils/Stemmer.h"
//#include "coding.h"
#include "../../../utils/Serializer.h"

#define BUFLEN ((1<<20)*16)

class LineCorpusScanner : public CorpusScanner
{
protected:
    //struct file_buffer *infb_;
    serializer *ser;
    docid_t docnum_;
    bool init;
public:
    LineCorpusScanner(int fd);
    DocumentScanner *getScanner();
    void freeScanner(DocumentScanner *scanner);

    class LineDocumentScanner : public DocumentScanner
    {
        friend class LineCorpusScanner;
    private:
        LineCorpusScanner *parent_;
        char *linebuf_;
        char *docid_;
        char *title_;
        char *body_;
        docid_t curdoc_;
        char *saveptr_;
        stemmer *z;
        inline void clean_text(char *buffer) {
          for(char *p = buffer;*p;++p) {
            char c = *p;
            if (!isalnum(c)) *p = ' '; //TODO: decide whether to include '-'
          }
        }


        LineDocumentScanner(LineCorpusScanner* parent);
        ~LineDocumentScanner();

    public:
        bool nextDocument(uint64_t &fpos, docid_t &docid, std::string name, std::vector<std::string> &title_terms, std::vector<std::string> &body_terms);
        uint64_t get_file_pos();
    };
};


#endif
