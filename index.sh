#!/bin/bash

# $1 -> path_to_data_file
# $2 -> representative_choice (Optional. If not given, 1 is chosen by default.)

if [ "$#" -eq 2 ]; then
	./src/pre_process_data/pre_process.sh $1
	./target/gen_cluster ./target/data.stem.clean $2
	python ./src/phase2/gen_representative_index/gen_super.py ./target/data.stem.clean
	python ./src/phase2/index_step/runjob_index.py
	./src/phase1/index_step/run_phase1_index.sh $2

	cp src/phase1/search_step/lucene_search.jar ./target/

elif [ "$#" -eq 1 ]; then	#if no option is mentioned for <representative_choice>, then choose 1:Super_Longest
	REP_CHOICE='1'		
	./src/pre_process_data/pre_process.sh $1
	./target/gen_cluster ./target/data.stem.clean $REP_CHOICE
	python ./src/phase2/gen_representative_index/gen_super.py ./target/data.stem.clean
	python ./src/phase2/index_step/runjob_index.py
	./src/phase1/index_step/run_phase1_index.sh $REP_CHOICE

	cp src/phase1/search_step/lucene_search.jar ./target/
	
else
	echo "Illegal number of arguments. Should be of format: ./index.sh <path_to_data_file> <representative_choice>"
fi