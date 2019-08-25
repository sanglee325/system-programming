#!/usr/bin/env python

# import modules
from itertools import groupby
from operator import itemgetter
import sys

data = {}

# input comes from STDIN
for line in sys.stdin:
    # remove leading and trailing whitespace
    line = line.strip()

    # parse the input we got from mapper.py
    group, num = line.split('\t', 1)

    # convert count (currently a string) to int
    num = float(num)

    if group in data.keys():
        data[group] = max(data[group], num)
    else:
        data[group] = num

for group, number in data.items():
    print("{},{}".format(group, number))
