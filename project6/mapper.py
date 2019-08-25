#!/usr/bin/env python
import sys

# 'file' in this case is STDIN
def read_input(file):
    # Split each line into words
    for line in file:
        yield line.split()

def main(separator='\t'):
    # Read the data using read_input
    data = read_input(sys.stdin)
    # Process each word returned from read_input
    for words in data:
        # Process each word
        for word in words:
            group, num = word.split(',')
            num = float(num)
            print('%s%s%f' % (group, separator, num))

if __name__ == "__main__":
    main()
