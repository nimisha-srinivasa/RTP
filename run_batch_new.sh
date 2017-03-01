#!/bin/bash

# $1 -> all queries
TOP_K=5
if [ "$#" -eq 1 ]; then
	EXEC_NAME="batch_query_search"
	./src/pre_process_query/batch_pre_process/pre_process.sh $1
	FILE_PATH='query.stem.clean'
 	cd ./target/

 	exec 3>&1 4>&2
	var=$( { time  ./$EXEC_NAME $FILE_PATH $TOP_K 1>&3 2>&4; } 2>&1 )  # Captures time only.
	exec 3>&- 4>&-

	echo "time taken for entire process:"$var
 	#./$EXEC_NAME $FILE_PATH $TOP_K

else
	echo "Illegal number of arguments. Should be of format: ./my_batch.sh <queries_file>"
fi