#!/bin/bash
# $1 -> query
# $2 -> query_len
# $3 -> top_k

cd target/
./single_query_search $3 $2 $1
cd ..