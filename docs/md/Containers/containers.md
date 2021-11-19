# Containers
REDHAWK 3.0.0 adds support to run components in Docker containers, either in docker or on container orchestration clusters.

Regarding container orchestration, the Python sandbox can select Docker or Kubernetes at runtime.
The Domain Manager can select Docker or Kubernetes, after installing a plugin.

Users can:

- build components into Docker containers
- run REDHAWK, or portions, in containers
- users can write plugins for other orchestration technologies

## Dependencies
General:
- REDHAWK >= 3.0.0

For Docker specific functionality:
- Docker >= 19.03.12

To extend the plugin class; specifically, for plugin yaml generation:
- yum package `yaml-cpp-devel`

For EksKube specific functionality:
- The [kubectl binary](https://kubernetes.io/docs/tasks/tools/install-kubectl-linux/#install-using-native-package-management) installed on your local REDHAWK system on your `$PATH`
- The [aws cli binary](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2-linux.html) installed on your local REDHAWK system on your `$PATH`

## Other Requirements
The following must be present in your environment in order to use RH components in clusters:
* A cluster running the desired orchestration technology
* The matching REDHAWK cluster plugin installed
* Networking, firewalls, and DNS setup to communicate with your cluster's control plane/nodes/API
* A local REDHAWK install with correct `$OSSIEHOME/cluster.cfg`

## Networking
Networking can get complicated quickly because it will vary depending on what you're trying to run in your cluster, what that cluster's networking setup looks like, and your local system's ability to send traffic to that cluster. Guiding rules of thumb are:

- If you are running DomMgr and/or GPP with OmniORB on your local system and only running waveforms on a cluster, those launched components need to be able to initiate connections to omniORB. This boils down to there being a "flat" network between where OmniORB runs and where the waveform runs. NAT will break one party's ability to talk to the other.
- In the scenario where all REDHAWK services (OmniORB, DomMgr, and/or GPP) run inside the cluster alongside the waveform payloads, so long as the containers can network resolve each other (almost always the case barring network security restrictions on the cluster), then there should not be any difficulties with networking.
