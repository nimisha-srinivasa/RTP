#!/usr/bin/python3

import sys
import argparse
import math

parser = argparse.ArgumentParser(description='Post-processing after labeling, please put your rating in a file with '
                                             'the same order as in docs.txt, one rating per line.')

parser.add_argument('--doc', required=True, help='path for doc_id_union.txt')
parser.add_argument('--rating', required=True, help='path for your rating file')
parser.add_argument('--result', required=True, help='list of doc_id.txt, separated by colon')
parser.add_argument('--dict', default='phase1/gen_index/vdrelation.txt', help='path for vdrelation.txt')
parser.add_argument('-n', type=int, default=10, help='number of results')

args = parser.parse_args()

v_id_path = args.doc
rating_path = args.rating
result_paths = args.result.split(',')

v_ids = []
ratings = []

data = []

try:
    print('Reading vid-did mapping ...')
    # read vid-did map from vdrelation.txt
    vd_map = {}
    with open(args.dict, 'r') as f:
        for line in f:
            v_id, doc_id = line.split()
            vd_map[v_id] = doc_id

    # read document ids
    with open(v_id_path, 'r') as f:
        for line in f:
            v_id = line.split()[0]
            v_ids.append(v_id)

    # read ratings
    with open(rating_path, 'r') as f:
        for line in f:
            rating = int(line.split()[0])
            ratings.append(rating)

    rating_map = dict(zip(v_ids, ratings))

    # read data from each result file
    for path in result_paths:
        with open(path, 'r') as f:
            local_v_ids = []
            for line in f:
                v_id = line.split()[0]
                local_v_ids.append((v_id, rating_map[v_id]))

            data.append((path, local_v_ids))

    # generate ideal rating order
    unique_ratings = {}
    for v_id, rating in rating_map.items():
        if vd_map[v_id] in unique_ratings:
            current_rating = unique_ratings[vd_map[v_id]]
            if rating > current_rating:
                unique_ratings[vd_map[v_id]] = rating
        else:
            unique_ratings[vd_map[v_id]] = rating

    rating_order = list(unique_ratings.values())
    rating_order.sort(reverse=True)
    ideal_rating_order = rating_order[:args.n] + [0] * (args.n - len(rating_order[:args.n])) # fix: fill the empty entries with 0s

    # calculate and generate result
    for path, rating_list in data:
        title = ' @ '.join(path.split('/')[-3:-1])
        print('\nResult for', title, '\n')
        print('{:<8}\t{:<8}\t{:<8}\t{:<8}\t{:<8}\t{:<8}\t{:<8}\t{:<8}\t{:<8}'.format('Rank','VID','Rating','Relevance','DCG','Ideal Rating','Ideal Relevance','IDCG','NDCG'))
        # print('Rank\tRating\tRelevance\tDCG\tIdeal Rating\tIdeal Relevance\tIDCG\tNDCG')
        rank = 1
        dcg = 0
        idcg = 0

        filled_rating_list = rating_list + [('Empty', 0)] * (args.n - len(rating_list))

        for v_id, rating in filled_rating_list:
            relevance = (2 ** rating - 1) / math.log2(1 + rank)
            dcg += relevance
            ideal_rating = ideal_rating_order[rank - 1]
            ideal_relevance = (2 ** ideal_rating - 1) / math.log2(1 + rank)
            idcg += ideal_relevance
            ndcg = dcg / idcg

            print('{:<8}\t{:<8}\t{:<8}\t{:<8.4f}\t{:<8.4f}\t{:<8}\t{:<8.4f}\t{:<8.4f}\t{:<8.4f}'.format(rank, v_id, rating, relevance, dcg, ideal_rating, ideal_relevance,
                                                           idcg, ndcg))

            rank += 1


except IOError as e:
    print('Cannot open necessary files', file=sys.stderr)