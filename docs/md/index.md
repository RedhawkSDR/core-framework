# REDHAWK Manual - 3.0.1

REDHAWK is a software framework designed to support the design, development, deployment, management, upgrade, and recycling of real-time distributed applications as well as the systems that run them.

To support the design and development of software applications, REDHAWK provides tools that allow development and testing of software modules, or <abbr title="See Glossary.">components</abbr>.
In addition, REDHAWK provides tools to facilitate composure of components into
<abbr title="See Glossary.">waveforms</abbr> that can be seamlessly deployed as
<abbr title="See Glossary.">applications</abbr> on a single host computer or a network-enabled system of computers.

Deployment, management, and upgrade of real-time distributed applications is supported by providing a runtime environment.

The runtime environment can:

  - Deploy components to different computers on a network.
  - Support processing hardware hot-swapping.
  - Manage colliding software dependencies.
  - Manage constrained/specialized hardware resources.
  - Reduce the configuration burden on remote computing hardware.
  - Coordinate the sharing of limited hardware resources between different applications.

Finally, REDHAWK supports the recycling of applications by establishing strong boundaries between processing stages and providing an integration path for existing libraries into the REDHAWK infrastructure.

### Benefits of Using REDHAWK

REDHAWK provides the following benefits when used in a computing system:

  - Defines patterns for integrating existing libraries into a common framework.
  - Enables seamless deployment of software applications to one or more computing resources.
  - Decouples specialized hardware from processing algorithms; this allows processing algorithms to be easily ported to new platforms.
  - Supports language agnosticism, allowing appropriate languages to be used for various aspects of the system.
  - Decouples processing software from the UI, allowing for any number of custom UIs to operate with the same underlying Core Framework (CF).
  - Supports metadata-tagging of data streams.
  - Supports precision-time-stamping of data API.
  - Provides a powerful and flexible IDE based on the extensible Eclipse Framework.
  - Supports dynamic inter-connection of modules, allowing redirection of data flow during runtime.
  - Provides a data transport mechanism optimized for signal processing applications.

### What Systems May Benefit from Using REDHAWK?

A hardware/software system may benefit from the use of REDHAWK if it:

  - Deals with multiple specialized hardware platforms but with a single software application.
  - Integrates multiple disparate libraries into a single solution space.
  - Emphasizes signal processing development rather than system software development.
  - Distributes its software algorithms to more than one piece of hardware.
  - Partitions development between geographically-separated teams.
  - Supports shifting support work from the development team to a support team.
  - Supports shifting deployment work from the development team to a deployment team.

### Relationship to the Software Communications Architecture (SCA)

REDHAWK adopts a significant number of concepts from the SCA (specifically, version 2.2.2). As a result, the SCA specification is a very useful piece of supplemental reading to the REDHAWK documentation.

### Overview of this Document

REDHAWK is infrastructure that enables for the distribution of processing elements over an arbitrary number of computers connected to a network. These processing elements can be associated with different types of hardware (i.e.: data acquisition) to ingest/egress data, or to leverage some specialized coprocessors. The REDHAWK framework provides tooling for the creation, deployment, and management of these processing elements and hardware interaction elements.

The basic processing element in REDHAWK is a component, which is described in detail in *Components* and *Component Structure*. A component is a single linux process. Components interact with each other through connections, which are described in *Connections*.  An application, described in *Waveforms*, is a logical association of interconnected components.

Hardware interaction elements are called devices. Devices are described in Working with Devices. Devices can be logically associated together in nodes, which are described in *Nodes*. A specialized device called `GPP` is used to model the processing availability of a single computer. Each node contains no more than one `GPP`, and is generally mapped to a single computer. By deploying components through different `GPP` devices, an application can be automatically distributed over multiple computers. The distributed processing aspects of REDHAWK are discussed in *Distributed Computing and RF Devices*).

Components can either be deployed through a distributed environment called a domain, as seen in *The Runtime Environment*, or standalone through a sandbox. The sandbox environment is explained in *Sandbox*.

REDHAWK contains code generators and visualization tools. Descriptions of this tooling are available throughout this document.
