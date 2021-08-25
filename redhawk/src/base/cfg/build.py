#!/usr/bin/python3

from jinja2 import Environment, FileSystemLoader
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--cluster",     default="Docker")
parser.add_argument("--registry",    default="\"\"")
parser.add_argument("--ssh_key",     default="\"\"")
parser.add_argument("--server_user", default="\"\"")
parser.add_argument("--server_ip",   default="\"\"")
parser.add_argument("--docker_dir",  default="\"\"")
parser.add_argument("--mount_dir",   default="\"\"")
parser.add_argument("--json",        default="\"\"")

args = parser.parse_args()

OSSIEHOME = os.getenv('OSSIEHOME')

content = 'This is about page'

file_loader = FileSystemLoader('templates')
env = Environment(loader=file_loader)

template = env.get_template('cluster.cfg')

output = template.render(cluster=args.cluster, dockerjsonconfig=args.json, registry=args.registry, ssh_key=args.ssh_key, server_user=args.server_user, server_ip=args.server_ip, docker_dir=args.docker_dir, mount_dir=args.mount_dir)
with open(OSSIEHOME+"/cluster.cfg", "w") as new_cluster:
    new_cluster.write(output)
