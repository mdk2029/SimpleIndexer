#!/usr/bin/python

import fnmatch
import os
import argparse
import re
import operator
import itertools
import heapq


parser = argparse.ArgumentParser(description='python sssi')
parser.add_argument('--dir', action="store", dest="startDir", required=True)
parser.add_argument('--result-file', action="store", dest="resultFile", required=True)
parser.add_argument('--dump-full', action="store_true", dest="dumpFull")
parser.add_argument('--all-files', action="store_true", dest="allFiles")

args = parser.parse_args();

pattern = '*.txt'
if args.allFiles:
    pattern = '*'
seenfiles = set()
wordCount = dict()
 
for root, dirs, files in os.walk(args.startDir):
    for filename in fnmatch.filter(files, pattern):
        fullpath = os.path.join(root,filename)
        if not os.path.islink(fullpath) :
            statbuf = os.stat(fullpath)
            if (statbuf.st_dev,statbuf.st_ino) not in seenfiles :
                seenfiles.add((statbuf.st_dev,statbuf.st_ino))
                try:
                    with open(fullpath,'r') as f:
                        #print "Processing : " , fullpath
                        for line in f :
                            words = re.findall('[a-zA-Z0-9]+',line)
                            for word in words:
                                if word.upper() in wordCount :
                                    wordCount[word.upper()] += 1
                                else :
                                    wordCount[word.upper()] = 1
                except IOError:
                    pass


if not args.dumpFull :
    sorted_wc = sorted(wordCount.items(), key=operator.itemgetter(1))
    sorted_wc.reverse();
    trimmed = sorted_wc[:10]
    with open(args.resultFile,'w') as f:
        for w in trimmed :
            f.write('{}\t{}\n'.format(w[0],w[1]))
else:    
    with open(args.resultFile,'w') as f:
        for k,v in wordCount.iteritems() :
            f.write('{}\t{}\n'.format(k,v))


        
