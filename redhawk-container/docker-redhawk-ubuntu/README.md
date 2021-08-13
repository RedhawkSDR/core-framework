# Docker-REDHAWK Ubuntu Edition

This repository builds a series of Docker images and scripts for standing up an installation of REDHAWK SDR as well as several example devices and a web server.  If you're familiar with [Docker-REDHAWK](http://github.com/GeonTech/docker-redhawk), this document will seem very familiar except for this:

 > **IMPORTANT NOTE:** This version's Components will not be binary compatible with a CentOS 7 or RPM -based installation.  You cannot deploy Components from one installation type onto the GPP of another.  That said, nothing should limit you from having an Ubuntu -borne Node join a CentOS 7 Domain or vice versa.

For the USRP image (geontech/redhawk-ubuntu-usrp), the UHD driver is recompiled to a newer version and the USRP_UHD Device is compiled from source against that newer driver.  The result is access to the latest Ettus Research USRPs from the container.

 > **DOCKER:** You must be using at least Docker 17.  This was tested using Docker-CE.

## Installing

Run `make` to pull the prebuild images and link the helper scripts.  Depending on your internet connection, this may take several minutes.

## Building

To build all images, simply type `make build`.  At the command line, you can also specify the variables: `REST_PYTHON` and `REST_PYTHON_BRANCH`, which specifically apply to the `geontech/redhawk-ubuntu-webserver`.

You will end up with the following images that are meant't to be run individually.

* `geontech/redhawk-ubuntu-base`: This is the repository installation, omni services (non-running), and an `/etc/omniORB.cfg` update script.
* `geontech/redhawk-ubuntu-runtime`: The typical "REDHAWK Runtime" group install.  It is the basis for the `domain` and various device launchers.

