# Glossary

Allocation Manager
  : An implementation of the `AllocationManager` interface, the Allocation Manager provides a central access point for all allocations performed in the domain.

Application
  : "Generically, an executable software program which may contain one or more modules. Within Software Communications Architecture (SCA), an application consists of one or more software modules which implement the base `Application` interfaces and which are identified within a Software Assembly Descriptor file. When loaded and executed, these modules create one or more components which comprise the application. " Source: SCA v2.2.2.

Application Factory
  : "An instantiation of the `ApplicationFactory` interface is used to create an instance of an application. The Domain Manager creates an application factory for each Software Assembly Descriptor that is installed. " Source: SCA v2.2.2.

Assembly Controller
  : "The assemblycontroller element of the Software Assembly Descriptor indicates the component that is the main resource controller for an application. " Source: SCA v2.2.2.

Component
  : A processing module that implements the `CF::Resource` interface that is deployable by an Application Factory (refer to SCA v2.2.2 for a description of `CF::Resource`).

Connection Manager
  : An implementation of the `ConnectionManager` interface, the Connection Manager provides a central access point for connections between domain objects.

Console View
  : Displays a variety of console types depending on the type of development and the current set of user settings. The Console view is a part of Eclipse, and the basic use is well documented by the Eclipse documentation: https://help.eclipse.org. The REDHAWK IDE uses multiple Console views for different purposes. There are also third party plug-ins within REDHAWK that have their own Console views.

CORBA Name Browser
  : Maps names to specific CORBA Servants. The CORBA Name Browser can be used to examine the current contents of the Naming Service as well as perform basic manipulation of that context. The contents of the view displays all currently bound name contexts (folders) and objects.

Device
  : "The SCA defines a `Device` interface class. This interface is an abstraction of a hardware device that defines the capabilities, attributes, and interfaces for that device." Source: SCA v2.2.2. In the context of REDHAWK, a device is is a software module that implements the `Device` interface class.

Device Manager
  : An implementation of the `DeviceManager` interface. The Device Manager is responsible for the creation of devices and services.

Domain
  : "A Domain defines a set of hardware devices and available applications under the control of a single Domain Manager." Source: SCA v2.2.2.

Domain Manager
  : "An implementation of the `DomainManager` interface, a Domain Manager manages the complete set of available hardware devices and applications. It is responsible for the set-up and shut-down of applications and for allocating resources, devices, and non-CORBA components to hardware devices." Source: SCA v2.2.2.

Error Log View
  : An Eclipse view that provides details of errors that have occurred with the running application.

Event Channel
  : A software object that mediates the transfer of CORBA events between producers and consumers.

Event Channel Manager
  : An implementation of the `EventChannelManager` interface, the Event Channel Manager provides a central access point for the management of event channels in the domain.

Event Service
  : Software that provides and manages event channels.

File Manager
  : An implementation of the `FileManager` interface, the File Manager provides a central access point for all of the file systems available in the domain.

GPP
  : General Purpose Processor Device that manages a computing node.

Hardware-Accelerated Component
  : A REDHAWK component that has the ability to access/maintain a designated portion of programmed hardware (for example, a single modem in a multi-modem load).

Message
  : Key/value pairs contained in a single instance of the `CORBA::Any` type and passed over the CORBA event API.

Naming Service
  : Software that provides a mapping between human-readable names of CORBA objects and the CORBA objects themselves.

NIC
  : The network interface card, usually interfacing with Ethernet.

Node
  : A single Device Manager, its associated devices, and its associated services.

Node Editor
  : The Node Editor is opened by double-clicking a Device Configuration Descriptor (DCD) file from the Project Explorer view. It presents all the content that can be found within the dcd.xml file in an editing environment designed for human use. The Node Editor contains an Overview, Devices, Diagram, and raw XML tab, which contains the DCD file content.

NUMA
  : Non-Uniform Memory Access.

Outline View
  : Displays an outline of a structured file that is currently open in the editor area. The Outline view is a part of Eclipse, and the basic use is well documented by the Eclipse documentation: <https://help.eclipse.org>.

Persona
  : The load or programming files used to load functionality onto programmable hardware (for example, bit files).

Persona Device
  : A REDHAWK device that encapsulates a single load for specific programmable hardware and any corresponding ports/properties (defined in the REDHAWK XML descriptor files).

Perspective
  : Each workbench window contains one or more perspectives. A perspective defines the initial set and layout of views in the Workbench window. Within the window, each perspective shares the same set of editors. Each perspective provides a set of functionality aimed at accomplishing a specific type of task or works with specific types of resources. For example, the Java perspective combines views that you would commonly use while editing Java source files, while the Debug perspective contains the views that you would use while debugging Java programs. As you work in the workbench, you will probably switch perspectives frequently. Perspectives control what appears in certain menus and toolbars. They define visible action sets, which you can change to customize a perspective. You can save a perspective that you build in this manner, making your own custom perspective that you can open again later. Source: <https://help.eclipse.org>.

