# Creating connections between components in FPGA.

In the context of REDHAWK, a component's implementation does not need to be tied solely to a single process or thread running on a microprocessor.
In instances where the component functionality is implemented on a different processor, like an FPGA, the role of the component process or thread is to manage the bitfile load, its properties, and its data ingress and egress.
In the case where the data ingress or egress is between components running in the same FPGA, the role of the components is to manage the FPGA load and provide command-and-control information to the embedded device, an example of which is where to push data to or retrieve data from.

Bulk IO allows for the addition of transports beyond those included by default. These transports are selected based on a priority, with CORBA's priority set to 99 and shared memory IPC set to 1; in the case of communications between components in the same process, the "local" transport is selected, which is defined as shared address space. The first code example shows how to create a new transport and apply it to links between components running in different processes. The second code example shows how to subclass a Bulk IO port and change the local transport to a custom one.

It needs to be noted that the custom transport is designed to interact with other Bulk IO ports. This means that in a component, if out-of-band communications as well as traditional microprocessor-based communications are supported, then data needs to be transferred both out-of-band and using the stream API in the microprocessor code. This option is impractical, but is necessary if the Bulk IO port is meant to support communications with other ports supporting the custom transport as well as traditional microprocessor-based software.

## Adding transports to Bulk IO

For the purposes of this example, suppose that there is a component called **transport_out** that has a single output *dataFloat* port (called **dataFloat_out**) and another component called **transport_in** that has a single input *dataFloat* port (called **dataFloat_in**). This example shows how to modify these components such that the data exchange occurs outside the scope of the built-in Bulk IO mechanisms, like an FPGA.

The pattern that is implemented in both the out (uses) and in (provides) side is to create a transport factory, which in turn is used to create a transport manager, which in turn creates the transport itself. The transport factory is instantiated globally, the transport manager is instantiated per port, and the transport is instantiated per connection. The Bulk IO base classes exercise this pattern.

In this pattern, the transport factories register statically with a transport registry. This transport registry contains all the transports that are supported. At transport negotiation time, the transport names are matched in order of their priority, from the lowest number to the highest. In this example, the "custom" transport is given a priority of 0, making it the highest priority transport, guaranteeing that it be selected if the transport is supported by both ends of the transaction.

Once a matching transport has been selected, an exchange of properties is performed. The output side implements *getNegotiationProperties* to provide transport information to the input side. The input side receives these properties in its *createInputTransport* function. The response properties from the input side are retrieved through the input side's *getNegotiationProperties* function. Once the output side receives the response from the input side, the response is passed to the port through the *setNegotiationResult* function.

In the example below, the properties included are examples only and do not need to be included in the actual implementation.

Several *cout* statements are included in this code to demonstrate where in the connection process these functions are invoked. These are the locations where device-specific customizations are meant to occur.

Ideally, the transport definition would be developed in a library and linked into the component, like a soft package dependency. However, for the sake of simplicity, the transport definition will be added as source to both components in this example.

### Component edits

In each component, two files need to be modified: the component's user header and cpp files, so transport_out.h/transport_out.cpp and transport_in.h/transport_in.cpp for transport_out and transport_in, respectively.

Modify transport_in.h and transport_out.h to include the following class declarations:

