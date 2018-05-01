#!/usr/bin/python

import argparse

parser = argparse.ArgumentParser(description='python sssi')
parser.add_argument('--py-result-file', action="store", dest="pyRes", required=True)
parser.add_argument('--cpp-result-file', action="store", dest="cppRes", required=True)
args = parser.parse_args();

pyWordCount = dict()
with open(args.pyRes,'r') as f:
    for line in f:
        k,v = line.split()
        pyWordCount[k.strip()] = int(v.strip())

cppWordCount = dict()
with open(args.cppRes,'r') as f:
    for line in f:
        k,v = line.split()
        cppWordCount[k.strip()] = int(v.strip())

if pyWordCount == cppWordCount :
    print "Python wordCount matches cpp wordCount :)"
else:
    print "Python wordCount does not match cpp wordCount :("
    for pk,pv in pyWordCount.iteritems() :
        cv = 0;
        if pk in cppWordCount:
            cv = cppWordCount[pk]
        if pv != cv :
            print "Python has : ",pk, ",", pv;
            if cv:
                print "cpp    has : ", pk , "," , cv;
            else:
                print "cpp does not have " , pk
        cppWordCount.erase(pk);

    print "Python does not have the following that are in cpp :"
    for ck,cv in cppWordCount.iteritems() :
        print ck, "," << cv;
                

        
