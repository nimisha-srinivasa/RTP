#!/bin/bash

# $1 -> query
# $2 -> top_k

cd target/
./single_query_search $2 $1
cd ..