```cpp
    class CustomInputTransport : public bulkio::InputTransport<BULKIO::dataFloat>
    {
    public:
        CustomInputTransport(bulkio::InPort<BULKIO::dataFloat>* port, const std::string& transportId)
          : bulkio::InputTransport<BULKIO::dataFloat>(port, transportId) {
            _port = port;
        };
        std::string transportType() const {
                    return "custom";
        };
        void disconnect() {
            std::cout<<"CustomInputTransport::disconnect"<<std::endl;
        };
        ~CustomInputTransport() {
            std::cout<<"CustomInputTransport::~CustomInputTransport"<<std::endl;
        }
        redhawk::PropertyMap    getNegotiationProperties();

    protected:
        bulkio::InPort<BULKIO::dataFloat>* _port;
    };

    // class implementing the custom transport
    class CustomOutputTransport : public bulkio::OutputTransport<BULKIO::dataFloat>
    {
    public:
        CustomOutputTransport(bulkio::OutPort<BULKIO::dataFloat>* parent, BULKIO::dataFloat_ptr port, const std::string& connectionId)
          : bulkio::OutputTransport<BULKIO::dataFloat>(parent, port) {
            _port = parent;
            _connectionId = connectionId;
            _negotiable_port = ExtendedCF::NegotiableProvidesPort::_nil();
            _transportId = "";
        };
        ~CustomOutputTransport() {
            std::cout<<"CustomOutputTransport::~CustomOutputTransport"<<std::endl;
        };
        std::string transportType() const {
            return "custom";
        };
        virtual CF::Properties transportInfo() const {
            redhawk::PropertyMap props;
            props["transport_side_information"] = "outbound";
            props["another_number"] = static_cast<short>(100);
            return props;
        }
        void _pushSRI(const BULKIO::StreamSRI& sri) {};
        void _pushPacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {};

        void disconnect() {
            std::cout<<"CustomOutputTransport::disconnect"<<std::endl;
            if ((not _transportId.empty()) and (_negotiable_port)) {
                    _negotiable_port->disconnectTransport(_transportId.c_str());
            }
        };
        redhawk::PropertyMap  getNegotiationProperties();
        void setTransportParameters(const std::string &transportId, ExtendedCF::NegotiableProvidesPort_ptr negotiable_port) {
            _transportId = transportId;
            _negotiable_port = negotiable_port;
        }
    protected:
        bulkio::OutPort<BULKIO::dataFloat>* _port;
        ExtendedCF::NegotiableProvidesPort_ptr _negotiable_port;
        std::string _connectionId;
        std::string _transportId;
    };

    // class that creates the transport instance
    class CustomInputManager : public bulkio::InputManager<BULKIO::dataFloat>
    {
    public:
        CustomInputManager(bulkio::InPort<BULKIO::dataFloat>* port)
          : bulkio::InputManager<BULKIO::dataFloat>(port) {
            _port = port;
        };
        CustomInputTransport* createInputTransport(const std::string& transportId, const redhawk::PropertyMap& properties) {
            std::cout<<"CustomInputManager::createInputTransport"<<std::endl;
            for ( redhawk::PropertyMap::const_iterator it = properties.begin(); it != properties.end(); it++) {
                    std::cout<<"key (from uses): "<<it->id<<std::endl;
            }
            return new CustomInputTransport(this->_port, transportId);
        };
        redhawk::PropertyMap getNegotiationProperties(redhawk::ProvidesTransport* providesTransport);

    protected:
        bulkio::InPort<BULKIO::dataFloat>* _port;
        std::string transportType(){
            return "custom";
        };
    };

    // class that creates the transport instance
    class CustomOutputManager : public bulkio::OutputManager<BULKIO::dataFloat>
    {
    public:
        CustomOutputManager(bulkio::OutPort<BULKIO::dataFloat>* port)
          : bulkio::OutputManager<BULKIO::dataFloat>(port) {};

        virtual ~CustomOutputManager() {};
        std::string transportType() {
                return "custom";
        }
        virtual CF::Properties transportProperties();
        virtual CustomOutputTransport* createOutputTransport(PtrType object, const std::string& connectionId, const redhawk::PropertyMap& properties);
        virtual redhawk::PropertyMap getNegotiationProperties(redhawk::UsesTransport* transport);
        virtual void setNegotiationResult(redhawk::UsesTransport* transport, const redhawk::PropertyMap& properties);
    };

    // class that creates the transport manager instance
    class CustomTransportFactory : public bulkio::BulkioTransportFactory<BULKIO::dataFloat> //public redhawk::TransportFactory
    {
    public:
        static CustomInputTransport* Create(bulkio::InFloatPort* parent, const std::string& transportId);
        std::string repoId() {
            return BULKIO::dataFloat::_PD_repoId;
        };
        std::string transportType() {
            return "custom";
        };
        int defaultPriority() {
            return 0;
        };
        virtual ~CustomTransportFactory() {};

        bulkio::OutputManager<BULKIO::dataFloat>* createOutputManager(OutPortType* port);
        bulkio::InputManager<BULKIO::dataFloat>* createInputManager(InPortType* port);
    };
```

Modify transport_in.cpp and transport_out.cpp to define these functions:

