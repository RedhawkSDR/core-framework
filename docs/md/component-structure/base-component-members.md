# Base Component Members

This section provides an overview of members available to the <abbr title="See Glossary.">component</abbr> class. There are four kinds of members: <abbr title="See Glossary.">ports</abbr>, <abbr title="See Glossary.">properties</abbr>, domain awareness, and network interface.

### Ports

Data flow into and out of components is accomplished through the use of ports. Ports are described as being either a **provides** (input) or **uses** (output) port. This naming convention is often viewed as counter-intuitive, so an explanation is in order. Ports are RPC interfaces to a component. An input port, therefore, **provides** functionality that can be **used** by an output port.

REDHAWK contains a variety of standardized interfaces that facilitate interoperability. These interfaces are implemented by ports. When a port is selected in the component generation wizard in the REDHAWK IDE, code to implement these interfaces is automatically generated.

Irrespective of direction, a port is accessed as a member of the component's base class. Assuming that a port called `myport` of any interface exists in the component, it is accessed in the following ways in C++, Python, and Java, respectively:

```cpp
this->myport
```

```python
self.port_myport
```

```java
this.port_myport
```

See [Standardized Data Interfaces](../connections/standardized-data-interfaces.html) for details on how to use ports for sending or receiving data.

### Properties

Much like ports, properties are available to the component through generated members to for the component's base class. The property is found through the property's name (if it has one) or its ID. For example, if a property is defined with an ID of `foo` and a name of `abc`, it would be accessed in the following ways in C++, Python, and Java, respectively:

```cpp
this->abc
```

```python
self.abc
```

```java
this.abc
```

If the property does not have a name defined, then it would be accessed in the following way in C++, Python, and Java, respectively:

```cpp
this->foo
```

```python
self.foo
```

```java
this.foo
```

Note that no automated check is performed on the code generation to avoid a name collision between properties or ports.

### Enumerations

`simple` properties can have enumerated values, which associate symbolic names with values. Code generation creates constants for these values, allowing the component developer to use the symbolic name instead of the literal value. For `simple` properties in `struct` or `struct sequence` properties, the generated constants are nested under the name of the struct.

#### C++
In C++, the generated constants for enumerations are static variables in nested namespaces, under the top-level namespace `enums`:

```cpp
enums::simple::LABEL
enums::structprop::field::LABEL
enums::structseq_struct::field::LABEL
```

Enumerated values for `simple` properties are in the component base class header, while those in `struct` or `struct sequence` properties are in `struct_props.h` along with the struct definitions.

#### Java
In Java, the generated constants for enumerations are public static variables in nested static classes, under a top-level class named `enums`:

```java
enums.simple.LABEL
enums.structprop.field.LABEL
enums.structseq_struct.field.LABEL
```

The `enums` class is a static nested class in the component base class.

#### Python
In Python, the generated constants for enumerations are class attributes in nested classes, under a top-level class named `enums`:

```python
enums.simple.LABEL
enums.structprop.field.LABEL
enums.structseq_struct.field.LABEL
```

The `enums` class is imported from the component base class module.

### Domain Awareness

Each component has two members that provide a reference to the <abbr title="See Glossary.">domain</abbr> and <abbr title="See Glossary.">application</abbr> in which the component is operating. To retrieve the <abbr title="See Glossary.">Domain Manager</abbr> and Application, access the member functions `getDomainManager()` and `getApplication()`, which return a `DomainManagerContainer` and `ApplicationContainer`, respectively. `DomainManagerContainer` has the member `getRef()`, which returns the CORBA pointer to the Domain Manager object. `ApplicationContainer` has the member `getRef()`, which returns the CORBA pointer to the Application object.

In the case of <abbr title="See Glossary.">devices</abbr>, instead of `getApplication()`, the base class contains `getDeviceManager()`, which returns a `DeviceManagerContainer`. The `DeviceManagerContainer` has the member `getRef()`, which returns the CORBA pointer to the `DeviceManager` object.

### Network Interface

If a component contains a dependency against any member of <abbr title="See Glossary.">GPP</abbr>'s `nic_allocation` allocation property, then the framework will, upon deployment, make sure that those network resources are made available to the component. Whichever <abbr title="See Glossary.">NIC</abbr> statisfies the allocation requirement is fed to the component, and it is made available to the developer through the `getNetwork()` member, which returns a `NetworkContainer`. `NetworkContainer` has the member function `getNic()`, which is the string name of the NIC that satisfied the requirement (i.e.: eth0).

Note that if the network dependency is declared for any one component, that component's deployment is made to the core(s) that are closest to the NIC on the processor. This happens automatically with no need for additional input from the deployer.
