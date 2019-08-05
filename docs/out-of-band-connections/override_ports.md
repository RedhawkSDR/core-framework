# Creating connections between components in FPGA.

In the context of REDHAWK, a component's implementation does not need to be tied solely to a single process or thread running on a microprocessor.
In instances where the component functionality is implemented on a different processor, like an FPGA, the role of the component process or thread is to manage the bitfile load, its properties, and its data ingress and egress.
In the case where the data ingress or egress is to a component running outside the scope of the FPGA, the role of the component is to manage the FPGA load and provide command-and-control information to the embedded device, like where to push data to or retrieve data from.

While not directly supported by the code generators, BULK IO ports can be extended to provide connection information to 2 separate components supporting functionality on an FPGA load. The extended classes override the base classes' connection functionality while maintaining the status information, allowing an out-of-band connection to be established while maintaining the tooling's ability to monitor the state of the connection wherever possible.

In this note, the port override necessary to support the connection between port C and port D in the following image:

![Port Connection](/docs/out-of-band-connections/images/NegotiableTransport.png "Logical ports corresponding to out-of-band links")

## Extending Bulk IO ports

For the purposes of this example, suppose that there is a component called **stream_out_override** that has a single output *dataFloat* port (called **dataFloat**) and another component called **stream_in_override** that has a single input *dataFloat* port (called **dataFloat**). This example shows how to modify these components such that the data exchange occurs in the FPGA rather than through the microprocessor.

This example requires that the \*\_base files be modified on each of these components. The IDE has been updated to hide several of the CORBA base classes, so the _remove_ref member is shown as an error. To hide this error: right-click on project in "Project Explorer" and select Properties->C/C++ General->Paths and Symbols->GNU C++, and add HAVE_OMNIORB4 as a symbol (no value necessary)

### The **stream_out_override** component

In *stream_out_override_base.h*:

    // declare the port class
    class my_outfloat : public bulkio::OutFloatPort {
    public:
        my_outfloat(const std::string port_name) : bulkio::OutFloatPort (port_name) {};

        ExtendedCF::TransportInfo make_transport();

        template <class Target, class Func>
        void setInitialTransportCallback (Target target, Func func)
        {
                    initial_setup_callbacks_.add(target, func);
        }

        template <class Target, class Func>
        void setupTransportCallback (Target target, Func func)
        {
                    setup_transport_callbacks_.add(target, func);
        }

        template <class Target, class Func>
        void setTearDownTransportCallback (Target target, Func func)
        {
                    teardown_callbacks_.add(target, func);
        }

        ExtendedCF::TransportInfoSequence* supportedTransports();
        ExtendedCF::ConnectionStatusSequence* connectionStatus();
        void connectPort(CORBA::Object_ptr object, const char* connectionId);
        void disconnectPort(const char* connectionId);

    private:
        ossie::notification<void (bool &, CF::Properties &)> initial_setup_callbacks_;
        ossie::notification<void (bool &, CF::Properties)> setup_transport_callbacks_;
        ossie::notification<void (CF::Properties)> teardown_callbacks_;
        std::map<std::string, ExtendedCF::NegotiationResult_var>connection_id_to_transport;
    };

change:

    bulkio::OutFloatPort *dataFloat;

to

    my_outfloat *dataFloat;

