#!/bin/bash
# $1 -> file_path
REL_PATH_TO_TARGET_DIR="./target/"
EXEC_NAME="pre_processor_query"

$REL_PATH_TO_TARGET_DIR$EXEC_NAME $1 $REL_PATH_TO_TARGET_DIR'query.stem'

python ./src/pre_process_query/batch_pre_process/clean_up.py $REL_PATH_TO_TARGET_DIR'query.stem' $REL_PATH_TO_TARGET_DIR'query.stem.clean'
rm $REL_PATH_TO_TARGET_DIR'query.stem'
