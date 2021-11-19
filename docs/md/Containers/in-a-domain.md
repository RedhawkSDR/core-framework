# Use in a Domain

This page is about running REDHAWK components, inside Docker containers, in a REDHAWK Domain.

The Domain Manager requires access to the images hosting the containerized components.
The `$OSSIEHOME/cluster.cfg` file provides configuration, both general, and specific to individual orchestration technologies.

If all components in a waveform are `<code type="Container">`, then no GPP is required in the Domain to run the waveform.
Waveforms support hybrid deployments, where components with Executable or SharedLibrary code types are deployed in the same waveform as Container components.
Hybrid waveforms, or any waveform that uses Executable or SharedLibrary code types still require an appropriate available GPP in the Domain.

Plugins are used to bind a specific orchestration technology with REDHAWK. The use of plugins allows REDHAWK to interact with any container orchestration technology. Because plugins are not included as part of the standard REDHAWK build, they must be built from source. Plugin development source code is included as part of the REDHAWK source code distribution. The following sections describe how to build and install plugins to support a set of orchestration technologies.

For the examples below, consider a user-defined component named `sample_comp`.  A Docker image with the component is stored at registry location `container-registry.my-company.org/my-project/sample-comp:latest`.

## XML Files in SDRROOT

See this [section](in-sandbox.html#xml-files-in-sdrroot) in the page Use in Python Sandbox.

## Plugin Installation

No plugins are built and installed by default. Two plugins are currently included with REDHAWK:
- Docker for running containers on the local system
- EksKube for running containers on an AWS EKS cluster

### Docker

To build an install the plugin for Docker:
```sh
cd core-framework/redhawk/src/base/plugin/clustermgr/clustertype
./build.py Docker
./reconf && ./configure && make && make install  # make install may require sudo
```

To modify `$OSSIEHOME/cluster.cfg`:
```sh
cd core-framework/redhawk/src/base/cfg
make Docker
```
This will set the top section of the file to be:
```ini
[CLUSTER]
name = Docker
```

### Kubernetes

To build an install the plugin for Kubernetes:
```sh
cd core-framework/redhawk/src/base/plugin/clustermgr/clustertype
./build.py EksKube
./reconf && ./configure && make && make install  # make install may require sudo
```

To modify `$OSSIEHOME/cluster.cfg`:
```sh
cd core-framework/redhawk/src/base/cfg
make EksKube REGISTRY="<your-registry>" JSON=""
```
The EksKube portions of the resulting file should look like:
```ini
[CLUSTER]
name = EksKube

[EksKube]
registry = container-registry.my-company.org
tag = latest
dockerconfigjson = ""
```
This table describes the variable values:

| Variable         | Example Value | Description |
|------------------|----------------|-------|
| registry         | sample.amazonaws.com/my\_context | Base URI for the container repository |
| tag              | latest | The image tag used (common across components) |
| dockerconfigjson |  | The auth field of ~/.docker/config.json (may not be needed) |

In AWS, the registry is found in AWS Elastic Container Registry`->`Repositories`->`URI. This is the image registry that EKS will use to find images to deploy as containers.

The tag `"latest"` is added by default by the image creation Dockerfiles included with RH.

The variable `dockerconfigjson` is not needed in most configurations. If deployment fails due to a docker authorization issue, populate this element with the auth field from the docker configuration json, otherwise leave an empty string as its value.

## Environment Configuration

The Domain Manager and Python sandbox invoke EKS commands directly by using `aws` and `kubectl` commands.
The environment for the user account exercising either the Domain Manager process or the Python sandbox session must be configured to successfully interact with the desired EKS cluster.
This configuration includes the application of whatever authorization certificates are necessary.

The configuration should include the hidden directories `~/.aws` and `~/.kube`.
These directories contain information like user credentials and EKS cluster information.

## Networking
Docker uses a bridged network by default by running the containers with the `--network host` option.

This configuration is ideal for enabling your containers to communicate with running REDHAWK services on the same system.

Communications between Pods (individual component instances) in the EKS cluster is simple in most EKS configurations.
If the Domain Manager or Python sandbox session reside outside the EKS cluster, it may be necessary to change the host computer's firewall session to allow communications to initiate from the EKS cluster's Pod instances to the host computer.


