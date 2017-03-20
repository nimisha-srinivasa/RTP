#ifndef _DOCUMENT_SCANNER_H_
#define _DOCUMENT_SCANNER_H_
#include <string>
#include <vector>
#include <stdint.h>

typedef uint64_t docid_t;
typedef uint64_t termid_t;

class DocumentScanner
{
public:
    virtual bool nextDocument(uint64_t &fpos, docid_t &docid, std::string doc_name, std::vector<std::string> &title_terms, std::vector<std::string> &body_terms) = 0;
    virtual ~DocumentScanner() {};
};

#endif