In *stream_out_override_base.cpp*:

    // define the new port class
    ExtendedCF::TransportInfo my_outfloat::make_transport() {
        ExtendedCF::TransportInfo transport;

        // define the name of the type of transport
        transport.transportType = "my_transport";

        // add transport properties that are common to all connections with this port
        char host[HOST_NAME_MAX+1];
        gethostname(host, sizeof(host));
        std::string _hostname = host;
        CF::Properties transport_properties;
        redhawk::PropertyMap &prop_map = redhawk::PropertyMap::cast(transport_properties);
        prop_map["hostname"] = _hostname;
        transport.transportProperties = transport_properties;
        return transport;
    }

    ExtendedCF::TransportInfoSequence* my_outfloat::supportedTransports()
    {
        ExtendedCF::TransportInfoSequence_var transports = new ExtendedCF::TransportInfoSequence;
        ossie::corba::push_back(transports, make_transport());
        return transports._retn();
    }

    ExtendedCF::ConnectionStatusSequence* my_outfloat::connectionStatus()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        ExtendedCF::ConnectionStatusSequence_var retVal = new ExtendedCF::ConnectionStatusSequence();
        for (ConnectionList::iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
            ExtendedCF::ConnectionStatus status;
            std::string tmp_connectionId = (*connection)->connectionId;
            status.connectionId = tmp_connectionId.c_str();
            status.port = CORBA::Object::_duplicate((*connection)->objref);
            status.alive = true;
            status.transportType = make_transport().transportType;
            status.transportInfo = connection_id_to_transport[tmp_connectionId]->properties;
            ossie::corba::push_back(retVal, status);
        }
        return retVal._retn();
    }

    void my_outfloat::connectPort(CORBA::Object_ptr object, const char* connectionId)
    {
        // Give a specific exception message for nil
        if (CORBA::is_nil(object)) {
            throw CF::Port::InvalidPort(1, "Nil object reference");
        }

        // Attempt to check the type of the remote object to reject invalid
        // types; note this does not require the lock
        _validatePort(object);

        const std::string connection_id(connectionId);
        {
            // Acquire the state lock before modifying the container
            boost::mutex::scoped_lock lock(updatingPortsLock);

            // Prevent duplicate connection IDs
            if (_findConnection(connection_id) != _connections.end()) {
                throw CF::Port::OccupiedPort();
            }
            ExtendedCF::NegotiableProvidesPort_var negotiable_port = ossie::corba::_narrowSafe<ExtendedCF::NegotiableProvidesPort>(object);

            if (CORBA::is_nil(negotiable_port)) {
                throw CF::Port::InvalidPort(1, "Port not negotiable");
            }

            ExtendedCF::TransportInfoSequence_var supported_transports;
            try {
                supported_transports = negotiable_port->supportedTransports();
            } catch (const CORBA::Exception& exc) {
                throw CF::Port::InvalidPort(1, "Unable to retrieve the supported transports");
            }
            bool found_transport = false;
            ExtendedCF::TransportInfoSequence_var transports = supportedTransports();
            std::string transportType(transports[0].transportType);
            for (CORBA::ULong index = 0; index < supported_transports->length(); ++index) {
                if (transportType == static_cast<const char*>(supported_transports[index].transportType)) {
                    found_transport = true;
                    break;
                }
            }
            if (not found_transport) {
                throw CF::Port::InvalidPort(1, "Unable to find matching transports");
            }
            CF::Properties negotiation_props;
            bool valid = false;
            initial_setup_callbacks_(valid, negotiation_props);
            if (not valid) {
                throw CF::Port::InvalidPort(1, "Unable to retrieve transport parameters");
            }

            ExtendedCF::NegotiationResult_var result;
            try {
                result = negotiable_port->negotiateTransport("my_transport", negotiation_props);
            } catch (const ExtendedCF::NegotiationError& exc) {
                throw CF::Port::InvalidPort(1, "Unable to negotiate transports");
            }

            setup_transport_callbacks_(valid, result->properties);

            if (not valid) {
                negotiable_port->disconnectTransport(result->transportId);
                throw CF::Port::InvalidPort(1, "Unable to establish the transport");
            }

            const std::string transport_id(result->transportId);
            // store the result's transportId to disconnect later
            connection_id_to_transport[connection_id] = result;
        }
        Connection* connection = _createConnection(object, connection_id);
        _connections.push_back(connection);
    };

    void my_outfloat::disconnectPort(const char* connectionId) {

        boost::mutex::scoped_lock lock(updatingPortsLock);
        std::string connection_id(connectionId);

        ConnectionList::iterator connection = _findConnection(connectionId);
        if (connection == _connections.end()) {
            std::string message = std::string("No connection ") + connectionId;
            throw CF::Port::InvalidPort(2, message.c_str());
        }
        ExtendedCF::NegotiableProvidesPort_var negotiable_port = ossie::corba::_narrowSafe<ExtendedCF::NegotiableProvidesPort>((*connection)->objref);
        teardown_callbacks_(connection_id_to_transport[connection_id]->properties);
        negotiable_port->disconnectTransport(connection_id_to_transport[connection_id]->transportId);
        connection_id_to_transport.erase(connection_id);
        _connections.erase(connection);
    };

