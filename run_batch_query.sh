#!/bin/bash

# $1 -> all_queries_file

TOP_K=1

if [ "$#" -eq 1 ]; then
	EXEC_NAME="batch_query_search"
	./src/pre_process_query/batch_pre_process/pre_process.sh $1
	FILE_PATH='query.stem.clean'
 	cd ./target/

 	./$EXEC_NAME $FILE_PATH $TOP_K

else
	echo "Illegal number of arguments. Should be of format: ./run_batch.sh <queries_file>"
fi