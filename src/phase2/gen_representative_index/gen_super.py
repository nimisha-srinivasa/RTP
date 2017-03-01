#!/usr/bin/python3

import pickle
import sys

path_to_target_dir = "./target/"
path_to_utils_dir = "./src/utils/"

## change your whole dataset path here
path_data = sys.argv[1]

# create dictionary 'dic' for vdrelation
dic = {}
dv_dic = {}
with open(path_to_target_dir + 'convert_table.txt', 'r') as fin_vd:
    for line_dv in fin_vd:
        line_dv_list = line_dv.split()
        did = int(line_dv_list[0])
        dv_dic[did] = int(line_dv_list[1])
        for i in range(2, len(line_dv_list)):
            dic[int(line_dv_list[i])] = did

# read stop words
stop_words = set()
with open(path_to_utils_dir + 'stop_words.txt', 'r') as f:
    for line in f:
        stop_words.add(line.strip())

# create a hash for each did a hash of word => frequency

print('Creating word unions ...')

# dict_d_tf: a dictionary, key is did, value is a dictionary dict_tf;
# dict_tf: a dictionary, key is word, value is frequency
dict_d_tf = {}

with open(path_data, 'r') as fin:
    vid = 0
    for line_data in fin:
        did = dic.get(vid)
        if did is None:
            print("version {} not found!".format(str(vid)))
            continue
        assert isinstance(line_data, str)
        words = line_data.split()

        dict_tf = dict_d_tf.setdefault(did, {})
        for word in words:
            dict_tf[word] = dict_tf.get(word, 0) + 1

        if vid > 0 and vid % 10000 == 0:
            print(vid, 'lines processed.')

        vid = vid + 1

index = {}

for doc_id in dict_d_tf:
    for word in dict_d_tf[doc_id]:
        if word in index:
            index[word].append(doc_id)
        else:
            index[word] = []

f = open(path_to_target_dir + 'super_index','w')
print('Writing super-version ...')
for word in index:
    if len(word) > 0 and len(index[word]) > 0 and word not in stop_words:
        tf_list = ['{did}:{tf:.2f}'.format(did=doc_id, tf=dict_d_tf[doc_id][word]/float(dv_dic.get(doc_id, 1))) for doc_id in index[word] if word not in stop_words]
        f.write('{w}\t{tf}'.format(w=word, tf=' '.join(tf_list)))
        f.write('\n')