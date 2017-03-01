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
    newc = com + ' >> log.out 2>&1'
    os.popen(newc)

fin = open(path_to_target_dir + 'convert_table.txt','r')
os.chdir(path_to_target_dir + 'cluster/')
curr_path_to_target_dir = "../../"

for line in fin.readlines():
    line_list = line.split(' ')
    did = line_list[0]
    if (int(did) % 100 == 0):
        print "finish lucene_step for "+did+ " clusters"
    os.chdir(did+'/')
    #run_command_n('cp ../../lucene_step/binary_index_gen ./')
    run_command_n(curr_path_to_target_dir + 'phase2_index --index index < all_fragments.txt')
    run_command_n(curr_path_to_target_dir + 'gen_bitmap')
    os.chdir('..')

