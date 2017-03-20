#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>

#include "XMLDataParser.h"

int main(int argc, char * argv[])
{
    auto return_code = EXIT_SUCCESS;
    if(argc < 3) {
        cout << "format: processor <data_file> <stem_file_name>" << endl;
        return EXIT_FAILURE;
    }
    try {
        string data_file_path = string(argv[1]);
        freopen(argv[2], "w", stdout);
        long chunk_size = 1024;
        std::ifstream is(data_file_path.c_str());

        if (!is)
            throw xmlpp::exception("Could not open file " + data_file_path);

        char buffer[chunk_size];
        const size_t buffer_size = sizeof(buffer) / sizeof(char);

        //Parse the file in chunks of "chunk_size" characters:
        XMLDataParser parser;
        parser.set_substitute_entities(true);
        do
        {
            std::memset(buffer, 0, buffer_size);
            is.read(buffer, buffer_size-1);
            if(is.gcount())
            {
                // We use Glib::ustring::ustring(InputIterator begin, InputIterator end)
                // instead of Glib::ustring::ustring( const char*, size_type ) because it
                // expects the length of the string in characters, not in bytes.
                Glib::ustring input(buffer, buffer+is.gcount());
                parser.parse_chunk(input);
            }
        }
        while(is);
        parser.finish_chunk_parsing();
    }
    catch(const xmlpp::exception& ex) {
        std::cerr << "Incremental parsing, libxml++ exception: " << ex.what() << std::endl;
        return_code = EXIT_FAILURE;
    }

    fclose(stdout);
    return return_code;
}