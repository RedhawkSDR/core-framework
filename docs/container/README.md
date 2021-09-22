# REDHAWK Container Orchestration
REDHAWK offers a standardized methodology for running waveforms on container orchestration technologies by means of an extensible plugin class. These capabilities have been integrated into core-framework through modifications to the Application Factory and Sandbox but in a manner that emphaised backwards compatibility; it is still possible to use the standard Domain Manager and GPP combo to run native Executable and SharedLibrary type components on your REDHAWK systems.

Each cluster technology has its own unique ways to launch, configure, and delete containers, so the plugin system allows users to develop their own Sandbox and Application Factory plugins that Domain Manager dynamically loads at run-time to spawn waveforms on a cluster according a global cluster configuration file called `cluster.cfg` that controls the parameters passed to each plugin.

*NOTE* These additions have *not* been reconciled with REDHAWK IDE. Viewing Components with spd.xml files modified in support of using these features will generate visual errors in RHIDE.

# Installation Dependencies

1. Install the dependencies for REDHAWK with the [REDHAWK Manual](https://redhawksdr.org/2.2.8/manual/appendices/dependencies/)
2. Yum install the `yaml-cpp-devel` package for the plugins' yaml generation
3. Docker version 19.03.12 is required for building REDHAWK Component docker images
4. Download and install a docker image that contains an installed instance of REDHAWK 3.0

# Containerized REDHAWK

A Docker image with a REDHAWK install is required to support containerized component development and deployment.
This image must contain the REDHAWK libraries and the appropriate REDHAWK environment variables such as $SDRROOT.
Install the image such that it becomes available for both containerizing components and orchestration software.

# Building a Containerized Component

While a container can host any arbitrary software, the interface between the running component and the REDHAWK infrastructure remains the REDHAWK interfaces.
This documentation provides a guide through the steps necessary to build a containerized REDHAWK component.
For simplicity, the example selected is a standard REDHAWK component.

1. Create a REDHAWK Component
 - For the sake of this example, call this component `sample_comp` and generate it in Python
2. Modify the components SPD file
 - Open the SPD file (in this case, `sample_comp.spd.xml`)
 - Update the `code` section as follows:
```
<code type="Container">
      <localfile name="python"/>
      <entrypoint>python/sample_comp.py::sample_comp</entrypoint>
</code>
```
 - For the sake of this example, call this component `sample_comp` and generate it in Python
2. Go to the `docker-redhawk-components` directory
 - /&lt;root src&gt;/core-framework/redhawk-container/docker-redhawk-components
3. Replace `@@@BASE_IMAGE@@@` with the image containing the REDHAWK installation in all included Dockerfiles
 - For general convenience, both CentOS and Ubuntu Dockerfiles are included. Use only the appropriate operating system ones for the component to be containerized
4. Change directory to the appropriate operating system subdirectory under docker-redhawk-components (centos or ubuntu)
5. Build the image
 - There are multiple categories for containerized components that can be generated, such as REDHAWK core assets. In this case, `sample_comp` is a `custom` component. The arguments for the build are the build category for the component and the location of the project:
 ```
  make custom CUSTOM=/home/mydir/sample_comp
 ```

## Verifying the Image is Built

To see if the image has been built, view the docker image list:
```
> docker image list
REPOSITORY            TAG                  IMAGE ID       CREATED          SIZE
sample_comp           latest               bef41b7af47a   45 seconds ago   4.89GB
```

## Finalizing Installation

At runtime, the Domain (or Sandbox) parses the component's XML profile to determine the deployment model for that component.
Because the Domain must read the component's XML before loading and deploying the component, the component's XML must be installed in the Domain's `$SDRROOT`.

To install the XML, either copy the XML files from the project into `$SDRROOT/dom/components/<component name>`, or `build.sh install` the component into the Domain's `$SDRROOT`; whatever binaries are installed in `$SDRROOT` along with the component XML are ignored.

## Verifying Containerized Component Runs

To see if the containerized component runs, use the Sandbox.
In our example, to deploy `sample_comp`:
```
> python3
Python 3.6.8 (default, Nov 16 2020, 16:55:22) 
[GCC 4.8.5 20150623 (Red Hat 4.8.5-44)] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from ossie.utils import sb
>>> c=sb.launch('sample_comp')
a430ac7416cee6d27cc423e7eca4bb1ae85245d3934f58c1e3a93d292b67d42f
>>> c.start()
>>> c.started
True
```
The value returned by the launch method is generated by docker; it is the component's container id.
To verify that the container id is correct and the component is running as a container, list the running containers:
```
> docker container ps -a
CONTAINER ID   IMAGE         COMMAND                  CREATED         STATUS       PORTS      NAMES
a430ac7416ce   sample_comp   "/bin/bash -lc '$SDR…"   2 minutes ago   Up 2 minutes            sample_comp_1
```

To release the component, exit the Sandbox's Python session.
Verify that the container has been removed by listing the running containers.

# Orchestration Configuration
The following must be present in your environment (have network connectivity to your system running REDHAWK and proper configuration) in order to use RH components in clusters:
   * A cluster running the desired orchestration technology
   * The matching RH cluster plugin installed
   * Your networking, firewalls and DNS setup to communicate with your cluster's control plane/nodes/API
   * Your local system with REDHAWK installed must have its `cluster.cfg` file set correctly and the plugin's approprriate environment variables set

# Domain Configuration
<!--The Application Factory loops through the list of components in the waveform making note of the component's code type. If the code type is set to "Container", then the spd parsing mechanism checks the component's entrypoint. The entrypoint element has been extended to identify the suffix of a docker image path. Consider the example below pulled from SigGen's spd.xml file.
```
<code type="Container">
      <localfile name="cpp/SigGen"/>
      <entrypoint>cpp/SigGen::rh.siggen</entrypoint>
</code>
```
In this entrypoint element, everything after the "::" is regarded as the desired image name suffix. The "::" is a delimiter. The prefix of the image is found in the `cluster.cfg` file.
-->

The Domain Manager requires access to the images hosting the containerized components.
The `cluster.cfg` file (installed in $OSSIEHOME/) contains information supporting generic containerized deployment like ssh management information.
Technology-specific information is also included in this configuration file.
For example, in the case of DockerSwarm, this file contains the image registry and tag.

If all components in a waveform are of `Container` code type, then no GPP is required in the Domain to run the waveform.
Waveforms support "hybrid" deployments, where components with `Executable` or `SharedLibrary` code types are deployed in the same waveform as `Container` components.
Hybrid waveforms, or any waveform that uses `Executable` or `SharedLibrary` code types still require an appropriate available GPP in the Domain.

# Building and Installing Plugins

Plugins are used to bind a specific orchestration technology with REDHAWK.
Because plugins are not included as part of the standard REDHAWK build, they must be built from source.
Plugin development source code is included as part of the REDHAWK source code distribution.
The following sections describe how to build and install plugins to support a set of orchestration technologies.

<!--# Building and Installing core-framework from Source
```bash
$ cd /core-framework/redhawk/src
$ ./build.sh
$ sudo make install
```
-->

## Building and Installing Plugins
No plugins are built and installed by default.
Three plugins are currently included with REDHAWK:
1. [Docker](plugin-docker.md)
2. [DockerSwarm](plugin-dockerswarm.md)
3. [EksKube](plugin-ekskube.md)

## The cluster.cfg file
The `$OSSIEHOME/cluster.cfg` file contains sections named according to the cluster orchestration plugin technology (EKS, Docker Swarm, etc). Each plugin-specific section contains variables (key value pairs) used by the specific plugin. You should ensure the values set in this file are correct prior to running a waveform.

### Installing the Cluster Configuration File
```bash
$ cd ./core-framework/redhawk/src/base/cfg
$ sudo -E ./build.py --cluster <Desired_Plugin_Name> <Arguments to set variables in cluster.cfg>
```

This will render the template cluster.cfg file into your $OSSIEHOME/cluster.cfg file. The plugin name argument you specify sets the plugin's top section which controls will plugin REDHAWK will use, assuming the plugin is already installed.

Possible values for the plugin name argument are:
1. [EksKube](plugin-ekskube.md)
2. [Docker](plugin-docker.md)
3. [DockerSwarm](plugin-dockerswarm.md)

## Networking
Networking can get complicated quickly because it will vary depending on what you're trying to run in your cluster, what that cluster's networking setup looks like, and your local system's ability to send traffic to that cluster. Guiding rules of thumb are:
* If you are running DomMgr and/or GPP with OmniORB on your local system and only running waveforms on a cluster, those launched components need to be able to initiate connections to omniORB. This boils down to there being a "flat" network between where OmniORB runs and where the waveform runs. NAT will break one party's ability to talk to the other.
* In the scenario where all REDHAWK services (OmniORB, DomMgr, and/or GPP) run inside the cluster alongside the waveform payloads, so long as the containers can network resolve each other (almost always the case barring network security restrictions on the cluster), then there should not be any difficulties with networking.

Please see each plugin's documention for more more network specifics.

## Misc additions
Each plugin behaves differently. Some require specialized networking, other require special credentials, and some might require environment variables. Please consult the specific plugin's documentation to learn more.
