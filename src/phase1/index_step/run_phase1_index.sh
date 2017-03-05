#!/bin/bash

#This script is used to generate the index for the representative document and generate TITLE_LENGTH_FILE

cd ./target
rm -rf index
if [ $1 -eq 1 ]; then
	time java -jar ../src/phase1/index_step/lucene_index.jar ./longest.txt
	./gen_title_len longest.txt
elif [ $1 -eq 2 ]; then
	time java -jar ../src/phase1/index_step/lucene_index.jar ./latest.txt
	./gen_title_len latest.txt
fi
cd ..
