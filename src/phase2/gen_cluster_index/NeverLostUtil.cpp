#include "NeverLostUtil.h"
#include <iostream>

string NeverLostUtil::readFileContent(const char* filename){
    streampos size;
    char * memblock;
    ifstream file (filename, ios::in|ios::binary|ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        string content_str(memblock, (uint32_t)size);
        delete[] memblock;
        return content_str;
    } else {
    	cout<<"ERROR: Unable to open file " << string(filename) << endl;
    }
    return "";
}

void NeverLostUtil::writeFileContent(const char* filename, const string& content_str) {
	ofstream outfile(filename, ios::out | ios::binary);
	if (!outfile.is_open()) {
		cout<<"ERROR: Unable to open file " << string(filename)<<endl;
	}
    outfile.write((const char*)content_str.c_str(), content_str.size());
    outfile.close();
}

void NeverLostUtil::chunkContent(const string& content_str, const bool var, const uint32_t chunksize, vector<string> &chunks) {
    const uint32_t kFixed_chunk_size = chunksize;
    chunks.clear();
    if (var == false) {
        // fixed-sized chunking
        uint32_t cur_ptr = 0;
        while (cur_ptr < content_str.size()) {
            int cur_chunk_size = kFixed_chunk_size;
            if (cur_ptr + cur_chunk_size > content_str.size()) {
                cur_chunk_size = content_str.size() - cur_ptr;
            }
            string cur_chunk = content_str.substr(cur_ptr, cur_chunk_size);
            chunks.push_back(cur_chunk);
            cur_ptr += cur_chunk_size;
        }
    } else {
        // variable-sized chunking
        // DOING
        // Let's come on and do it!
        uint32_t avg_size;      // average block size in bytes
        uint32_t Tmin, Tmax;    // min & max threshold
        uint32_t D, Ddash;      // primary & backup divisor
        uint64_t new_start;     // position of the last breakpoint
        uint64_t backup_break;  // position of the last backup breakpoint
        uint64_t offset;     // buffer1's position in file
        struct rabin_state *s=NULL;  // for rabin fingerprinting
        struct rabin_state *inited = rabin_init(POLY, RABIN_WINDOW_SIZE);
        // uint64_t fix_boundary_size; // for fix boundary mode
        uint64_t rabinf;

        // for variable chunking, the argument chunksize is average chunking size
        avg_size = chunksize;
        Tmin = avg_size >> 1; // amount of bytes being skipped
        Tmax = avg_size << 2; // maximum block size
        D = avg_size - Tmin; // primary divisor indicates the real average scan length
        Ddash = D >> 1; // to avoid hard break, backup divisor is half of the primary divisor
        
        new_start = 0;
        backup_break = 0;
        offset = 0;
        s = rabin_init_from_state(inited, RABIN_WINDOW_SIZE);
        
        for (int p = 0; p < content_str.size(); p++) {
            rabinf = rabin_slide8(s, content_str[p]); // update rabin state
            offset = p;
            if (offset - new_start+ 1 < Tmin) continue; // not at min size yet
            if ((rabinf % Ddash) == Ddash -1 ) { // possible backup breakpoint
                backup_break = offset;
            }
            if ((rabinf % D) == D - 1) { 
                uint32_t cur_chunk_size = offset - new_start + 1;
                string cur_chunk = content_str.substr(new_start, cur_chunk_size);
                chunks.push_back(cur_chunk);
                
                new_start = offset +1;
                backup_break = 0;
                rabin_free(s);
                s = rabin_init_from_state(inited, RABIN_WINDOW_SIZE);
                continue;
            }
            if ((offset - new_start + 1) < Tmax) { // current byte didn't bring up a breakpoint
                continue;
            }
            if (backup_break != 0) { // use backup breakpoint if we found one
                uint32_t cur_chunk_size = backup_break - new_start + 1;
                string cur_chunk = content_str.substr(new_start, cur_chunk_size);
                chunks.push_back(cur_chunk);
                
                new_start = backup_break + 1;
                backup_break = 0;
                rabin_free(s);
                s = rabin_init_from_state(inited, RABIN_WINDOW_SIZE);
                for (int i = new_start; i<=offset; i++) {
                    rabin_slide8(s, content_str[i]);
                }
            } else { // impose a hard break;
                uint32_t cur_chunk_size = offset - new_start + 1;
                string cur_chunk = content_str.substr(new_start, cur_chunk_size);
                chunks.push_back(cur_chunk);
                
                new_start = offset + 1;
                backup_break = 0;
                rabin_free(s);
                s = rabin_init_from_state(inited, RABIN_WINDOW_SIZE);
            }
        } // for

        if (new_start <= offset) { // remaining data
            uint32_t cur_chunk_size = offset - new_start + 1;
            string cur_chunk = content_str.substr(new_start, cur_chunk_size);
            chunks.push_back(cur_chunk);
        }
        rabin_free(s);
    } // if var chunking
}


