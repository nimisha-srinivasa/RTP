#!/bin/bash

#run phase2_index_search in a specific cluster

REL_PATH_TO_TARGET_DIR="../../../target/"
EXEC_NAME="phase2_index_search"
rm -rf search_frag.txt
cp $REL_PATH_TO_TARGET_DIR$EXEC_NAME ./
./$EXEC_NAME --index index --no-features --postings <<< $1 &> /dev/null
rm -rf $EXEC_NAME