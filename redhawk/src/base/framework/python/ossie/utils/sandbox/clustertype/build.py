#!/usr/bin/python3

from jinja2 import Environment, FileSystemLoader
import os
import sys

cluster = sys.argv[1]



OSSIEHOME = os.getenv("OSSIEHOME")

content = 'This is about page'

file_loader = FileSystemLoader('templates')
env = Environment(loader=file_loader)

template = env.get_template('cluster.py')

output = template.render(cluster=cluster, cluster_lower=cluster.lower())

with open(OSSIEHOME+"/lib/python/ossie/utils/sandbox/cluster.py", "w") as new_cluster:
        new_cluster.write(output)
