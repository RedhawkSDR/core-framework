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
    InPortStubBase() :
        sriCounter(0),
        packetCounter(0)
    {
    }

    virtual void pushSRI(const BULKIO::StreamSRI& H)
    {
        this->H = H;
        ++sriCounter;
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

    BULKIO::StreamSRI H;
    int sriCounter;

    BULKIO::PrecisionUTCTime T;
    bool EOS;
    std::string streamID;
    int packetCounter;

protected:
    void _push(const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
    {
        this->T = T;
        this->EOS = EOS;
        this->streamID = streamID;
        this->packetCounter++;
    }
};

template <class PortType>
class InPortStub : public InPortStubBase<PortType>
{
public:
    typedef typename bulkio::CorbaTraits<PortType>::SequenceType SequenceType;

    virtual void pushPacket(const SequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
    {
        this->data = data;
        this->_push(T, EOS, streamID);
    }

    SequenceType data;
};

template <>
class InPortStub<BULKIO::dataXML> : public InPortStubBase<BULKIO::dataXML>
{
public:
    virtual void pushPacket(const char* data, CORBA::Boolean EOS, const char* streamID)
    {
        this->data = data;
        this->_push(bulkio::time::utils::notSet(), EOS, streamID);
    }

    std::string data;
};

template <>
class InPortStub<BULKIO::dataFile> : public InPortStubBase<BULKIO::dataFile>
{
public:
    virtual void pushPacket(const char* data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
    {
        this->data = data;
        this->_push(T, EOS, streamID);
    }

    std::string data;
};

#endif // BULKIO_INPORTSTUB_H
