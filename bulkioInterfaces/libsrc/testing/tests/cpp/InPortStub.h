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
#ifndef BULKIO_INPORTSTUB_H
#define BULKIO_INPORTSTUB_H

#include "bulkio.h"

template <class PortType>
class InPortStubBase : public virtual bulkio::CorbaTraits<PortType>::POAType
{
public:
    InPortStubBase()
    {
    }

    virtual void pushSRI(const BULKIO::StreamSRI& H)
    {
        this->H.push_back(H);
    }

    virtual BULKIO::PortUsageType state()
    {
        return BULKIO::IDLE;
    }

    virtual BULKIO::PortStatistics* statistics()
    {
        return new BULKIO::PortStatistics();
    }

    virtual BULKIO::StreamSRISequence* activeSRIs()
    {
        return new BULKIO::StreamSRISequence();
    }

    std::vector<BULKIO::StreamSRI> H;
};

template <class PortType>
class InPortStub : public InPortStubBase<PortType>
{
public:
    typedef typename bulkio::CorbaTraits<PortType>::SequenceType SequenceType;

    virtual void pushPacket(const SequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
    {
        packets.push_back(Packet(data, T, EOS, streamID));
    }

    struct Packet {
        Packet(const SequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID) :
            data(data),
            T(T),
            EOS(EOS),
            streamID(streamID)
        {
        }

        size_t size() const
        {
            return data.length();
        }

        SequenceType data;
        BULKIO::PrecisionUTCTime T;
        bool EOS;
        std::string streamID;
    };

    std::vector<Packet> packets;
};

template <>
inline size_t InPortStub<BULKIO::dataBit>::Packet::size() const
{
    return data.bits;
}

template <>
class InPortStub<BULKIO::dataXML> : public InPortStubBase<BULKIO::dataXML>
{
public:
    virtual void pushPacket(const char* data, CORBA::Boolean EOS, const char* streamID)
    {
        this->packets.push_back(Packet(data, EOS, streamID));
    }

    struct Packet {
        Packet(const char* data, CORBA::Boolean EOS, const char* streamID) :
            data(data),
            EOS(EOS),
            streamID(streamID)
        {
        }

        size_t size() const
        {
            return data.size();
        }

        std::string data;
        bool EOS;
        std::string streamID;
    };

    std::vector<Packet> packets;
};

template <>
class InPortStub<BULKIO::dataFile> : public InPortStubBase<BULKIO::dataFile>
{
public:
    virtual void pushPacket(const char* data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
    {
        packets.push_back(Packet(data, T, EOS, streamID));
    }

    struct Packet {
        Packet(const char* data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID) :
            data(data),
            T(T),
            EOS(EOS),
            streamID(streamID)
        {
        }

        size_t size() const
        {
            return data.size();
        }

        std::string data;
        BULKIO::PrecisionUTCTime T;
        bool EOS;
        std::string streamID;
    };

    std::vector<Packet> packets;
};

#endif // BULKIO_INPORTSTUB_H
