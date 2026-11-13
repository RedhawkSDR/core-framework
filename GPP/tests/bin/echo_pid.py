#!/usr/bin/python3

import os

print('CWD:', os.getcwd())
with open('pid.out', 'w') as fp:
    print(os.getpid(), file=fp)
