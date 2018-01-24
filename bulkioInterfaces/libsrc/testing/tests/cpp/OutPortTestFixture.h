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
#ifndef BULKIO_OUTPORTTESTFIXTURE_H
#define BULKIO_OUTPORTTESTFIXTURE_H

#include <cppunit/extensions/HelperMacros.h>

#include <bulkio/bulkio_out_port.h>

#include "InPortStub.h"

template <class Port>
class OutPortTestFixture : public CppUnit::TestFixture
{
public:
    void setUp()
    {
        port = new Port(getPortName());

        stub = _createStub();

        CORBA::Object_var objref = stub->_this();
        port->connectPort(objref, "test_connection");
    }

    void tearDown()
    {
        try {
            _disconnectPorts();
        } catch (...) {
            // Ignore disconnection errors
        }

        // The port has not been used as a CORBA object, so we can delete it directly
        delete port;

        _releaseServants();

        stub = 0;
    }

protected:
    typedef typename Port::CorbaType CorbaType;
    typedef InPortStub<CorbaType> StubType;

    virtual std::string getPortName() const
    { 
        std::string name = bulkio::CorbaTraits<CorbaType>::name();
        return name + "_out";
    };

    StubType* _createStub()
    {
        StubType* inport = new StubType();
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(inport);
        _servants.push_back(inport);
        return inport;
    }

    void _disconnectPorts()
    {
        ExtendedCF::UsesConnectionSequence_var connections = port->connections();
        for (CORBA::ULong ii = 0; ii < connections->length(); ++ii) {
            port->disconnectPort(connections[ii].connectionId);
        }
    }

    void _releaseServants()
    {
        for (ServantList::iterator servant = _servants.begin(); servant != _servants.end(); ++servant) {
            try {
                PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(*servant);
                ossie::corba::RootPOA()->deactivate_object(oid);
            } catch (...) {
                // Ignore CORBA exceptions
            }
            (*servant)->_remove_ref();
        }
        _servants.clear();
    }

    inline void _pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime& time,
                                bool eos, const std::string& streamID)
    {
        typename Port::NativeSequenceType data;
        data.resize(length);
        port->pushPacket(data, time, eos, streamID);
    }

    Port* port;
    StubType* stub;

    typedef std::vector<PortableServer::ServantBase*> ServantList;
    ServantList _servants;
};

template <>
inline void OutPortTestFixture<bulkio::OutBitPort>::_pushTestPacket(size_t length,
                                                                    const BULKIO::PrecisionUTCTime& time,
                                                                    bool eos, const std::string& streamID)
{
    redhawk::bitbuffer data(length);
    port->pushPacket(data, time, eos, streamID);
}

template <>
inline void OutPortTestFixture<bulkio::OutFilePort>::_pushTestPacket(size_t length,
                                                                     const BULKIO::PrecisionUTCTime& time,
                                                                     bool eos, const std::string& streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data, time, eos, streamID);
}

template <>
inline void OutPortTestFixture<bulkio::OutXMLPort>::_pushTestPacket(size_t length,
                                                                    const BULKIO::PrecisionUTCTime&,
                                                                    bool eos, const std::string& streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data, eos, streamID);
}

#endif // BULKIO_OUTPORTTESTFIXTURE_H