change:

        dataFloat = new bulkio::OutFloatPort("dataFloat");

to

        dataFloat = new my_outfloat("dataFloat");

In *stream_out_override.h*:

Add these public declarations:

        void getInitialProperties(bool &valid, CF::Properties &props);
        void setupTranportProperties(bool &valid, CF::Properties props);
        void resetTransportProperties(CF::Properties props);

In *stream_out_override.cpp*:

Implement the FPGA transport definition methods:

    void stream_out_override_i::getInitialProperties(bool &valid, CF::Properties &props)
    {
        // retrieve instance-specific properties to give to the other side to establish a link
        std::cout<<"stream_out_override getting the initial properties"<<std::endl;
        valid = true;
    }

    void stream_out_override_i::setupTranportProperties(bool &valid, CF::Properties props)
    {
        // attempt to establish a link given the other side's information
        std::cout<<"stream_out_override setting up the transport"<<std::endl;
        valid = true;
    }

    void stream_out_override_i::resetTransportProperties(CF::Properties props)
    {
        std::cout<<"stream_out_override resetting the transport"<<std::endl;
        // tear down the link described by the properties
}

bind the callbacks (implement in the "constructor" method):

        this->dataFloat->setInitialTransportCallback(this, &stream_out_override_i::getInitialProperties);
        this->dataFloat->setupTransportCallback(this, &stream_out_override_i::setupTranportProperties);
        this->dataFloat->setTearDownTransportCallback(this, &stream_out_override_i::resetTransportProperties);

### The **stream_in_override** component:

In the *stream_in_override_base.h* file:

    // declare the new port class
    class my_infloat : public bulkio::InFloatPort {
    public:
        my_infloat(const std::string port_name) : bulkio::InFloatPort (port_name) {};

        ExtendedCF::TransportInfo make_transport();

        template <class Target, class Func>
        void setEstablishTransportCallback (Target target, Func func)
        {
            setup_callbacks_.add(target, func);
        }

            template <class Target, class Func>
        void setTearDownTransportCallback (Target target, Func func)
        {
                    teardown_callbacks_.add(target, func);
        }

        ExtendedCF::TransportInfoSequence* supportedTransports();
        ExtendedCF::NegotiationResult* negotiateTransport(const char* transportType, const CF::Properties& transportProperties);
        void disconnectTransport(const char* transportId);

    protected:
        ossie::notification<void (bool &, CF::Properties &)> setup_callbacks_;
        ossie::notification<void (CF::Properties)> teardown_callbacks_;
        std::map<std::string, ExtendedCF::NegotiationResult_var>current_transport_instances;
    };

Change:

    bulkio::InFloatPort *dataFloat;

to

    my_infloat *dataFloat;

