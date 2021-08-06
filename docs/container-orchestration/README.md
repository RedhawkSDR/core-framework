# REDHAWK Container Orchestration
REDHAWK offers a standardized methodology for running waveforms on container orchestration technologies by means of an extensible plugin class. These capabilities have been integrated into core-framework through modifications to the Application Factory and Sandbox but in a manner that emphaised backwards compatibility; it is still possible to use the standard Domain Manager and GPP combo to run native Executable and SharedLibrary type components on your REDHAWK systems.

Each cluster technology has its own unique ways to launch, configure, and delete containers, so the plugin system allows users to develop their own Sandbox and Application Factory plugins that Domain Manager dynamically loads at run-time to spawn waveforms on a cluster according a global cluster configuration file called `cluster.cfg` that controls the parameters passed to each plugin.

*NOTE* These additions have *not* been reconciled with REDHAWK IDE. Viewing Components with spd.xml files modified in support of using these features will generate visual errors in RHIDE.

# General Installation Dependencies

1. Install the dependencies for REDHAWK with the [REDHAWK Manual](https://redhawksdr.org/2.2.8/manual/appendices/dependencies/)
2. Yum install the `yaml-cpp-devel` package for the plugins' yaml generation
3. Docker version 19.03.12 is required for building REDHAWK Component docker images
4. Download and install [docker-redhawk](https://github.com/Geontech/docker-redhawk.git) tag 2.2.8-0 or [docker-redhawk-ubuntu](https://github.com/Geontech/docker-redhawk-ubuntu.git) branch develop-2.2.1 (Ubuntu is only required for GNURadio components)
5. The following must be present in your environment (have network connectivity to your system running REDHAWK and proper configuration) in order to use RH components in clusters:
   * A cluster running the desired orchestration technology
   * The matching RH cluster plugin installed
   * Your networking, firewalls and DNS setup to communicate with your cluster's control plane/nodes/API
   * Your local system with REDHAWK installed must have its `cluster.cfg` file set correctly and the plugin's approprriate environment variables set
   
For plugin specific instructions, please see that plugin's corresponding documentation.

# Plugin Execution Path
The Application Factory loops through the list of components in the waveform making note of the component's code type. If the code type is set to "Container", then the spd parsing mechanism checks the component's entrypoint. The entrypoint element has been extended to identify the suffix of a docker image path. Consider the example below pulled from SigGen's spd.xml file.
```
<code type="Container">
      <localfile name="cpp/SigGen"/>
      <entrypoint>cpp/SigGen::rh.siggen</entrypoint>
</code>
```
In this entrypoint element, everything after the "::" is regarded as the desired image name suffix. The "::" is a delimiter. The prefix of the image is found in the `cluster.cfg` file.

If all components in the waveform are of Container code type, then no GPP is required to run the waveform. Waveforms can be ran in a "hybrid" manner in which components with Executable or SharedLibrary code types will run on the native REDHAWK system while those with Container code type are executed on the cluster dictated by the installed plugin and `cluster.cfg` file. Hybrid waveforms, or any waveform that uses Executable or SharedLibrary code types still required a GPP to run (this is for backwards compatibility).

# Building and Installing core-framework from Source
```bash
$ cd /core-framework/redhawk/src
$ ./build.sh
$ sudo make install
```
## Building and Installing Plugins
No plugin is installed by default. Three plugins are currently included with REDHAWK:
1. [Docker](plugin-docker.md)
2. [DockerSwarm](plugin-dockerswarm.md)
3. [EksKube](plugin-ekskube.md)

## The cluster.cfg file
The `$OSSIEHOME/cluster.cfg` file contains sections named according to the cluster orchestration plugin technology (EKS, Docker Swarm, etc). Each plugin-specific section contains variables (key value pairs) used by the specific plugin. You should ensure the values set in this file are correct prior to running a waveform.

#### Installing the Cluster Configuration File
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
