TARGET = ./target
SOURCE = ./src
CC = g++
CFLAGS = -std=c++0x
LFLAGS = -Wall $(DEBUG)
DEBUG = -g
PARSER_FLAG = -Wno-inconsistent-missing-override -Wno-deprecated -Wno-format -Wno-write-strings `pkg-config --cflags --libs libxml++-2.6 glibmm-2.4`
OPENSSL_FLAGS=-lssl -lcrypto

all: create_dir pre_process_data pre_process_query gen_cluster convert gen_title_len gen_bitmap phase1_search phase2_index phase2_index_search phase2_read_bitmap phase2_cluster_search phase2_search single_query_search batch_query_search

create_dir:
	rm -rf $(TARGET)
	mkdir $(TARGET)

pre_process_data: 
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/pre_process_data/, Page.cpp XMLDataParser.cpp PreProcessor.cpp) -o $(TARGET)/pre_processor_data $(PARSER_FLAG)

pre_process_query:
	gcc $(addprefix $(SOURCE)/pre_process_query/batch_pre_process/, stem.c) -o $(TARGET)/pre_processor_query 

gen_cluster:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, NeverLostUtil.cpp  rabin_asm.S rabin.cpp CreateCluster.cpp  CreateClusterDriver.cpp) -o $(addprefix $(TARGET)/, gen_cluster)

convert:	
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, Convert.cpp ConvertDriver.cpp) -o $(addprefix $(TARGET)/, convert)

gen_title_len:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase1/index_step/, GenTitleLength.cpp) -o $(addprefix $(TARGET)/, gen_title_len)

gen_bitmap:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, GenBitMap.cpp) -o $(addprefix $(TARGET)/, gen_bitmap)

phase1_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, ScoreResult.cpp) $(addprefix $(SOURCE)/phase1/search_step/, Phase1_Searcher.cpp Phase1_SearcherDriver.cpp) $(CFLAGS) -o $(TARGET)/phase1_search

phase2_index:
	$(CC) -o $(TARGET)/phase2_index $(addprefix $(SOURCE)/utils/, Stemmer.cpp Serializer.cpp) $(addprefix $(SOURCE)/phase2/index_step/indexer/, IndexGenerator.cpp LineScanner.cpp Coding.cpp) $(OPENSSL_FLAGS)

phase2_index_search:
	$(CC) -o $(TARGET)/phase2_index_search $(addprefix $(SOURCE)/utils/, Stemmer.cpp Serializer.cpp) $(addprefix $(SOURCE)/phase2/search_step/, Phase2_IndexSearcher.cpp Phase2_IndexSearcherDriver.cpp) $(OPENSSL_FLAGS)

phase2_read_bitmap:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/search_step/, BitMapReader.cpp BitMapReaderDriver.cpp) $(CFLAGS) -o $(TARGET)/phase2_read_bitmap

phase2_cluster_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, Vid_Occurence.cpp Fid_Occurence.cpp ScoreResult.cpp) $(addprefix $(SOURCE)/phase2/search_step/, Phase2_Searcher.cpp Phase2_SearcherDriver.cpp) $(CFLAGS) -o $(TARGET)/phase2_cluster_search

phase2_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, ScoreResult.cpp) $(addprefix $(SOURCE)/phase2/search_step/, Phase2_SearchRunner.cpp) -o $(TARGET)/phase2_search $(PARSER_FLAG)

single_query_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, ScoreResult.cpp) $(addprefix $(SOURCE)/phase1/search_step/, Phase1_Searcher.cpp) $(addprefix $(SOURCE)/search_scripts/, SingleQuerySearcher.cpp SingleQuerySearchDriver.cpp) -o $(TARGET)/single_query_search $(PARSER_FLAG)

batch_query_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, ScoreResult.cpp) $(addprefix $(SOURCE)/phase1/search_step/, Phase1_Searcher.cpp) $(addprefix $(SOURCE)/search_scripts/, SingleQuerySearcher.cpp BatchQuerySearchDriver.cpp) -o $(TARGET)/batch_query_search $(PARSER_FLAG)

clean:
	rm -rf $(TARGET)/