```cpp
    bulkio::InputManager<BULKIO::dataFloat>* CustomTransportFactory::createInputManager(bulkio::InPort<BULKIO::dataFloat>* port) {
        return new CustomInputManager(port);
    }

    redhawk::PropertyMap CustomInputTransport::getNegotiationProperties() {
        std::cout<<"CustomInputTransport::getNegotiationProperties"<<std::endl;
        redhawk::PropertyMap props;
        props["data::requestSize"] = static_cast<CORBA::Long>(1000);
        props["data::address"] = "0.0.0.0";
        props["data::port"] =  static_cast<CORBA::Long>(0);
        props["data::protocol"] = "udp";
        return props;
    }

    redhawk::PropertyMap CustomInputManager::getNegotiationProperties(redhawk::ProvidesTransport* providesTransport)
    {
        CustomInputTransport* _transport = dynamic_cast<CustomInputTransport*>(providesTransport);
        if (!_transport) {
            throw redhawk::FatalTransportError("Invalid provides transport instance");
        }

        // return data end point connection information
        redhawk::PropertyMap properties;
        properties =  _transport->getNegotiationProperties();
        return properties;
    }

    redhawk::PropertyMap CustomOutputTransport::getNegotiationProperties() {
        std::cout<<"CustomOutputTransport::getNegotiationProperties"<<std::endl;
        redhawk::PropertyMap props;
        props["data_protocol"] = "hello";
        return props;
    }

    CF::Properties CustomOutputManager::transportProperties() {
        std::cout<<"CustomOutputManager::transportProperties"<<std::endl;
        redhawk::PropertyMap props;
        return props;
    }

    CustomOutputTransport* CustomOutputManager::createOutputTransport(PtrType object, const std::string& connectionId, const redhawk::PropertyMap& inputTransportProps)
    {
        return new CustomOutputTransport(this->_port, object, connectionId);
    }

    redhawk::PropertyMap CustomOutputManager::getNegotiationProperties(redhawk::UsesTransport* transport) {
        CustomOutputTransport* _transport = dynamic_cast<CustomOutputTransport*>(transport);
        if (!_transport) {
            throw redhawk::FatalTransportError("Invalid vita49 transport object provided.");
        }

        redhawk::PropertyMap properties;
        properties =  _transport->getNegotiationProperties();
        return properties;
    }

    void CustomOutputManager::setNegotiationResult(redhawk::UsesTransport* transport, const redhawk::PropertyMap& properties) {
        std::cout<<"CustomOutputManager::setNegotiationResult"<<std::endl;
        for ( redhawk::PropertyMap::const_iterator it = properties.begin(); it != properties.end(); it++) {
                std::cout<<"key (from provides): "<<it->id<<std::endl;
        }
    }

    bulkio::OutputManager<BULKIO::dataFloat>* CustomTransportFactory::createOutputManager(OutPortType* port)
    {
        return new CustomOutputManager(port);
    };
```

Modify transport_in.cpp and transport_out.cpp to register the transport:

```cpp
    static int initializeModule() {
        static CustomTransportFactory factory;
        redhawk::TransportRegistry::RegisterTransport(&factory);
        return 0;
    }

    static int initialized = initializeModule();
```

## Testing the new transport

To test the above code examples, compile and install both components (**transport_out** and **transport_in**).

The following Python session shows how to run the components (note that shared is set to "False", forcing the components to run in different process spaces), connect them, and verify the state of the connections:

```python
    >>> from ossie.utils import sb
    >>> src=sb.launch('transport_out', shared=False)
    >>> snk=sb.launch('transport_in')
    >>> src.connect(snk)
    CustomOutputManager::transportProperties
    CustomOutputTransport::getNegotiationProperties
    CustomInputManager::createInputTransport
    key (from uses): data_protocol
    CustomInputTransport::getNegotiationProperties
    CustomOutputManager::setNegotiationResult
    key (from provides): data::requestSize
    key (from provides): data::address
    key (from provides): data::port
    key (from provides): data::protocol
    >>> src.ports[0]._get_connectionStatus()
    [ossie.cf.ExtendedCF.ConnectionStatus(connectionId='DCE_66bd31e4-3cab-452b-8c21-6c3a2bc165eb', port=<bulkio.bulkioInterfaces.BULKIO.internal._objref_dataFloatExt object at 0x7f55d294f990>, alive=True, transportType='custom', transportInfo=[ossie.cf.CF.DataType(id='transport_side_information', value=CORBA.Any(CORBA.TC_string, 'outbound')), ossie.cf.CF.DataType(id='another_number', value=CORBA.Any(CORBA.TC_short, 100))])]
    >>> src.disconnect(snk)
    CustomOutputTransport::disconnect
    CustomInputTransport::~CustomInputTransport
    CustomOutputTransport::~CustomOutputTransport
```

## Overloading Bulk IO Ports

