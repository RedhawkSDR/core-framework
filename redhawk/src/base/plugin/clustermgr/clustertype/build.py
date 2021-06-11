#!/usr/bin/python

from jinja2 import Environment, FileSystemLoader
import os
import sys

cluster = sys.argv[1]

OSSIEHOME = os.getenv("OSSIEHOME")

file_loader = FileSystemLoader('templates')
env = Environment(loader=file_loader)

mgr_template = env.get_template('clustermgr.cpp')

mgr = mgr_template.render(cluster=cluster)

with open("clustermgr.cpp", "w") as new_cluster:
        new_cluster.write(mgr)

make_template = env.get_template('Makefile.am')

make = make_template.render(cluster=cluster)

with open("Makefile.am", "w") as new_make:
        new_make.write(make)
