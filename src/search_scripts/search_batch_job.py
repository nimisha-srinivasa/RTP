#!/usr/bin/python3

import sys
import subprocess

list = []
query_file_path = "./target/" + sys.argv[1]
with open(query_file_path, 'r') as f:
    for line in f:
        q = line.strip()
        length = len(q.split(' '))
        list.append((q, length))
for (q, length) in list:
    print(q + " " + str(length))
    comm2 = './src/search_scripts/run_search.sh "' + q + '" ' + str(length)
    subprocess.call(comm2, shell = True)