Adding transports enables the developer to customize the transport mechanism beyond that provided by the REDHAWK baseline. One of the limitations of the transport mechanism addition is that if the two components are located in the same process space, the transport selection mechanism defaults to shared address space, which is optimal for threads located on the same process. However, there are instances in which this approach is not the optimal one. For example, some embedded hardware requires a single point of entry from the microprocessor, so all processing threads that require access to the embedded hardware through the driver must be placed in the same process space. In this case, it may be desirable for the embedded hardware resources to connect to each other directly even though the controlling software resides in two separate threads in the microprocessor. In such instances, it is necessary to overload the provided ports and change the behavior of the default transport mechanism for components that share address space.

For this example, assume that the components with the updated transport shown above are modified as described above. The only additional change is to overload the required ports to select the custom transport for shared address space components; in the case of transport definition, endpoints that share the same address space are considered "local".

This example requires that the \*\_base files be modified on each of these components, so component re-generation for attributes like new properties is not possible while safeguarding these changes. Furthermore, the IDE has been updated to hide several of the CORBA base classes, so the _remove_ref member is shown as an error. To hide this error: right-click on project in "Project Explorer" and select Properties->C/C++ General->Paths and Symbols->GNU C++, and add HAVE_OMNIORB4 as a symbol (no value necessary).

Because the output port selects the transport to be used, the only port that needs to be overloaded is the output port, so only **transport_out** needs to be modified.

### Modifications to transport_out_base.h

The modifications to *transport_out_base.h* involve the declaration of the new overloaded port and the modification of the port member to the component. Luckily, the only function that needs to be overloaded is *_createLocalTransport* that, as the name implies, returns the transport to be used when both endpoints reside in the same process space.

Modify *transport_out_base.h* by adding the new port declaration:

```cpp
    class CustomOutPort : public bulkio::OutFloatPort {
    public:
            virtual ~CustomOutPort() {};
            CustomOutPort(std::string port_name)
              : bulkio::OutFloatPort(port_name) {};

            virtual redhawk::UsesTransport* _createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId);
    };
```

Change the member declaration for the port in *transport_out_base.h* from:

```cpp
    bulkio::OutFloatPort *dataFloat_out;
```

to:

```cpp
    CustomOutPort *dataFloat_out;
```

### Modifications to transport_out_base.cpp

In *transport_out_base.cpp*, the port behavior needs to be defined and the new port class has to be instantiated.

Add the following function definition in *transport_out_base.cpp*.

```cpp
    redhawk::UsesTransport* CustomOutPort::_createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId) {
        // disable the local transport and force a negotiation
        return 0;
    }
```

Change the following class instantiation in *transport_out_base.cpp* from:

```cpp
    dataFloat_out = new bulkio::OutFloatPort("dataFloat_out");
```

to:

```cpp
    dataFloat_out = new CustomOutPort("dataFloat_out");
```

## Testing the new port

Like in the earlier example, compile and install **transport_out**, the only component modified for this example.

The following Python session shows how to run the components (notice that shared is not set to False, running both components in the same process space), connect them, and verify the state of the connections:

```python
    >>> from ossie.utils import sb
    >>> src=sb.launch('transport_out')
    >>> snk=sb.launch('transport_in')
    >>> src.connect(snk)
    CustomOutputManager::transportProperties
    CustomOutputTransport::getNegotiationProperties
    CustomInputManager::createInputTransport
    key (from uses): data_protocol
    CustomInputTransport::getNegotiationProperties
    CustomOutputManager::setNegotiationResult
    key (from provides): data::requestSize
    key (from provides): data::address
    key (from provides): data::port
    key (from provides): data::protocol
    >>> src.ports[0]._get_connectionStatus()
    [ossie.cf.ExtendedCF.ConnectionStatus(connectionId='DCE_66bd31e4-3cab-452b-8c21-6c3a2bc165eb', port=<bulkio.bulkioInterfaces.BULKIO.internal._objref_dataFloatExt object at 0x7f55d294f990>, alive=True, transportType='custom', transportInfo=[ossie.cf.CF.DataType(id='transport_side_information', value=CORBA.Any(CORBA.TC_string, 'outbound')), ossie.cf.CF.DataType(id='another_number', value=CORBA.Any(CORBA.TC_short, 100))])]
    >>> src.disconnect(snk)
    CustomOutputTransport::disconnect
    CustomInputTransport::~CustomInputTransport
    CustomOutputTransport::~CustomOutputTransport
```
