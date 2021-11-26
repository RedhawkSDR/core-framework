# Further Reading

The REDHAWK manual explains the use of REDHAWK to build, deploy, and manage data streaming applications. The principal REDHAWK features are outlined in the following sections, and a reference to the corresponding REDHAWK documentation is provided for further reading.

### References for Application Developers

The following chapters are particularly useful for application developers:

  - **Component development** is introduced in [Components](../Components/components.html). Greater detail related to component development is discussed in the following chapters:

      - [Component Structure](../Component-Structure/_index.html)

      - [Connections](../Connections/_index.html)

      - [Logging](../Logging/_index.html)

  - **Waveforms**, including a demonstration of creating a REDHAWK waveform and launching it as an application using the IDE, are discussed in-depth in [Waveforms](../Waveforms/_index.html)

  - **The sandbox**, which is used to run components on a local host without any additional runtime infrastructure such as the Domain Manager, is described in-depth in [Sandbox](../Sandbox/_index.html)

## References for System Developers

The following chapters are useful to system developers:

  - Managing and interacting with hardware through [**Devices**](../Devices/_index.html)

  - **Devices and Device Managers** make up individual nodes, which are used to deploy and manage devices in a REDHAWK system. Devices are used to determine whether or not a host can deploy any given component. Devices and Device Managers are discussed in-depth in [Devices](../Devices/_index.html), [Nodes](../Nodes/_index.html), and [The Runtime Environment](../Runtime-Environment/_index.html).

  - **The Domain Manager and Device Manager** are the foundation for [The Runtime Environment](../Runtime-Environment/_index.html) for the deployment of distributed applications. [Runtime Environment / Inspection](../Runtime-Environment/inspection.html) describes additional tooling.

  - [**Services**](../Services/_index.html), are programs that provide some system-specific always-on software support to components. An example of a service is a web server.
