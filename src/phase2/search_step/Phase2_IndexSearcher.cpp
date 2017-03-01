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
      if (!ser_stats.read_vbyte64(stat)) { fprintf(stderr, "fail to read stat value\n"); throw "fail to read stat value"; } //read stat value
      stats[std::string(tbuffer)] = stat;
    }
    //printf("read %lu stats\n",stats.size());
  } catch (const char * s) {
    fprintf(stderr,"warning: no stats file, or invalid stats file\n");
  }
  try {
    //serializer ser_docs(fname + ".docs", serializer::READ);
    //uint64_t docid;
    //std::map<docid_t, std::pair<uint64_t,std::pair<uint32_t,uint32_t> > > docs;
    //while(ser_docs.read_uint64(docid)) { //read docid
    //  uint64_t fpos;
    //  uint32_t title_length;
    //  uint32_t body_length;
    //  uint32_t title_terms;
    //  uint32_t body_terms;
    //  if (!ser_docs.read_uint64(fpos)) { fprintf(stderr, "fail to read doc fpos\n"); throw "fail to read doc fpos"; }
    //  if (!ser_docs.read_uint32(title_length)) { fprintf(stderr, "fail to read doc title length\n"); throw "fail to read doc title length"; }
    //  if (!ser_docs.read_uint32(body_length)) { fprintf(stderr, "fail to read doc body length\n"); throw "fail to read doc body length"; }
    //  if (!ser_docs.read_uint32(title_terms)) { fprintf(stderr, "fail to read doc title terms length\n"); throw "fail to read doc title terms length"; }
    //  if (!ser_docs.read_uint32(body_terms)) { fprintf(stderr, "fail to read doc body terms length\n"); throw "fail to read doc body terms length"; }
    //  //docs[docid] = std::tuple<uint64_t, uint32_t,uint32_t,uint32_t,uint32_t>(fpos,title_length,body_length,title_terms,body_terms);
    //}
    //printf("read %lu docs\n",docs.size());
  } catch (const char * s) {
    //commented by Qinghao
    //fprintf(stderr,"warning: no docs file, or invalid docs file\n");
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
    if (!ser_terms.read_uint64(term_offset)) { fprintf(stderr, "couln't read from index\n"); throw "couln't read from index"; }
    term_offsets[tid] = term_offset;
  }
  //printf("read %d terms\n",nterms);
  
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
  return true; //FIXME: hardcoded
}

bool Phase2_IndexSearcher::lookup_term(termid_t termid, std::vector<std::pair<docid_t,std::vector<offset_t> > > &posting) {
  scan_index = -1;
  if (!is_open()) {
    fprintf(stderr, "index not loaded\n"); throw "index not loaded";
  }
  std::map<termid_t, uint64_t>::const_iterator it = term_offsets.find(termid);
  if (it == term_offsets.end()) {
    return false;
  }

  if (!postings_ser->seek(it->second)) { fprintf(stderr, "fail seek to %llu\n",it->second); return false; }

  uint32_t ndocs;
  if (!postings_ser->read_vbyte32(ndocs)) { fprintf(stderr,"fail 3\n"); return false; }

  posting.clear();
  posting.reserve(ndocs);

  docid_t prev_docid = 0;
  for(uint32_t i=0;i<ndocs;++i) {
    docid_t docid;
    if (!postings_ser->read_vbyte64(docid)) { fprintf(stderr,"fail 4\n"); return false; } //read docid
    docid += prev_docid;
    prev_docid = docid+1;
    posting.push_back(std::pair<docid_t,std::vector<uint32_t> >(docid,std::vector<uint32_t>()));
  }

  for(uint32_t i=0;i<ndocs;++i) {
    uint32_t posting_length;
    if (0 != postings_ser->read_s9(posting_length)) { fprintf(stderr,"fail 5\n"); return false; } //read posting length
    posting_length += 1;
    posting[i].second.resize(posting_length); //allocate and initialize all the entries
  }

  for(uint32_t i=0;i<ndocs;++i) {
    uint32_t posting_length = posting[i].second.size();
    uint32_t prev_pos = 0;
    for(uint32_t j = 0; j < posting_length; ++j) {
      uint32_t pos;
      int sr = postings_ser->read_s9(pos);
      if (sr) { fprintf(stderr,"fail 6. flag %d\n",sr); return false; } //read posting pos
      pos += prev_pos;
      prev_pos = pos + 1;
      posting[i].second[j] = pos;
    }
  }
  //if (fb_postings->s9_i != 28) { fprintf(stderr,"fail 7. leftovers %d\n",fb_postings->s9_i); return false; }

  return true;
}

