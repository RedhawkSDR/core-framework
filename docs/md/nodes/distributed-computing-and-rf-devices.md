# Distributed Computing and RF Devices

### Dependencies

A <abbr title="See Glossary.">component</abbr> can be described as [consuming an arbitrary amount of capacity from a device](running-a-node.html#creating-a-component-that-consumes-resources); this relationship is called a **dependency**. Dependencies are generalized, so it is possible to create a dependency based on attributes (<abbr title="See Glossary.">property</abbr> of kind allocation) of <abbr title="See Glossary.">devices</abbr> irrespective of the specific nature of a device.

A dependency is defined by how much of a particular device resource is required. For example, a component dependency could be a requirement of 1.7 units of device property **some_id**. Testing this description requires the runtime environment to only determine whether a device contains an allocation property with the ID **some_id**; this concept can be generalized to any physical constraint that a device might have, requiring only a convention between the component developer and the device developer regarding a device's property ID and units, both of which are publicized in a device's Properties File (PRF)  file (`prf.xml`).

### Allocation Properties

Allocation properties are device properties that are associated with an action and are used by the runtime environment to determine whether or not a device can support a particular component dependency. This action can either be **external** or some logical operation (e.g., **eq** (equals) or **lt** (less than)).

#### Non-External Properties

Non-external properties are evaluated by the runtime environment without querying the device. For example, a component dependency on an allocation property with operation **eq** might be *blue*. The runtime environment then checks the device's PRF file (`prf.xml`) and the <abbr title="See Glossary.">Device Manager's</abbr> configuration file for the deployment of that device (`dcd.xml`) to determine whether that property's value is equal to *blue*.

#### External Properties

External properties, on the other hand, are handled by the device. For example, a component dependency on an allocation property with operation **external** might be *yellow*. The runtime environment then makes a function call on that device with two arguments: the allocation property ID and the value *yellow*. The device then, at its own discretion, determines whether to provide a positive response or a negative one. The algorithm used by the device to determine whether or not it has sufficient capacity to allocate against a value of *yellow* is left up to the device developer.

### Allocation Usage

Capacity allocation can be defined at either the component level or the <abbr title="See Glossary.">application</abbr> level. When defining the capacity allocation at a component level, that means that every time this component is deployed, that capacity must be available. When defining the capacity allocation at a application level, that means that every time that this application is deployed, that capacity must be available, irrespective of which components make up the application.

Even though capacity allocation can be used to describe any dependency that a component may have, in reality there are two kinds of capacities that concern REDHAWK: RF and computing.

#### RF Allocation

Although RF capacity dependencies can be declared at either the component level or the application level, it is recommended that the allocation be defined at the application level because the application understands the context for the deployment. REDHAWK defines RF allocations using the `FrontendInterfaces` specification. For more information, refer to [Frontend Interfaces](../appendices/fei.html) (FEI).

#### Computing Allocation

Determining capacity for computation is different from determining RF needs. RF needs are fairly straightforward (i.e.: I need to tune to 100 MHz with a 200 kHz bandwidth). Computing resources, on the other hand, can vary substantially. A program (i.e.: a component) consumes different amount of computing resources depending on the nature of the computing platform. Things like L2 cache size and the layout of the processor (from a <abbr title="See Glossary.">NUMA</abbr> perspective) can have a dramatic impact on the computational load a program has on the processor.

Due to the variability intrinsic to computing resources, REDHAWK does not rely on hardcoded estimates for the selection of a computing platform. Instead, the <abbr title="See Glossary.">GPP</abbr> implements a reservation mechanism. When a component is loaded onto the GPP, the GPP reserves an arbitrary, and tunable, amount of resources. GPP tracks the state of the component, and when the component changes state from stopped to started, the reservation is tabled, and GPP inspects the actual usage by the component. When the state of the component changes from started to stopped, the tabled reservation is applied again.

Using this reservation/observation behavior, components can be automatically distributed over an arbitrary number of computers with no planning needed on the part of the user.

The user can force components to deploy to specific GPPs. To force a specific placement, use the device assignment fields when creating the application.

#### The Deployment Process

The runtime environment scans all non-busy executable devices registered in the <abbr title="See Glossary.">domain</abbr> to determine which GPP matches the processor/operating system dependency. A device is busy when its state is returned as **BUSY**, otherwise it is either **IDLE** or **ACTIVE**. When a device is found that satisfies all component dependencies, it marks that device as assigned to the deployment of that component and moves on to whatever other components make up the <abbr title="See Glossary.">waveform</abbr>. Once all components have been found an assigned GPP, the components are deployed to all those GPPs.

#### Binding a Component's Deployments to Executable Devices

When deploying components, you may want to bind a component's deployment to a specific executable device without going through the allocation process. The <abbr title="See Glossary.">node</abbr>'s definition file, the Device Configuration Descriptor (DCD), provides the necessary `deployerrequires` id/value pairs to be associated with an executable device. These id/value pairs are alphanumeric strings that are matched during a component's deployments. If a component defines a set of `devicerrequires`, then the domain is searched for an `ExecutableDevice` that has a matching set. For components that do not have an id/value set, but request to deploy on an `ExecutableDevice` with an id/value set, that request for deployment fails.

The following example describes the new xml elements `deployerrequires` that can be assigned to a GPP device (`ExecutableDevice`):

```xml
<!-- example of ID/value pairs for a GPP in a DCD file
<componentplacement>
  <componentfileref refid="GPP1_SPD_1"/>
  <componentinstantiation id="RED_NODE:GPP_1">
     <usagename>RED_NODE:GPP_1</usagename>
     <deployerrequires>
         <requires id="color" value="RED"/>
         <requires id="rank" value="15"/>
     </deployerrequires>
  </componentinstantiation>
</componentplacement>
```

#### Distribution of Files

All component binaries and descriptor files reside in `$SDRROOT/dom` on whatever host is running the <abbr title="See Glossary.">Domain Manager</abbr>. In every host that is running a Device Manager, a cache directory is created in `$SDRROOT/dev/.<Device Manager name>`, with a directory entry for each device that the Device Manager manages. When a component runs on any given device, the binary (or module or JAR file) is copied from `$SDRROOT/dom` to the device's cache directory. Using this mechanism, the device can start the component process on a remote host.

#### Dependency Management

A difficulty in deploying a component is that, as a program, it might have dependencies like C/C++ libraries, Python modules, or Java JARs. REDHAWK allows for the creation of soft package dependencies, where a library, module, or JAR can be associated with its own profile. Components that have a runtime dependency with this library, module, or JAR, can declare this library profile as a dependency. When the component is loaded over the network, this dependency is also loaded, and before the component is forked, the component's local running environment is changed to include the library in `$LD_LIBRARY_PATH`, module in `$PYTHONPATH`, or JAR in `$CLASSPATH`. The allocation/dependency requirements associated with the component are also applied to the library; for example, if a component is designed to run on an x86_64 platform, the runtime environment checks that dependency runs on an x86_64 platform.
