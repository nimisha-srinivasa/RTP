#include "LineScanner.h"
#include <cstring>
#include <iostream>


LineCorpusScanner::LineCorpusScanner(int fd) : init(false) {
    ser = new serializer(fd, serializer::READ,BUFLEN);
    docnum_ = 0;
}

DocumentScanner *LineCorpusScanner::getScanner() {
    if(init) return NULL;
    else {
        init = true;
        return new LineDocumentScanner(this);
    }
}
void LineCorpusScanner::freeScanner(DocumentScanner *scanner) {
    delete scanner;
}

LineCorpusScanner::LineDocumentScanner::LineDocumentScanner(LineCorpusScanner *parent) : parent_(parent) {
    linebuf_ = (char*)malloc(BUFLEN);
    z = Stemmer::create_stemmer();
}

uint64_t LineCorpusScanner::LineDocumentScanner::get_file_pos() {
    return parent_->ser->pos();
}

bool LineCorpusScanner::LineDocumentScanner::nextDocument(uint64_t &fpos, docid_t &docid, std::string doc_name, std::vector<std::string> &title_terms, std::vector<std::string> &body_terms) {
    char *tok;
    int len;
    fpos = parent_->ser->pos();
    //while(0 == (len = fbuffer_getline(parent_->infb_,linebuf_,BUFLEN-1))) {
    if(0 == (len = parent_->ser->read_line(linebuf_,BUFLEN-1))) {

        return false;
    }
    if (len < 0) return false;
    char strttt[] = " t \0";
    curdoc_ = parent_->docnum_++;
    docid_ = strttt;
    docid = curdoc_;
    title_ = strchr(strttt,'t');
    *title_ = '\0';
    ++title_;
    body_ = linebuf_;


    clean_text(title_);
    clean_text(body_);

    //return true;

    title_terms.clear();
    body_terms.clear();

    if (docid_ != NULL) {
        doc_name = std::string(docid_);
    }

    if (title_ != NULL) {
        tok = strtok_r(title_," ",&saveptr_);
        while(tok) {
            title_terms.push_back(std::string(tok));
            tok = strtok_r(NULL," ",&saveptr_);
        }
    }
    if (body_ != NULL) {
        tok = strtok_r(body_," ",&saveptr_);
        while(tok) {
            body_terms.push_back(std::string(tok));
            tok = strtok_r(NULL," ",&saveptr_);
        }
    }
    return true;
}

LineCorpusScanner::LineDocumentScanner::~LineDocumentScanner() {
    free(linebuf_);
    Stemmer::free_stemmer(z);
}
