# Domain Manager

The <abbr title="See Glossary.">Domain Manager</abbr> is in charge of the control and configuration of the entire systems <abbr title="See Glossary.">domain</abbr>.

Its primary responsibilities can be grouped into three main categories:

  - Registration
  - Core Framework (CF) administration
  - Human Computer Interfacing

Each domain has a single Domain Manager that keeps track of a <abbr title="See Glossary.">File Manager</abbr>, a set of <abbr title="See Glossary.">Device Managers</abbr>, and a set of <abbr title="See Glossary.">Application Factories</abbr>. The Domain Manager maintains information on all aspects of the <abbr title="See Glossary.">waveform's</abbr> implementations contained within its system.

The Domain Manager is configured from the Domain Manager Configuration Descriptor (DMD) XML file that is located at `$SDRROOT/dom/Domain`. This file contains the domain's name, an ID and a description of the domain.

### Launching a Domain Manager from the command line

Enter the following commands to launch a Domain Manager from the command line:

```bash
nodeBooter -D
INFO:DomainManager - Starting Domain Manager
INFO:DomainManager - Starting ORB!
```

### Registration

The Domain Manager is able to maintain information about all working parts and interactions in the environment through registration. It can be thought of as the Domain Manager's responsibility of bookkeeping for the system.

Any time a Device Manager, <abbr title="See Glossary.">device</abbr>, <abbr title="See Glossary.">service</abbr>, or <abbr title="See Glossary.">application</abbr> is created within the system, it is registered through the Domain Manager. Likewise, when they are destroyed, they are unregistered from the Domain Manager. These tasks are handled through a series of `register()` and `unregister()` functions for each of the different modules the Domain Manager is responsible for supporting.

### Core Framework Administration

The Domain Manager has CF administrative duties that are required to provide interface access to its registered items. The API of the Device Managers, ApplicationFactories, applications, and the FileManager that are registered in the domain are made available to be accessed from an external piece of software.

This is made available so that changes can be made from outside of the running domain. A Python module is distributed with REDHAWK that allows for simple interfacing with a running domain. This allows for [runtime inspection](../runtime-inspection/_index.html) and tweaking of the environment.

### Human Computer Interfacing

The Domain Manager is also responsible for providing functionality that allows for simple interaction between the user and the system, granting the user control over the running domain. Functionality exists that provides the ability to configure the domain, get its current device, service and application capabilities and launch maintenance functions.

Capabilities are managed through a series of data structure sequences. Lists are maintained for all services, Device Managers and devices that have been registered with the domain. Each entry in the list contains name and identification information as well as a reference to the object that has been registered. For any application that has been installed, a reference to its Application Factory is stored in a list along with its name and identification information. Once the application has started, its reference, along with all relevant identification, <abbr title="See Glossary.">component</abbr>, connection and ordering information is stored for later retrieval.

### Logging

By default the Domain Manager will output messages to the console. You can provide a custom logging configuration file and logging level using the following options:

```bash
nodeBooter -D -endlogcfgfile <config file>  -enddebug <5=TRACE,4=DEBUG,3=INFO,2=WARN,1=ERROR,0=FATAL>
```

This file location will be used as the default value when resolving the `LOGGING_CONFIG_URI` as well as the `DEBUG_LEVEL` during deployments. For further details about logging configuration files and `LOGGING_CONFIG_URI` resolution, consult [Logging](../logging/_index.html).

To override the default resolution of the `LOGGING_CONFIG_URI` during deployments, the Domain Manager can be launched with an option to use a [plugin library](../appendices/logging-config-plugin.html) (`libossielogcfg.so`) for this resolution. To activate this feature launch the Domain Manager as follows:

```bash
nodeBooter -D --useloglib
```

### Persistence Store

A unique feature of the Domain Manager is the ability to recover from catastrophic failures through domain persistence.
In order to make use of persistence, the CF must be compiled with support enabled.
As of REDHAWK 2.2.1, this is the default build setting.
To disable this support when building the CF, refer to [Installing the Framework from Source](../appendices/source-installation.html#installing-the-framework-from-source).

With this feature enabled, all bookkeeping data structures that are used to maintain information about services, devices, Device Managers, applications, and Application Factories are written to a database whenever any change is made to them. This database file needs to be specified upon launch of the Domain Manager with the `--dburl <file path>` argument:

```bash
nodeBooter -D --dburl $SDRROOT/dom/persistence.sqlite
```

> **NOTE**  
> The `--dburl` argument only applies to the Domain Manager.  
> It is an error to provide `--dburl` when only launching a Device Manager.

During normal operation, all IDs and references to objects within the Domain are stored. If the Domain Manager fails, objects such as components, devices, services, and Device Managers continue to run because they are processes separate from the Domain Manager process. This separation allows the Domain Manager to be relaunched. Once relaunched, the Domain Manager is returned to its last known good state by reloading the database file passed as the `dburl` argument to `nodeBooter`. All connections to Device Managers, devices, services, and components are restored; Applications residing in the Domain Manager process space are also restored.

#### System-Wide Reset

When persistence is enabled and the Domain Manager suffers a catastrophic failure, the data storage functions may lose synchronization. (For example, the <abbr title="See Glossary.">Naming Service</abbr> may be out of sync with the object database.) If this occurs, it is necessary to erase the database, clean out the <abbr title="See Glossary.">Event Service</abbr>, and clean out the Naming Service. In such instances, the Device Managers that are operating in the system, which may span multiple computers, also need to be reset to an idle state.

To deal with such instances, each Device Manager contains a separate thread that checks to see if the Domain Manager contains a reference to this Device Manager. If the reference is missing, the Device Manager assumes that the Domain Manager has been restarted to a blank state, and the Device Manager resets itself (as well as all devices and services that it controls). After resetting itself, the Device Manager re-associates with the Domain Manager. The frequency with which the Device Manager checks with the Domain Manager is controlled by the Device Manager <abbr title="See Glossary.">property</abbr>, `DOMAIN_REFRESH`. This property's default value is 10 seconds but can be set to any system-appropriate setting.
