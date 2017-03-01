#!/bin/bash

# $1 -> all queries

if [ "$#" -eq 1 ]; then
	./src/pre_process_query/pre_process.sh $1
	FILE_PATH='query.stem.clean'
 	
 	python ./src/search_scripts/search_batch_job.py $FILE_PATH
else
	echo "Illegal number of arguments. Should be of format: ./run_batch.sh <queries_file>"
fi