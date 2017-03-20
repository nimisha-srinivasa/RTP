TARGET = ./target
SOURCE = ./src
CC = g++
CFLAGS = -std=c++0x
LFLAGS = -Wall $(DEBUG)
DEBUG = -g
PARSER_FLAG = -Wno-inconsistent-missing-override -Wno-deprecated -Wno-format -Wno-write-strings `pkg-config --cflags --libs libxml++-2.6 glibmm-2.4`
OPENSSL_FLAGS=-lssl -lcrypto

all: create_dir preprocess index_data search

create_dir:
	rm -rf $(TARGET)
	mkdir $(TARGET)

preprocess: pre_process_data pre_process_query

index_data: gen_cluster convert gen_title_len gen_bitmap phase2_index

search: single_query_search batch_query_search

pre_process_data: 
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/pre_process_data/, Page.cpp XMLDataParser.cpp PreProcessor.cpp) -o $(TARGET)/pre_processor_data $(PARSER_FLAG)

pre_process_query:
	gcc $(addprefix $(SOURCE)/pre_process_query/batch_pre_process/, Stem.c) -o $(TARGET)/pre_processor_query 

gen_cluster:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, NeverLostUtil.cpp  rabin_asm.S rabin.cpp CreateCluster.cpp  CreateClusterDriver.cpp) -o $(addprefix $(TARGET)/, gen_cluster)

convert:	
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, Convert.cpp ConvertDriver.cpp) -o $(addprefix $(TARGET)/, convert)

gen_title_len:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase1/index_step/, GenTitleLength.cpp) -o $(addprefix $(TARGET)/, gen_title_len)

gen_bitmap:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/phase2/gen_cluster_index/, GenBitMap.cpp) -o $(addprefix $(TARGET)/, gen_bitmap)

phase2_index:
	$(CC) $(CFLAGS) -o $(TARGET)/phase2_index $(addprefix $(SOURCE)/utils/, Stemmer.cpp Serializer.cpp) $(addprefix $(SOURCE)/phase2/index_step/indexer/, IndexGenerator.cpp LineScanner.cpp Coding.cpp) $(OPENSSL_FLAGS)

single_query_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, Vid_Occurence.cpp Fid_Occurence.cpp ScoreResult.cpp) $(addprefix $(SOURCE)/phase1/search_step/, Phase1_Searcher.cpp) $(addprefix $(SOURCE)/utils/, Stemmer.cpp Serializer.cpp) $(addprefix $(SOURCE)/phase2/search_step/, Phase2_IndexSearcher.cpp Phase2_ClusterSearcher.cpp Phase2_SearchRunner.cpp) $(addprefix $(SOURCE)/search_scripts/, SingleQuerySearcher.cpp SingleQuerySearchDriver.cpp) -o $(TARGET)/single_query_search $(PARSER_FLAG) $(OPENSSL_FLAGS)

batch_query_search:
	$(CC) $(CFLAGS) $(addprefix $(SOURCE)/data_structures/, Vid_Occurence.cpp Fid_Occurence.cpp ScoreResult.cpp) $(addprefix $(SOURCE)/phase1/search_step/, Phase1_Searcher.cpp) $(addprefix $(SOURCE)/utils/, Stemmer.cpp Serializer.cpp) $(addprefix $(SOURCE)/phase2/search_step/, Phase2_IndexSearcher.cpp Phase2_ClusterSearcher.cpp Phase2_SearchRunner.cpp) $(addprefix $(SOURCE)/search_scripts/, SingleQuerySearcher.cpp BatchQuerySearchDriver.cpp) -o $(TARGET)/batch_query_search $(PARSER_FLAG) $(OPENSSL_FLAGS)

clean:
	rm -rf $(TARGET)/