bool Phase2_IndexSearcher::reset_scan() {
  if (!postings_ser->seek(0)) { fprintf(stderr, "fail term seek to 0\n"); return false; }
  else if (!dict_ser->seek(0)) { fprintf(stderr, "fail dict seek to 0\n"); return false; }
  else { scan_index = 0; prev_tid = 0; return true; }
}

bool Phase2_IndexSearcher::next_posting(std::string &term, termid_t &termid, std::vector<std::pair<docid_t,std::vector<offset_t> > > &posting) {

  if (scan_index < 0) {
    fprintf(stderr, "invalid scan state\n"); throw "invalid scan state";
  } else if (scan_index >= (int64_t)term_offsets.size()) {
    return false;
  } else {
    if (!dict_ser->read_vbyte64(termid)) { fprintf(stderr, "fail to read dict termid\n"); throw "fail to read dict termid"; } //read termid

    char tbuffer[1024];
    if (0 > dict_ser->read_string(tbuffer,1024)) { fprintf(stderr, "fail to read dict term\n"); throw "fail to read dict term\n"; } //read ndocs
    termid += prev_tid;
    prev_tid = termid+1;
    //if (termid != term_offsets[scan_index]) {
    //  throw "termid mismatch ";
    //}
    term = std::string(tbuffer);

    uint32_t ndocs;
    if (!postings_ser->read_vbyte32(ndocs)) { fprintf(stderr,"fail 3\n"); return false; }

    posting.clear();
    posting.reserve(ndocs);

    docid_t prev_docid = 0;
    for(uint32_t i=0;i<ndocs;++i) {
      docid_t docid;
      if (!postings_ser->read_vbyte64(docid)) { fprintf(stderr,"fail 4\n"); return false; } //read docid
      docid += prev_docid;
      prev_docid = docid+1;
      posting.push_back(std::pair<docid_t,std::vector<uint32_t> >(docid,std::vector<uint32_t>()));
    }

    postings_ser->clear_s9();

    for(uint32_t i=0;i<ndocs;++i) {
      uint32_t posting_length;
      if (0 != postings_ser->read_s9(posting_length)) { fprintf(stderr,"fail 5\n"); return false; } //read posting length
      posting_length += 1;
      posting[i].second.resize(posting_length); //allocate and initialize all the entries
    }

    for(uint32_t i=0;i<ndocs;++i) {
      uint32_t posting_length = posting[i].second.size();
      uint32_t prev_pos = 0;
      for(uint32_t j = 0; j < posting_length; ++j) {
        uint32_t pos;
        int sr = postings_ser->read_s9(pos);
        if (sr) { fprintf(stderr,"fail 6. flag %d\n",sr); return false; } //read posting pos
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
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) { fprintf(stderr, "invalid docid"); throw "invalid docid"; }
  //return std::get<0>(docs[docid]);
  return -1;
}

uint32_t Phase2_IndexSearcher::get_doc_title_length(docid_t docid) {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) { fprintf(stderr, "invalid docid"); throw "invalid docid"; }
  //return std::get<1>(docs[docid]);
  return -1;
}
uint32_t Phase2_IndexSearcher::get_doc_body_length(docid_t docid) {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) { fprintf(stderr, "invalid docid"); throw "invalid docid"; }
  //return std::get<2>(docs[docid]);
  return -1;
}

uint32_t Phase2_IndexSearcher::get_doc_title_terms(docid_t docid) {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) { fprintf(stderr, "invalid docid"); throw "invalid docid"; }
  //return std::get<3>(docs[docid]);
  return -1;
}
uint32_t Phase2_IndexSearcher::get_doc_body_terms(docid_t docid) {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) { fprintf(stderr, "invalid docid"); throw "invalid docid"; }
  //return std::get<4>(docs[docid]);
  return -1;
}

std::vector<uint64_t> Phase2_IndexSearcher::get_docids() {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  std::vector<docid_t> ret;
  //for(std::map<docid_t, std::tuple<uint64_t, uint32_t,uint32_t,uint32_t,uint32_t> >::const_iterator it = docs.begin(); it != docs.end(); ++it) {
  //  ret.push_back(it->first);
  //}
  return ret;
}

bool Phase2_IndexSearcher::get_raw_document(docid_t docid, std::string &doc) {
  //if (docs.size() == 0) { fprintf(stderr, "no doc index\n"); throw "no doc index"; }
  //if (docs.find(docid) == docs.end()) return false;
  //if (!corpus_ser) { return false; }
  //char *t = new char[16 * (1 << 20)];
  //if (!corpus_ser->seek(get_doc_fpos(docid))) {
  //  fprintf(stderr, "bad corpus seek\n"); throw "bad corpus seek";
  //}
  //if (0 > corpus_ser->read_line(t,16 * (1 <<20)-1)) {
  //  fprintf(stderr, "bad corpus read\n"); throw "bad corpus read";
  //}
  //doc = std::string(t);
  //return true;
  return false;
}
