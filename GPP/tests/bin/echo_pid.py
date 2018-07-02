#!/usr/bin/python

import os

print 'CWD:', os.getcwd()
with open('pid.out', 'w') as fp:
    print >>fp, os.getpid()
