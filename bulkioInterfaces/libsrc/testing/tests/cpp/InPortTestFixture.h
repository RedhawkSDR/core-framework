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
#ifndef BULKIO_INPORTTESTFIXTURE_H
#define BULKIO_INPORTTESTFIXTURE_H

#include <cppunit/extensions/HelperMacros.h>

#include <bulkio/bulkio_in_port.h>

template <class Port>
class InPortTestFixture : public CppUnit::TestFixture
{
public:
    void setUp()
    {
        port = new Port(getPortName());
    }

    void tearDown()
    {
        delete port;
    }

protected:
    typedef typename Port::CorbaType CorbaType;

    virtual std::string getPortName() const
    {
        std::string name = bulkio::CorbaTraits<CorbaType>::name();
        return name + "_in";
    }

    inline void _pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime& time,
                         bool eos, const char* streamID)
    {
        typename Port::PortSequenceType data;
        data.length(length);
        port->pushPacket(data, time, eos, streamID);
    }

    Port* port;
};

template <>
inline void InPortTestFixture<bulkio::InFilePort>::_pushTestPacket(size_t length,
                                                                   const BULKIO::PrecisionUTCTime& time,
                                                                   bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), time, eos, streamID);
}

template <>
inline void InPortTestFixture<bulkio::InXMLPort>::_pushTestPacket(size_t length,
                                                                  const BULKIO::PrecisionUTCTime&,
                                                                  bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), eos, streamID);
}

template <>
inline void InPortTestFixture<bulkio::InBitPort>::_pushTestPacket(size_t length,
                                                                  const BULKIO::PrecisionUTCTime& time,
                                                                  bool eos, const char* streamID)
{
    BULKIO::BitSequence data;
    data.data.length((length+7)/8);
    data.bits = length;
    port->pushPacket(data, time, eos, streamID);
}

#endif // BULKIO_INPORTTESTFIXTURE_H