The remaining images are derived and come with helper scripts for deploying your system:

 * `geontech/redhawk-ubuntu-omniserver`: Inherits from `geontech/redhawk-ubuntu-base`, it has OmniORB and OmniEvents services wrapped in a single image, intended to be run as a singleton in the network.  If you have an Omni server running elsewhere, you do not need this.
 * `geontech/redhawk-ubuntu-development`: Configured to expose a workspace volume and run the IDE.
 * `geontech/redhawk-ubuntu-domain`: Configured to run as a Domain.
 * `geontech/redhawk-ubuntu-gpp`: Configured to run as a GPP -bearing Node.
 * `geontech/redhawk-ubuntu-rtl2832u`: Configured to run as an RTL2832U -bearing Node. 
 * `geontech/redhawk-ubuntu-usrp`: Configured to run as an USRP_UHD -bearing Node.
 * `geontech/redhawk-ubuntu-bu353s4`: Configured to run as a BU353S4 -bearing Node.
 * `geontech/redhawk-ubuntu-webserver`: Instantiates a rest-python web server.

 The following scripts will also be linked into the main directory.  Each script supports `-h` and `--help` to learn the usage of the script, and running the script with no arguments provides a status update.

 * `login`: Starts a bash shell for the named container (basically a `docker exec` wrapper).
 * `show-log`: Displays a log from a named container.
 * `omniserver`: Manages an instance of the OmniORB services (locally to the Docker host, optional).
 * `domain`: Manages REDHAWK Domain instances.
 * `gpp`: Starts or stops a GPP for the named domain and external OmniServer IP address.
 * `rtl2832u`: Manages [RTL2832U](http://github.com/redhawksdr/RTL2832U) Nodes (USB-attached devices).
 * `usrp`: Manages [USRP_UHD](http://github.com/redhawksdr/USRP_UHD) Nodes (USB- or network-attached devices).
 * `bu353s4`: Manages [BU353S4](http://github.com/geontech/BU353S4) Nodes (USB serial).
 * `volume-manager`: Creates or deletes Docker volumes labeled for use as an SDRROOT or a development (IDE) Workspace.
 * `rhide`: Runs an instance of the REDHAWK IDE with named SDRROOT and workspace (Docker volume or host file system).
 * `webserver`: Manages an instance of a REST-Python server.

## Usage

The main elements one needs for a REDHAWK system are the naming and event services (OmniORB and OmniEvents), a Domain, and a GPP.  If the scripts are not in the main directory, use `make scripts` to generate the links.  Each scripts supports the `-h` and `--help` that cover usage.  Below is a simplified example.

    ./omniserver
    ./domain start REDHAWK_DEV1

At this point you will have a functioning REDHAWK Domain at a host-exposed OmniORB server.  Its container name will be `REDHAWK_DEV`.  Other non-Docker REDHAWK instances can now join this Domain as well as long as your host system's firewall settings expose ports 2809 and 11169.

    ./gpp start GPP1 -d REDHAWK_DEV1

A GPP container launches the node GPP1 on the REDHAWK_DEV1 Domain.  Its container name will be `GPP1-REDHAWK_DEV1`.  You can now launch waveforms.

If you would like to log into the Domain container, use `login`:

    ./login REDHAWK_DEV1 redhawk

You will enter a bash shell as the `redhawk` user.  

 > **Note:** Not all images have this user defined.  For example, the only user in the `geontech/redhawk-ubuntu-omniserver` image is `root`, the default.

 > **Note:** For containers where the name is derived from the Node and Domain names (e.g., GPP), you have to specify the full container name (in this example, `GPP1-REDHAWK_DEV1`).

## Persistent SDRROOT

Use `volume-manager` to create an SDRROOT volume that can be mounted to the Domain and IDE.

    ./volume-manager create sdrroot MY_REDHAWK
    ./domain start MY_DOMAIN --sdrroot MY_REDHAWK

The result will be a Domain with a persistent SDRROOT.

## Running the IDE

The `geontech/redhawk-ubuntu-development` image provides the development libraries necessary to develop components and devices.  Use the `rhide` script to map your SDRROOT volume and workspace (absolute path or volume name):

    ./rhide --sdrroot MY_REDHAWK --workspace /home/me/workspace

## REST-Python Web Server

The following section contains information about the `geontech/redhawk-ubuntu-webserver` REST-Python server image.

### Running

Running the REST-Python server via the script is simple:

    ./webserver start

The host-side port to map can be set using either `-p` or `--port`.  

Additionally, if an alternate version of REST-Python should be mounted, use `--rest-python` to specify the local absolute file path.  Alternatively, one can build the image with a different version already installed (see [this](#customizing-the-image)).

### Customizing the Image

Building the REST-Python `geontech/redhawk-ubuntu-webserver` image has a two options:

 * `REST_PYTHON`: URL to a git server where the REST-Python source is located (default is [Geon's](http://github.com/geontech/rest-python)).
 * `REST_PYTHON_BRANCH`: Branch name of the preferred REST-Python source tree (default is master).

Specifying no options (`make geontech/redhawk-ubuntu-webserver`) bakes in the default REST-Python server and branch.  The above options can be specified by passing the variables at make time:

    make geontech/redhawk-ubuntu-webserver REST_PYTHON=http://my_other_target/repo

## Device Node Controllers

The following devices have their own images, launching scripts, etc. to facilitate rapid development and integration with hardware.  The included devices are the REDHAWK SDR community's GPP, USRP_UHD and RTL2832U as well as Geon's [BU353S4](http://github.com/geontech/BU353S4).

In each case, similar to running `./domain` with no arguments, each of these device launchers will also print a status message for all running instances of their respecive images.

### GPP

The `geontech/redhawk-ubuntu-gpp` image provides a Node running a GPP Device for your system.  The `gpp` script provides only a few options for naming the device, setting the Domain name, etc.  See the `--help` for usage.  Starting with the default domain:

    ./gpp start MyGPP


### USRP_UHD

The `geontech/redhawk-ubuntu-usrp` image compiles the 3.10 version of UHD and then compiles the USRP_UHD from source.  The resulting device is capable of working with many of the latest Ettus Research USRPs like the USB-attached B205mini.  The associated `usrp` script allows you to configure the Device in its Node when starting an instance of it.

For the B205mini, it identifies as the `b200` type.  With it connected to a powered USB 3.0 port:

    ./usrp start MyB205 --usrptype b200

This will start a container attached to REDHAWK_DEV (the default Domain).  The container will be mapped to the host's `/dev/usb` bus as a volume so that UHD can locate the device and update it before the node is started.  

 > **Note:** Because the container startup is going through that lengthy process, startup may take several seconds.

 Other USRP types can be as well.  See the script's `--help` for a list of options.

#### Special notes for USB USRPs

USB-attached USRPs like the B205mini may need firmware loaded the first time the device is plugged into the computer (or virtual machine).  Otherwise, when you `show-log` the container, you will see CORBA Transient errors when the device fails to initialize quickly enough.  You can stop and restart the container andd the problem will be resolved, or you can run a container first to initialize the device:

    docker run --rm -it \
        -v /dev/bus/usb:/dev/bus/usb \
        --privileged \
        geontech/redhawk-usrp \
        bash -lc "uhd_find_devices && uhd_usrp_probe"

The result is a one-off container that will configure the firmware and FPGA image.

### RTL2832U

The `geontech/redhawk-ubuntu-rtl2832u` image provides the RTL2832U Device.  The associated launcher script is `rtl2832u`:

    ./rtl2832u start MyRTL

This will start a container with the RTL2832U device configured with its defaults.

 > **IMPORTANT:** You may need to blacklist the RTL's kernel driver(s) on your host system before starting this container.

 #### Black-listing

 Methods for temporarily unloading the kernel drivers varies with host operating system distribution.  For example, Ubuntu 16.04 is by way of `modprobe`:

    modprobe -r dvb_usb_rtl28xxu rtl2832

To make the change permanent, one creates a configuration (`.conf`) file in `/etc/modprobe.d` with these contents:

    blacklist dvb_usb_rtl28xxu
    blacklist rtl2832
    blacklist rtl2830

### BU353S4

The `geontech/redhawk-ubuntu-bu353s4` image provides Geon's BU353S4 FEI 2.0 -compliant USB-attached serial GPS receiver.  Like the Device, it has only one option: the path to the serial device in `/dev`.  The default is `/dev/ttyUSB0`.  Starting an instance on the default REDHAWK_DEV Domain is:

    ./bu353s4 start MyGPS

You can then attach to the GPS port and pull coordinates, time, etc.
