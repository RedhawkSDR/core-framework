# Nodes

In [The Runtime Environment](../Runtime-Environment/_index.html), the <abbr title="See Glossary.">Device Manager</abbr> is introduced along with its primary role: to deploy and manage device proxies in whatever host the Device Manager has been launched. The term <abbr title="See Glossary.">node</abbr> is used to refer to the Device Manager and its associated <abbr title="See Glossary.">devices</abbr> and <abbr title="See Glossary.">services</abbr>.

The concept of a device and its relationship with the deployment of a set of <abbr title="See Glossary.">components</abbr> in an <abbr title="See Glossary.">application</abbr> is one of the more complicated concepts in REDHAWK. A device is a program that the <abbr title="See Glossary.">Domain Manager</abbr> interacts with to determine whether or not a component can run on the host where that device is located. If it is determined that the device can host that component, the device provides the Domain Manager with the API necessary to load the component binaries over the network and execute them. In short, the device provides the remote host discovery mechanism to the Domain Manager and the mechanism needed to load and execute whatever component files are needed to run the component on the device host.

This chapter provides a description of how to interact with devices and nodes using the REDHAWK IDE.