Port
  : A CORBA object that produces or consumes data and/or commands. A port is referred to as "uses" when it is a source/producer/output, and as a "provides" when it is a sink/consumer/input. A uses port implements the `CF::Port` interface (see SCA v2.2.2 for a description of `CF::Port`).

Problems View
  : As you work with resources in the workbench, various builders may automatically log problems, errors, or warnings in the Problems view. The Problems view is a part of Eclipse, and the basic use is well documented by the Eclipse documentation: <https://help.eclipse.org>.

Programmable Device
  : A REDHAWK device that is the main proxy into physical hardware. It maintains which Persona (child) device should be loaded onto the hardware.

Programmable Hardware
  : Hardware that may be dynamically programmed to operate with specific functionality (such as FPGAs and some microprocessors).

Project Explorer View
  : Provides a hierarchical view of the resources in the workbench. The Project Explorer view is a part of Eclipse, and the basic use is well documented by the Eclipse documentation: <https://help.eclipse.org>.

Properties View
  : Displays property names and basic properties of a selected resource. The Properties view is a part of Eclipse, and the basic use is well documented by the Eclipse documentation: <https://help.eclipse.org>.

Property
  : An SCA Property is a variable that contains a value of a specific type. Configuration Properties are parameters to the configure and query operations of the `PropertySet` interface. Allocation Properties define the capabilities required of a Device by a Resource." Source: SCA v2.2.2.

PyDev
  : PyDev is a third-party plug-in for Eclipse. It is an IDE used for programming in Python supporting code refactoring, graphical debugging, code analysis and many other features.

REDHAWK Explorer
  : The REDHAWK Explorer, an application built on the REDHAWK Core Framework, is used to navigate the contents of a REDHAWK domain. It provides capabilities for viewing the contents of the domain, configuring domain resources, and launching domain waveforms.

REDHAWK Explorer View
  : Allows a user to navigate the contents of a REDHAWK domain. It provides capabilities for viewing the contents of the domain, configuring instantiated resources, and launching applications in a target Software-Defined Radio (SDR) environment. It also provides access to the sandbox, which is an environment for running components and applications without a Domain Manager and Device Manager.

Sandbox
  : A Python environment that can be used to easily run and interact with components without creating a waveform or running a Domain Manager.

SCA Waveform
  : Deprecated. Refer to Waveform.

Service
  : Software made available by the Device Manager at the Device Manager startup.

SoftPkg Editor
  : The SoftPkg Editor is opened by double-clicking a Software Package Descriptor (SPD) file from the Project Explorer view. It presents all the content that can be found within the spd.xml file in an editing environment designed for human use. If the SPD file references a Properties File (PRF) or Software Component Descriptor (SCD) file, additional tabs are made available that represent these files in similar fashion.

Target SDR
  : The Target SDR refers to the REDHAWK resources that your workspace will be built and run against. It describes the platform for which you are developing. By default, it points to the file location specified by the environment variable SDRROOT.

`usesdevice` relationship
  : A logical association between a component and a specific device that provides some capacity to that component.

View
  : A view is a workbench part that can navigate a hierarchy of information or display properties for an object. Only one instance of any given view is open in a workbench page. When the user makes selections or other changes in a view, those changes are immediately reflected in the workbench. Views are often provided to support a corresponding editor. For example, an outline view shows a structured view of the information in an editor. A properties view shows the properties of an object that is currently being edited. Source: <https://help.eclipse.org>.

Waveform
  : A REDHAWK waveform is analogous to an SCA Waveform Application. A REDHAWK waveform is defined as a composition of components, their interconnections, and their configuration overrides. This composition is defined in a Software Assembly Descriptor file.

Waveform Editor
  : The Waveform Editor is opened by double-clicking a Software Assembly Descriptor (SAD) file from the Project Explorer view. It presents all the content that can be found within the sad.xml file in an editing environment designed for human use. The Waveform Editor contains an Overview, Diagram, and raw XML tab, which contains the SAD file content.

Workbench
  : The term workbench refers to the desktop development environment. The workbench aims to achieve seamless tool integration and controlled openness by providing a common paradigm for the creation, management, and navigation of workspace resources. Source: <https://help.eclipse.org/>.

Workspace
  : In Eclipse, a workspace is a logical collection of projects on which you are actively working. Source: <https://help.eclipse.org>.
