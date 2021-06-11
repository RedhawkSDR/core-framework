# DockerSwarm
The DockerSwarm plugin is designed to run REDHAWK waveforms on Docker Swarm clusters.

# Building and Installing the Plugin

Application Factory
```bash
$ cd core-framework/redhawk/src/base/plugin/clustermgr/clustertype
$ sudo ./build.py DockerSwarm
$ ./reconf && ./configure && make && sudo make install
```
Sandbox
```bash
$ cd core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox/clustertype
$ make FILE=DockerSwarm
```

This will compile and install the Application Factory and Sandbox plugins for the user. The plugins are built in a specific location in core-framework (`core-framework/redhawk/src/base/plugin/clustermgr/`and `core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox/`) and are both installed to `/usr/local/redhawk/core/lib`

# Plugin Specifics
## Dependencies
1. None

## The cluster.cfg file
```bash
cd core-framework/redhawk/src/base/cfg
sudo -E ./build.py --cluster DockerSwarm --registry <your-registry> --ssh_key <your-ssh-key> --server_user <your-server-username> --server_ip <your-server-ip-addr>
```
OR
```bash
cd core-framework/redhawk/src/base/cfg
make DockerSwarm REGISTRY="<your-registry>" SSH_KEY="<your-ssh-key>" SERVER_USER="<your-server-username>" SERVER_IP="<your-server-ip-addr>"
```
This will properly set the top section to use the DockerSwarm plugin and pass in the assortment of arguments to setup the cluster.cfg file.

## cluster.cfg file variables
The top section of the file should specify that the DockerSwarm plugin is desired like so:
```
[CLUSTER]
name = DockerSwarm
```
| Variable         | Example Value      | Description                                                                                                                                            |
|------------------|--------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| registry         | geontech           | This value is concatenated with a "/" and then the image suffix found in the entrypoint of the component's spd.xml file. Shared across all components. |
| tag              | latest             | The image tag used. Shared across all components.                                                                                                      |
| key              | ~/.ssh/ssh_key.pem | Path to your SSH key. Must be read-accessible by Domain Manager                                                                                        |
| user             | centos             | User used to log into node via SSH                                                                                                                     |
| ip               | 10.10.10.10        | IP address or FQDN of the Swarm Master node to connect to via SSH                                                                                      |
| docker_login_cmd | "docker login"     | The command ran by the plugin to log into the registry hosting the Component container images                                                          |

## Credentials
The DockerSwarm plugin needs the following credentials:
1. A linux user on a Docker Swarm Manager node authorized to SSH into it with a corresponding SSH private key
2. Docker login creds on the Docker Swarm Manager Node

Docker Swarm does not offer a "remote control" capability like K8S does via kubectl, so the Docker Swarm plugin uses SSH to log into a Swarm Manager Node to run/apply the generated docker-compose yaml. For this SSH login to work, a pre-configured user must exist on the targeted Manager node that is allowed to SSH in.

When the plugin logs into the node over SSH, it first runs the command defined in the docker_login_cmd variable in `cluster.cfg`. You can overrite this value to supply your username and password as arguments in the `cluster.cfg` file.

## Networking
Docker Swarm uses virtual overlay networks by default, which isn't ideal for waveforms on the Swarm and REDHAWK services running on a remote system external to the cluster. It is for this reason that the yaml generated for Docker Swarm creates a host network and attached all Component containers to that network:
```yaml
version: '3'
networks:
  outside:
    external:
      name: host
services:
  siggen1:
    ...
    networks:
      - outside
````
This makes each Component container share the host node's IP address.


This configuration is ideal for running REDHAWK services external to the cluster (on your local REDHAWK system).
