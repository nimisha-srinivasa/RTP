#!/bin/bash

# $1 -> query, $2 -> query length
top_k=5

if [ -z "$1" ]; then
    echo "The query cannot be empty."
else
#echo ""
#echo "************************************************"
#echo "Run query: $1"
    # Step 1: In phase1_rep: run lucene package to generate "search_frag.txt"
    cd ./target/
    #edit query
    rm -rf search_frag.txt
    LUCENE_SEARCH_EXEC_NAME="java -jar ../src/phase1/search_step/lucene_search.jar $1" #&> /dev/null
    exec 3>&1 4>&2
    lucene_search_time=$( { time  $LUCENE_SEARCH_EXEC_NAME 1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    echo "Time taken for lucene search:"$lucene_search_time


    # Step 2: In phase1: run search_doc to generate "vidlist.txt" & "result.txt"
    PHASE1_SEARCH_EXEC_NAME="./phase1_search $2 $1"
    exec 3>&1 4>&2
    phase1_search_time=$( { time  $PHASE1_SEARCH_EXEC_NAME 1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    echo "Time taken for phase1 search:"$phase1_search_time

    # Step 3: In phase2: run "runjob_optionC.py" for optionC and "runjob_optionA.py" for optionA
    # edit query
    #sed -i "s/query_len = .*#!!!/query_len = $2#!!!/g" runjob_optionTestCluster_vector_new.py
    #sed -i "s/k=.*#!!!/k=$top_k#!!!/g" runjob_optionTestCluster_vector_new.py
    #sed -i '' "s/.\/index_search --index index --no-features --postings <<< '.*/.\/index_search --index index --no-features --postings <<< '$1'/g" lucene_step/run_search_qinghao.sh
    #sed -i "s/query_len = .*#!!!/query_len = $2#!!!/g" calculateTestCluster.py
    #sed -i "s/k = .*#!!!/k = $top_k#!!!/g" calculateTestCluster.py

#echo
#echo ">>>> Start Searching using OptionTestCluster <<<<"
#echo
    exec 3>&1 4>&2
    phase2_search_time=$( { time  $python ../src/phase2/search_step/run_phase2_search.py "$1" $2 $top_k 1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    echo "Time taken for phase2 search:"$phase2_search_time
#echo ">>>> Finish Searching using OptionTestCluster <<<<"

fi
