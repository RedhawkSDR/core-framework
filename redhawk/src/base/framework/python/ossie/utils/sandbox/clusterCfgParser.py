
import os
import configparser

OSSIEHOME = os.getenv("OSSIEHOME")
CLUSTER_FILE = OSSIEHOME + "/cluster.cfg"

class ClusterCfgParser():
    def __init__(self, clusterName=None):
        config = configparser.ConfigParser()
        cluster_file = OSSIEHOME + '/cluster.cfg'
        data = open(cluster_file, 'r').read()
        config.read(cluster_file)
        if clusterName is None:
            clusterName = config["CLUSTER"]["name"]
        self.info = config[clusterName]