In the *stream_in_override_base.cpp* file:

    // define the new port class
    ExtendedCF::TransportInfo my_infloat::make_transport() {
        ExtendedCF::TransportInfo transport;

        // define the name of the type of transport
        transport.transportType = "my_transport";

        // add transport properties that are common to all connections with this port
        char host[HOST_NAME_MAX+1];
        gethostname(host, sizeof(host));
        std::string _hostname = host;
        CF::Properties transport_properties;
        redhawk::PropertyMap &prop_map = redhawk::PropertyMap::cast(transport_properties);
        prop_map["hostname"] = _hostname;
        transport.transportProperties = transport_properties;
        return transport;
    }

    ExtendedCF::TransportInfoSequence* my_infloat::supportedTransports()
    {
        ExtendedCF::TransportInfoSequence_var transports = new ExtendedCF::TransportInfoSequence;
        ossie::corba::push_back(transports, make_transport());
        return transports._retn();
    }

    ExtendedCF::NegotiationResult* my_infloat::negotiateTransport(const char* transportType, const CF::Properties& transportProperties)
    {
        boost::mutex::scoped_lock lock(_transportMutex);

        // determine whether or not the requested transport is available (check just one in this case)
        std::string str_transportType = std::string(transportType);
        std::string local_transport = std::string(make_transport().transportType);
        if (str_transportType != local_transport) {
            throw ExtendedCF::NegotiationError("transport not supported");
        }

        ExtendedCF::NegotiationResult_var result = new ExtendedCF::NegotiationResult;

        // generate a unique id for this transport instance
        std::string transport_id = ossie::generateUUID();
        result->transportId = transport_id.c_str();
        // these properties are a response to the requester
        result->properties = make_transport().transportProperties;
        bool valid;
        setup_callbacks_(valid, result->properties);

        if (valid) {
            current_transport_instances[transport_id] = result;
            return result._retn();
        }

        ExtendedCF::NegotiationResult_var bad_result = new ExtendedCF::NegotiationResult;
        return bad_result._retn();
    }

    void my_infloat::disconnectTransport(const char* transportId)
    {
        boost::mutex::scoped_lock lock(_transportMutex);

        std::string transport_id = std::string(transportId);

        // tear down the transport in the fpga
        teardown_callbacks_(current_transport_instances[transport_id]->properties);

        // disable the transport corresponding to transportId
        current_transport_instances.erase(transport_id);
    }

Change:

    dataFloat = new bulkio::InFloatPort("dataFloat");

to

    dataFloat = new my_infloat("dataFloat");

In the stream_in_override.h file:

declare the public members:

    void getTransportProperties(bool &valid, CF::Properties &props);
    void resetTransportProperties(CF::Properties props);

In the stream_in_override.cpp file:

    void stream_in_override_i::getTransportProperties(bool &valid, CF::Properties &props)
    {
        // request transport parameters needed to receive data
        // in this case, the parameters are appended to those passed to this function from the port
        // set valid to false if transport cannot be supported
        redhawk::PropertyMap &prop_map = redhawk::PropertyMap::cast(props);
        prop_map["additional_prop"] = "hello";
        valid = true;
        std::cout<<"stream_in_override returning transport properties for the other side to establish a connection"<<std::endl;
    }

    void stream_in_override_i::resetTransportProperties(CF::Properties props)
    {
        // undo transport
        std::cout<<"stream_in_override tearing down the transport"<<std::endl;
    }

bind the callbacks (implement in the "constructor" method):

    this->dataFloat->setEstablishTransportCallback(this, &stream_in_override_i::getTransportProperties);
    this->dataFloat->setTearDownTransportCallback(this, &stream_in_override_i::resetTransportProperties);

## Testing the new ports

To test the above code examples, compile and install both components (**stream_in_override** and **stream_out_override**).

The following Python session shows how to run the components, connect them, and verify the state of the connections:

    >>> from ossie.utils import sb
    >>> src=sb.launch('stream_out_override')
    >>> snk=sb.launch('stream_in_override')
    >>> src.connect(snk)
    stream_out_override getting the initial properties
    stream_in_override returning transport properties for the other side to establish a connection
    stream_out_override setting up the transport
    >>> src.ports[0]._get_connectionStatus()
    [ossie.cf.ExtendedCF.ConnectionStatus(connectionId='DCE_4bff4ab6-6ad2-4977-a166-3ba56bfae5eb', port=<bulkio.bulkioInterfaces.BULKIO.internal._objref_dataFloatExt object at 0x7fc73b513310>, alive=True, transportType='my_transport', transportInfo=[ossie.cf.CF.DataType(id='hostname', value=CORBA.Any(CORBA.TC_string, 'my_host.my_company.com')), ossie.cf.CF.DataType(id='additional_prop', value=CORBA.Any(CORBA.TC_string, 'hello'))])]
    >>> src.disconnect(snk)
    stream_out_override resetting the transport
    stream_in_override tearing down the transport


