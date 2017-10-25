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
#ifndef __bulkio_BulkioTransport_h
#define __bulkio_BulkioTransport_h

#include <ossie/Transport.h>

#include "bulkio_base.h"
#include "bulkio_typetraits.h"

namespace bulkio {

    template <class PortType>
    class InPort;

    template <class PortType>
    class OutPort;

    template <typename PortType>
    class OutputTransport : public redhawk::UsesTransport
    {
    public:
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        virtual ~OutputTransport();

        virtual void disconnect();

        void pushSRI(const std::string& streamID, const BULKIO::StreamSRI& sri, int version);

        void pushPacket(const BufferType& data,
                        const BULKIO::PrecisionUTCTime& T,
                        bool EOS,
                        const std::string& streamID,
                        const BULKIO::StreamSRI& sri);

        BULKIO::PortStatistics getStatistics();

    protected:
        typedef OutPort<PortType> OutPortType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortType::_var_type VarType;
        typedef typename NativeTraits<PortType>::NativeType NativeType;

        OutputTransport(OutPortType* port, PtrType objref);

        virtual void _pushSRI(const BULKIO::StreamSRI& sri) = 0;

        virtual void _sendPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri);

        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID) = 0;

        void _recordPush(const std::string& streamID, size_t elements, bool endOfStream);

        virtual redhawk::PropertyMap _getExtendedStatistics();

        //
        // Returns the total number of elements of data in a pushPacket call, for
        // statistical tracking; enables XML and File specialization, which have
        // different notions of size
        //
        size_t _dataLength(const BufferType& data);

        OutPortType* _port;
        VarType _objref;
        typedef std::map<std::string,int> VersionMap;
        VersionMap _sriVersions;

    private:
        linkStatistics _stats;
    };

    template <class PortType>
    class InputTransport : public redhawk::ProvidesTransport
    {
    protected:
        typedef InPort<PortType> InPortType;
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        InputTransport(InPortType* port, const std::string& transportId);

        inline void _queuePacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, bool eos, const std::string& streamID)
        {
          _port->queuePacket(data, T, eos, streamID);
        }

        InPortType* _port;
    };

    template <class PortType>
    class OutputManager : public redhawk::UsesTransportManager
    {
    public:
        typedef typename PortType::_ptr_type PtrType;

        virtual OutputTransport<PortType>* createOutputTransport(PtrType object, 
                                                                 const std::string& connectionId,
                                                                 const redhawk::PropertyMap& properties) = 0;
    protected:
        typedef OutPort<PortType> OutPortType;

        OutputManager(OutPortType* port);

        OutPortType* _port;

    private:
        virtual redhawk::UsesTransport* createUsesTransport(CORBA::Object_ptr object,
                                                            const std::string& connectionId,
                                                            const redhawk::PropertyMap& properties);
    };

    template <class PortType>
    class InputManager : public redhawk::ProvidesTransportManager
    {
    public:
        virtual InputTransport<PortType>* createInputTransport(const std::string& transportId,
                                                               const redhawk::PropertyMap& properties) = 0;
    protected:
        typedef InPort<PortType> InPortType;

        InputManager(InPortType* port);

        InPortType* _port;

    private:
        virtual redhawk::ProvidesTransport* createProvidesTransport(const std::string& transportId,
                                                                    const redhawk::PropertyMap& properties);
    };

    template <class PortType>
    class BulkioTransportFactory : public redhawk::TransportFactory
    {
    public:
        typedef InPort<PortType> InPortType;
        typedef OutPort<PortType> OutPortType;

        virtual std::string repoId();

        virtual InputManager<PortType>* createInputManager(InPortType* port) = 0;
        virtual OutputManager<PortType>* createOutputManager(OutPortType* port) = 0;

    private:
        virtual redhawk::ProvidesTransportManager* createProvidesManager(redhawk::NegotiableProvidesPortBase* port);
        virtual redhawk::UsesTransportManager* createUsesManager(redhawk::NegotiableUsesPort* port);
    };
}

#endif // __bulkio_BulkioTransport_h
