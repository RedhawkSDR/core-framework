# Component Model Overview 

REDHAWK 2.0 C++ Components
--------------------------

Historically in REDHAWK, a C++ component is an executable which contains
both the component-specific implementation class (e.g., "TuneFilterDecimate\_i") and a small `main()` driver that creates and executes the component.

![component_exe](/docs/shared-address/images/component_exe.png)

Most of the actual work of `main()` is done inside of the core libraries, in the `Resource_impl::start_component()` function, so the generated `main.cpp` contains very little code.

Shared Address Space C++ Components
-----------------------------------

In order to support creating arbitrary components in the same address space, the creation of the component is separated from the management of the process.

![component_so](/docs/shared-address/images/component_so.png)


The component code provides a factory function, `make_component()`, that implements creation of an instance of the specific component (this is a subset of what `main()` used to do). The rest of the component implementation remains the same.

A new meta-component handles the normal `main()` responsibilites, and provides an interface for the system to create a component inside its process space. This is called the ComponentHost.

REDHAWK 2.0 Process Model
-------------------------

The classic REDHAWK model is a 1:1 mapping of processes to components.

![current](/docs/shared-address/images/current.png)

The component's executable is either run by the ApplicationFactory via the GPP, or directly within the Sandbox/Chalkboard. Either does a fork-and-exec; the component executable creates a single instance of the component, which then registers back with the ApplicationFactory or Sandbox.

Shared Address Space Process Model
----------------------------------

The way to remove IPC costs is, naturally, to do away with the P (at least in part). This means a 1:N process-to-component mapping.

![shared_address](/docs/shared-address/images/shared_address.png)

The Sandbox/Chalkboard or ApplicationFactory executes the component host, then launches the components into the host. The component host is capable of dynamically loading and unloading components and their dependencies; this allows the set of components running in the component host to change arbitrarily at run-time. The Sandbox benefits from this behavior now, since it doesn't know the future. Applications, on the other hand, do not support dynamic deployment *yet*.

Mixing Shared Libraries and Executables
---------------------------------------

The shared address space model does not affect existing components. Executable-based C++ components will still compile and run as-is; however, without updating and re-generating a component, it will not be able to take advantage of shared address space benefits.

![mixed](/docs/shared-address/images/mixed.png)

The ability to generate executable components is not going away. Although I would expect it to be rare, there may be particular reasons why a given component cannot be run in a shared address space–perhaps due to the requirements of an external library, for example.

Process Per Component
---------------------

While the intent is to run all shared library components from an application in a single component host in order to take advantage of the benefits of memory sharing, the design does not require it (1 is a perfectly reasonable N in 1:N).

![process_per_component](/docs/shared-address/images/process_per_component.png)

Each component can be launched in its own component host, giving the exact same behavior as REDHAWK 2.0 and prior. Or, a few troublesome components could be isolated from the rest by running them in their own component hosts, while the rest of the components share a host.

A further enhancement could allow an application designer to choose arbitrary groupings of components to be run in a set of component hosts, though I have my doubts as to how valuable this is when balanced against the complexity.
