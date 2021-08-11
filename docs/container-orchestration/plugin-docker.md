# Docker
The Docker plugin is designed to run REDHAWK waveforms on your local system's Docker installation.

# Building and Installing the Plugin

Both the Application Factory and Sandbox plugins for Docker are installed when core-framework is built and installed from source.

Application Factory ("make install" command may need sudo)
```bash
$ cd core-framework/redhawk/src/base/plugin/clustermgr/clustertype
$ ./build.py Docker
$ ./reconf && ./configure && make && make install
```
Sandbox ("make" command may need sudo)
```bash
$ cd core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox
$ make FILE=Docker
```

This will compile and install the Application Factory and Sandbox plugins for the user. The plugins are built in a specific location in core-framework (`core-framework/redhawk/src/base/plugin/clustermgr/`and `core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox/`) and are both installed to `${OSSIEHOME}/lib`

# Plugin Specifics
## Dependencies
1. Docker installed on your system along with REDHAWK

## The cluster.cfg file
```bash
cd core-framework/redhawk/src/base/cfg
sudo -E ./build.py --cluster Docker --docker_dir <where-to-mount-on-container> --mount_dir <where-to-mount-from>
```
OR
```bash
cd core-framework/redhawk/src/base/cfg
make Docker DOCKER_DIR="<your-docker-dir>" MOUNT_DIR="<your-mount-location>"
```
This will properly set the top section to use the Docker plugin and pass in the assortment of arguments to setup the cluster.cfg file.

## cluster.cfg file variables
The top section of the file should specify that the Docker plugin is desired like so:
```
[CLUSTER]
name = Docker
```
| Variable   | Example Value         | Description                                                |
|------------|-----------------------|------------------------------------------------------------|
| docker_dir | /mnt/                 | Path inside of the docker container to mount mount_dir to  |
| mount_dir  | /home/bob/myshareddir | Path on the docker host to mount into the docker container |

*NOTE*: These variables are only used by the Sandbox Docker plugin and do not work for the Application Factory plugin.

## Credentials
The Docker plugin needs the following credentials:
1. Your ~/.docker/config.json updated to allow docker to pull your Component images from your desired Registry if the images are not already present on your local system

## Networking
Docker uses a bridged network by default by running the containers with the "--network host" option.

This configuration is ideal for enabling your containers to communicate with running REDHAWK services on the same system.
