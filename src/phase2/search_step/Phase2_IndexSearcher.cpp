#include "Phase2_IndexSearcher.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
Phase2_IndexSearcher::Phase2_IndexSearcher(std::string fname, std::string corpus_path) {
    serializer ser_terms(fname + ".trm", serializer::READ);
    try {
        serializer ser_stats(fname + ".stats", serializer::READ);
        char tbuffer[1024];
        while(0 < ser_stats.read_string(tbuffer,1024)) { //read stat key
            uint64_t stat;
            if (!ser_stats.read_vbyte64(stat)) {
                fprintf(stderr, "fail to read stat value\n");    //read stat value
                throw "fail to read stat value";
            }
            stats[std::string(tbuffer)] = stat;
        }
        //printf("read %lu stats\n",stats.size());
    } catch (const char * s) {
        fprintf(stderr,"warning: no stats file, or invalid stats file\n");
    }
    scan_index = 0;
    prev_tid = 0;

    postings_ser = new serializer(fname + ".idx", serializer::READ);
    dict_ser = new serializer(fname + ".dct", serializer::READ);


    int nterms = 0;
    termid_t tid;
    while(ser_terms.read_uint64(tid)) { //read termid
        nterms++;

        termid_t term_offset;
        if (!ser_terms.read_uint64(term_offset)) {
            fprintf(stderr, "couln't read from index\n");
            throw "couln't read from index";
        }
        term_offsets[tid] = term_offset;
    }

    if (corpus_path.size() > 0) {
        corpus_ser = new serializer(corpus_path, serializer::READ);
    } else {
        corpus_ser = NULL;
    }
}

Phase2_IndexSearcher::~Phase2_IndexSearcher() {
    delete postings_ser;
    delete dict_ser;
    if (corpus_ser) delete corpus_ser;
}

bool Phase2_IndexSearcher::is_open() {
    return true;
}

bool Phase2_IndexSearcher::lookup_term(termid_t termid, std::vector<std::pair<docid_t,std::vector<offset_t> > > &posting) {
    scan_index = -1;
    if (!is_open()) {
        fprintf(stderr, "index not loaded\n");
        throw "index not loaded";
    }
    std::map<termid_t, uint64_t>::const_iterator it = term_offsets.find(termid);
    if (it == term_offsets.end()) {
        return false;
    }

    if (!postings_ser->seek(it->second)) {
        fprintf(stderr, "fail seek to %llu\n",it->second);
        return false;
    }

    uint32_t ndocs;
    if (!postings_ser->read_vbyte32(ndocs)) {
        fprintf(stderr,"fail 3\n");
        return false;
    }

    posting.clear();
    posting.reserve(ndocs);

    docid_t prev_docid = 0;
    for(uint32_t i=0; i<ndocs; ++i) {
        docid_t docid;
        if (!postings_ser->read_vbyte64(docid)) {
            fprintf(stderr,"fail 4\n");    //read docid
            return false;
        }
        docid += prev_docid;
        prev_docid = docid+1;
        posting.push_back(std::pair<docid_t,std::vector<uint32_t> >(docid,std::vector<uint32_t>()));
    }

    for(uint32_t i=0; i<ndocs; ++i) {
        uint32_t posting_length;
        if (0 != postings_ser->read_s9(posting_length)) {
            fprintf(stderr,"fail 5\n");    //read posting length
            return false;
        }
        posting_length += 1;
        posting[i].second.resize(posting_length); //allocate and initialize all the entries
    }

    for(uint32_t i=0; i<ndocs; ++i) {
        uint32_t posting_length = posting[i].second.size();
        uint32_t prev_pos = 0;
        for(uint32_t j = 0; j < posting_length; ++j) {
            uint32_t pos;
            int sr = postings_ser->read_s9(pos);
            if (sr) {
                fprintf(stderr,"fail 6. flag %d\n",sr);    //read posting pos
                return false;
            }
            pos += prev_pos;
            prev_pos = pos + 1;
            posting[i].second[j] = pos;
        }
    }
    return true;
}

bool Phase2_IndexSearcher::reset_scan() {
    if (!postings_ser->seek(0)) {
        fprintf(stderr, "fail term seek to 0\n");
        return false;
    }
    else if (!dict_ser->seek(0)) {
        fprintf(stderr, "fail dict seek to 0\n");
        return false;
    }
    else {
        scan_index = 0;
        prev_tid = 0;
        return true;
    }
}

