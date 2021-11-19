# Process Management and Interaction

This chapter addresses the basic level of decomposition and deployment for REDHAWK and the middleware used to support communications between basic functional units.

## Process Management

The basic functional unit in REDHAWK is a component, which represents a single process on a host computer. The component supports the interfaces necessary to initialize, configure, query, test, connect to other components, and terminate the component. It also manages a processing thread which contains the component's functionality and the buffering of input/output data. Components can be written in C++, Python, or Java.

A waveform is a logical collection of components that are to be deployed as an application onto a REDHAWK system and is defined in an XML file. A waveform allows a developer to create algorithms composed of components. The composition of algorithms as separate processes allows REDHAWK to appropriately deploy these components into a network environment. REDHAWK supports distributed computing by finding an appropriate host for a component, deploying the component to that host, and managing that component once it is running.

## Data Transfer

Data exchange across a network is integral to REDHAWK's core functionality. Managing the exchange of data is handled through "middleware", which is a sophisticated software infrastructure that provides a common language for the efficient transfer of data between arbitrary languages over arbitrary media. The middleware selected for REDHAWK is omniORB, an implementation of the CORBA (Common Object Request Broker Architecture) specification. REDHAWK uses omniORB because it provides substantial technical benefits over other middleware implementations.

The primary benefits of using omniORB include:

  - omniORB is a small package that is easy to build and install. It supports Python and C++ by default and was easily expandable to support Java.
  - omniORB data transfers are very efficient. The pluggable nature of omniORB's communication mechanism allows the use of multiple underlying transport protocols that can be tailored based on the deployment environment. For example, components that are located on the same host use Unix domain sockets for the transfer of data as an alternative to IP-based communications.
  - CORBA handles the data translation between different host types (e.g., 32-bit versus 64-bit systems and big endian versus little endian).
  - CORBA supports the Any type, allowing generic compatibility between interfaces.
  - omniORB implements the open standard CORBA, allowing system developers to use whatever other implementation of CORBA they may want to use for their infrastructure to interact with the underlying REDHAWK system.

The technical benefits of omniORB come at the cost of CORBA's awkward language mapping. This disadvantage is addressed in REDHAWK by mapping CORBA constructs to native language types through code generators and base classes, thus alleviating the burden of CORBA's complexity from the REDHAWK user.

In conclusion, omniORB is a simple and efficient middleware package that allows programs in C++, Python, or Java to interact with each other. The disadvantages from CORBA are mitigated by the REDHAWK framework, while CORBA's inherent benefits, such as platform independence, generic type support, strongly-typed interfaces, and open standard, bring powerful features to REDHAWK.
