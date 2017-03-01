#!/bin/bash

# $1 -> file_path
REL_PATH_TO_TARGET_DIR="./target/"
EXEC_NAME="pre_processor_data"

$REL_PATH_TO_TARGET_DIR$EXEC_NAME $1 $REL_PATH_TO_TARGET_DIR'data.stem'

python ./src/pre_process_data/clean_up.py $REL_PATH_TO_TARGET_DIR'data.stem' $REL_PATH_TO_TARGET_DIR'data.stem.clean'
rm $REL_PATH_TO_TARGET_DIR'data.stem'