bool Phase2_IndexSearcher::next_posting(std::string &term, termid_t &termid, std::vector<std::pair<docid_t,std::vector<offset_t> > > &posting) {

    if (scan_index < 0) {
        fprintf(stderr, "invalid scan state\n");
        throw "invalid scan state";
    } else if (scan_index >= (int64_t)term_offsets.size()) {
        return false;
    } else {
        if (!dict_ser->read_vbyte64(termid)) {
            fprintf(stderr, "fail to read dict termid\n");    //read termid
            throw "fail to read dict termid";
        }

        char tbuffer[1024];
        if (0 > dict_ser->read_string(tbuffer,1024)) {
            fprintf(stderr, "fail to read dict term\n");    //read ndocs
            throw "fail to read dict term\n";
        }
        termid += prev_tid;
        prev_tid = termid+1;
        term = std::string(tbuffer);

        uint32_t ndocs;
        if (!postings_ser->read_vbyte32(ndocs)) {
            fprintf(stderr,"fail 3\n");
            return false;
        }

        posting.clear();
        posting.reserve(ndocs);

        docid_t prev_docid = 0;
        for(uint32_t i=0; i<ndocs; ++i) {
            docid_t docid;
            if (!postings_ser->read_vbyte64(docid)) {
                fprintf(stderr,"fail 4\n");    //read docid
                return false;
            }
            docid += prev_docid;
            prev_docid = docid+1;
            posting.push_back(std::pair<docid_t,std::vector<uint32_t> >(docid,std::vector<uint32_t>()));
        }

        postings_ser->clear_s9();

        for(uint32_t i=0; i<ndocs; ++i) {
            uint32_t posting_length;
            if (0 != postings_ser->read_s9(posting_length)) {
                fprintf(stderr,"fail 5\n");    //read posting length
                return false;
            }
            posting_length += 1;
            posting[i].second.resize(posting_length); //allocate and initialize all the entries
        }

        for(uint32_t i=0; i<ndocs; ++i) {
            uint32_t posting_length = posting[i].second.size();
            uint32_t prev_pos = 0;
            for(uint32_t j = 0; j < posting_length; ++j) {
                uint32_t pos;
                int sr = postings_ser->read_s9(pos);
                if (sr) {
                    fprintf(stderr,"fail 6. flag %d\n",sr);    //read posting pos
                    return false;
                }
                pos += prev_pos;
                prev_pos = pos + 1;
                posting[i].second[j] = pos;
            }
        }
        scan_index++;
        return true;
    }
}

uint64_t Phase2_IndexSearcher::get_ntitle_terms() {
    return stats["ntitle_terms"];
}
uint64_t Phase2_IndexSearcher::get_ntitle_words() {
    return stats["ntitle_words"];
}
uint64_t Phase2_IndexSearcher::get_nbody_terms() {
    return stats["nbody_terms"];
}
uint64_t Phase2_IndexSearcher::get_nbody_words() {
    return stats["nbody_words"];
}
uint64_t Phase2_IndexSearcher::get_npostings() {
    return stats["npostings"];
}
uint64_t Phase2_IndexSearcher::get_nterms() {
    return stats["nterms"];
}
uint64_t Phase2_IndexSearcher::get_nwords() {
    return stats["nwords"];
}
uint64_t Phase2_IndexSearcher::get_ndocs() {
    return stats["ndocs"];
}

uint64_t Phase2_IndexSearcher::get_doc_fpos(docid_t docid) {
    return -1;
}

uint32_t Phase2_IndexSearcher::get_doc_title_length(docid_t docid) {
    return -1;
}
uint32_t Phase2_IndexSearcher::get_doc_body_length(docid_t docid) {
    return -1;
}

uint32_t Phase2_IndexSearcher::get_doc_title_terms(docid_t docid) {
    return -1;
}
uint32_t Phase2_IndexSearcher::get_doc_body_terms(docid_t docid) {
    return -1;
}

std::vector<uint64_t> Phase2_IndexSearcher::get_docids() {
    std::vector<docid_t> ret;
    return ret;
}

bool Phase2_IndexSearcher::get_raw_document(docid_t docid, std::string &doc) {
    return false;
}

