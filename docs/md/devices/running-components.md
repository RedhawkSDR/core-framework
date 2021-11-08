# Using Devices to Run Components

The <abbr title="See Glossary.">sandbox</abbr> runs <abbr title="See Glossary.">components</abbr> without needing a <abbr title="See Glossary.">device</abbr> proxy; it forks the component process and manages its lifecycle. When running in a <abbr title="See Glossary.">domain</abbr>, however, the deployment of components in an <abbr title="See Glossary.">application</abbr> requires the domain to search for available host computers that can run the components in the application. The search requires each host computer to have a program that can publicize the host computer's capabilities (for example, operating system, processor type, or available memory). This proxy is referred to as an executable device. REDHAWK includes a default executable device, the <abbr title="See Glossary.">GPP</abbr>, which is automatically configured by the `create_node.py` script whenever a new <abbr title="See Glossary.">node</abbr> is installed on the host computer. The GPP is written in C++ and can serve any node that supports x86 64-bit architecture. In cases where a more specialized proxy is required, REDHAWK includes base classes that can be extended in Python or C++. However, a detailed discussion of this process is beyond the scope of this document.

### Controlling the Cache and Working Directory

When a component is deployed by the GPP, it operates as a separate entity, either as a forked process or an operating thread (in the case of C++). Each component has a corresponding cached file or set of cached files and a working directory from which the program executes. The GPP manages the cached files and working directory settings through the `cacheDirectory` and `workingDirectory` <abbr title="See Glossary.">properties</abbr>, respectively. These properties can be reconfigured using the node's configuration file (DCD).