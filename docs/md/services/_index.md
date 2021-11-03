# Services

In the context of REDHAWK, infrastructure software is managed by the <abbr title="See Glossary.">Device Manager</abbr>. Infrastructure software can take two forms: hardware management and system <abbr title="See Glossary.">services</abbr>. System services are the equivalent to Linux services like an HTTP server; the service does not directly control hardware but is some facility that becomes automatically available when the host is started.

## Management

A service is managed through the Device Manager.  [Nodes](../nodes/_index.html) are described as a Device Manager instance and the set of <abbr title="See Glossary.">devices</abbr> and services associated with that Device Manager instance. When the Device Manager instance is created, it reads its configuration file (`dcd.xml`), which contains a list of all devices and services that the node contains. The devices and services are forked as individual processes and remain running until the Device Manager is shut down. Devices are bound to the <abbr title="See Glossary.">Naming Service</abbr> under the Device Manager's context. Services, on the other hand, are bound to the <abbr title="See Glossary.">domain's</abbr> naming context.

## Service API

REDHAWK does not include any standard service interfaces; the Software Communications Architecture (SCA) specification includes a logging service, but in REDHAWK, [logging](../logging/_index.html) is performed through `log4j`/`log4cxx`.

To implement a service, it should use one of the standard REDHAWK interfaces, or a specialized interface to support some specific behavior.  The specialized interface can be added using the [SPD editor](../ide/editors-and-views/softpkg-editor.html). The REDHAWK code generators provide the appropriate methods for the service's selected interface. It is the developer's task to implement the functionality of these methods. If the service is created with any of the following REDHAWK interfaces, LifeCycle: `PropertySet`, or `PropertyEmitter`, the same deployment behavior as used for a REDHAWK device is performed. That is, the Device Manager will try and call the service's `initializeProperties`, `initialize`, and `configure` methods with the appropriate parameters.

A service is created and torn-down by the Device Manager.  Because there is no defined interface for the service, services do not support the `LifeCycle` interface, and more importantly, the `releaseObject()` method. Because there is no release method, the Device Manager issues operating-system level signals to terminate the Service. It is the developer's task to perform whatever cleanup functionality is required in response to the receipt of a termination signal.

## Files Defining a Service

A service, much like a <abbr title="See Glossary.">component</abbr> or device, is a binary file and a set of XML descriptors, normally just the Software Package Descriptor (SPD) (describing the service's software package) and Software Component Descriptor (SCD) (describing the service's interfaces).  The service file package resides in `$SDRROOT/dev`, usually in a `services` subdirectory or the `devices` subdirectory.  More information can be found in the [SPD editor section](../ide/editors-and-views/softpkg-editor.html).

A service normally does not support interfaces necessary to initialize properties over CORBA. In order to support properties in services, a property can be defined for a service and set as command-line, allowing the property to be overloaded for the specific service instance. The resulting command line argument will resolve from the service's Properties File (PRF) file, and optionally be overridden from the Device Configuration Descriptor (DCD) file. Standard property processing will apply to all types with the exception of the boolean type; in which case the exact contents supplied in either the PRF or DCD file will be passed on the command line.

## Finding a Service

<abbr title="See Glossary.">Application</abbr> descriptor files (`sad.xml`) are described in [The Runtime Environment](../runtime-environment/_index.html); these files describe a logical collection of components that are deployed to support some system-level application. In the application descriptor file, connections between deployed entities can be described in a variety of ways. These connection descriptions are how an application, or more specifically a component in an application, can interact with a service.

The [**Find By** feature](../runtime-environment/applications.html#using-the-find-by-feature) enables users to find services through either their (globally unique) name or by the interface that they support. Using a globally unique name as the search key allows a developer to specify an single instance of a service to connect to. On the other hand, by searching for a service through its supported interface, a developer can create a generalized search for a service that satisfies a particular need, irrespective of how it is associated with the Domain.
