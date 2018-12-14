
import os
import sys


class HelloWorld():
    def __init__(self):
        self.a = 100
        self.b = [ 1.0, 2.0, 3.0 ]
        self.c = { 'testing' : 1, 'foobar' : 2, 'jones': 'goober' }


    def __str__(self):
        return "a: " + str(self.a) + "\nb: " + str(self.b) + "\nc: " + str(self.c)
