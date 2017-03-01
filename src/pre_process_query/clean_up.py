#!/usr/bin/python3

import re
import os
import sys

if __name__ == '__main__':
    src_path = sys.argv[1]
    dest_path = sys.argv[2]

    if os.path.isfile(src_path):
        with open(src_path, 'r') as f_in, open(dest_path, 'w') as f_out:
            for line in f_in:
                s = re.sub(r'[^a-zA-Z0-9\s]', ' ', line)
                f_out.write(s)
