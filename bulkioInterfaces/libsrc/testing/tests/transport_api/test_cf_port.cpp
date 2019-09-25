

/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "transport_api_test.h"

template <class OutPort, class InPort>
class Custom_BulkioPacket_Base : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(Custom_BulkioPacket_Base);
    CPPUNIT_TEST(CreatePorts);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void setUp_pre();
    void setUp_post();
    void tearDown();

    void CreatePorts();

protected:
    typedef typename OutPort::StreamType OutStreamType;
    typedef typename InPort::StreamType InStreamType;
    typedef typename OutStreamType::ScalarType ScalarType;
    typedef typename InStreamType::DataBlockType DataBlockType;

    virtual std::string getPortName() const = 0;

    OutPort*             outPort; // output port
    InPort*              inPort;  // input port
    rh_logger::LoggerPtr logger;  // logger for diagnostics
    std::string          cid;     // connection id
};

template <class OutPort, class InPort>
void Custom_BulkioPacket_Base<OutPort,InPort>::setUp()
{
    logger = rh_logger::Logger::getLogger("custom.bulkio.blocking.stream");
}

template <class OutPort, class InPort>
void Custom_BulkioPacket_Base<OutPort,InPort>::setUp_pre()
{
    setUp_bulkio_test();
    setUp_custom_priority("0");
    setUp_disable_transport("local");
    setUp_disable_transport("corba");
    setUp_disable_transport("shm");
}


template <class OutPort, class InPort>
void Custom_BulkioPacket_Base<OutPort,InPort>::setUp_post()
{
  cid="custom_connection";
  std::string name = getPortName();
    try {
        ossie::corba::CorbaInit(0,0);
        outPort = new OutPort("data" + name + "_out");
        outPort->initializePort();
        inPort = new InPort("data" + name + "_in");
        inPort->initializePort();
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(inPort);
        CORBA::Object_var objref = inPort->_this();
        outPort->connectPort(objref, cid.c_str());
    }
    catch(...){
        RH_ERROR(logger,"Failure....during setup step");
    }
}

template <class OutPort, class InPort>
void Custom_BulkioPacket_Base<OutPort,InPort>::tearDown()
{

    if ( inPort ) {
        outPort->disconnectPort(cid.c_str());

        // deactivate input port...
        try {
            PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(inPort);
            ossie::corba::RootPOA()->deactivate_object(oid);
        } catch (...) {
            // Ignore CORBA exceptions
        }

        // cleanup objects
        inPort->_remove_ref();
    }

    // handle cases where we are making bad things happen
    try {
        delete outPort;
    }
    catch(...)  {}

    tearDown_reset_env();
}


template <class OutPort, class InPort>
void Custom_BulkioPacket_Base<OutPort,InPort>::CreatePorts()
{
    setUp_pre();

  cid="custom_connection";
  std::string name = getPortName();
    try {
        ossie::corba::CorbaInit(0,0);
        outPort = new OutPort("data" + name + "_out");
        outPort->initializePort();
        inPort = new InPort("data" + name + "_in");
        inPort->initializePort();
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(inPort);
        CORBA::Object_var objref = inPort->_this();
        outPort->connectPort(objref, cid.c_str());
    }
    catch(...){
        RH_ERROR(logger,"Failure....during setup step");
    }

}


#define CREATE_TEST(x,pp)						\
    class Custom_BulkioPacket_##x##_##pp##_Test : public Custom_BulkioPacket_Base<bulkio::Out##x##Port,bulkio::In##x##Port> \
    {                                                                   \
        typedef Custom_BulkioPacket_Base< CustomOut##x##Port,bulkio::In##x##Port> TestBase; \
        CPPUNIT_TEST_SUB_SUITE(Custom_BulkioPacket_##x##_##pp##_Test, TestBase); \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Custom_BulkioPacket_##x##_##pp##_Test);

CREATE_TEST(Float,udp);
