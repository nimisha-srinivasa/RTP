#!/bin/bash

# $1 -> all queries

if [ "$#" -eq 1 ]; then
	./src/pre_process_query/pre_process.sh $1
	FILE_PATH='query.stem.clean'
 	
 	EXEC_NAME="python ./src/search_scripts/search_batch_job.py $FILE_PATH &> /dev/null"
	exec 3>&1 4>&2
	var=$( { time  $EXEC_NAME 1>&3 2>&4; } 2>&1 )  # Captures time only.
	exec 3>&- 4>&-

	echo "time taken for entire process:"$var

else
	echo "Illegal number of arguments. Should be of format: ./run_batch.sh <queries_file>"
fi