#!/usr/bin/python3

#This python script is used to output the FORWARD_TABLE from index.txt for all clusters

import os

path_to_target_dir = "./target/"

# function to run shell command
def run_command(com):
    print com
    output = os.popen(com).read()
    a = output.split('\n')
    for b in a:
        print b

# function to run shell command but no output to screen
def run_command_n(com):
    newc = com + ' >> ' + path_to_target_dir + 'log.out 2>&1'
    os.popen(newc)

fin = open(path_to_target_dir + 'convert_table.txt','r')

for line in fin.readlines():
    line_list = line.split(' ')
    did = line_list[0]
    if (int(did) % 1000 == 0):
        print "finish gen_forward for "+did+ " clusters"
    run_command_n(path_to_target_dir + 'convert '+did);

