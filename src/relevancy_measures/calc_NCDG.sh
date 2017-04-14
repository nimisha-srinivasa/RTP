#!/bin/bash

# $1 -> query, $2 -> query length

num_docs=10
top_k=10

if [ -z "$1" ]; then
	echo "The query cannot be empty."
else
	./process_labeled_result.py --doc "results/$1/union_super/doc_id_union.txt" --rating "results/$1/union_super/rating.txt" --result "results/$1/baseline_www07/doc_id.txt,results/$1/baseline_sigir12/doc_id.txt,results/$1/twophase_latest/doc_id.txt,results/$1/twophase_longest/doc_id.txt,results/$1/super_latest/doc_id.txt,results/$1/super_longest/doc_id.txt"
	
fi