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
#ifndef __custom_transport_h
#define __custom_transport_h

#include <ossie/PropertyMap.h>
#include <ossie/ProvidesPort.h>
#include <ossie/UsesPort.h>
#include <BulkioTransport.h>

namespace custom_transport
{

    /**
        Custom Transport that configure and controls the data flow over the actual transport
     */
    class CustomInputTransport : public bulkio::InputTransport<BULKIO::dataFloat>
    {
    public:
    CustomInputTransport(bulkio::InPort<BULKIO::dataFloat>* port,
                         const std::string& transportId,
                         const redhawk::PropertyMap &usesTransportProps )

        : bulkio::InputTransport<BULKIO::dataFloat>(port, transportId) {

            _usesTransportProps = usesTransportProps;
        };

        virtual ~CustomInputTransport() {
            RH_NL_DEBUG("custom.transport", "CustomInputTransport::~CustomInputTransport" );
        }

        std::string transportType() const {
            return "custom";
        };

        redhawk::PropertyMap    getNegotiationProperties();

        /**
           Control interface used by port to start the transport.
        */
        void startTransport() {
            RH_NL_DEBUG("custom.transport", "CustomInputTransport::startTransport");
        };

        /**
           Control interface used by port to stop the transport.
        */
        void stopTransport() {
            RH_NL_DEBUG("custom.transport", "CustomInputTransport::stopTransport");
        };

    protected:
        //  provides transport layer statistics to port
        redhawk::PropertyMap  _getExtendedStatistics() {
            return redhawk::PropertyMap();
        }

    private:
        redhawk::PropertyMap _usesTransportProps;
    };

    // class implementing the custom transport
    class CustomOutputTransport : public bulkio::OutputTransport<BULKIO::dataFloat>
    {
    public:
    CustomOutputTransport(bulkio::OutPort<BULKIO::dataFloat>* parent,
                          BULKIO::dataFloat_ptr port,
                          const std::string& connectionId,
                          const redhawk::PropertyMap &props)
        : bulkio::OutputTransport<BULKIO::dataFloat>(parent, port) {
            _connectionId = connectionId;
            _inProps = props;
        };
        virtual ~CustomOutputTransport() {

            RH_NL_DEBUG("bulkio::custom::OutTransport", "CustomOutputTransport::~CustomOutputTransport");
        };
        std::string transportType() const {
            return "custom";
        };
        virtual CF::Properties transportInfo() const {
            redhawk::PropertyMap props;
            props["bulkio::custom::transport_side_information"] = "outbound";
            props["bulkio::custom::another_number"] = static_cast<short>(100);
            return props;
        }
        void _pushSRI(const BULKIO::StreamSRI& sri) {};
        void _pushPacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {};

        void disconnect() {
            RH_NL_DEBUG("bulkio::custom::OutTransport", "CustomOutputTransport::disconnect");
        };

        redhawk::PropertyMap  getNegotiationProperties();

    protected:
        std::string _connectionId;
        redhawk::PropertyMap _inProps;
    };


    // Manager class that creates input transport layers for a negotiable port
    class CustomInputManager : public bulkio::InputManager<BULKIO::dataFloat>
    {
    public:
    CustomInputManager(bulkio::InPort<BULKIO::dataFloat>* port)
        : bulkio::InputManager<BULKIO::dataFloat>(port) {
        };

        CustomInputTransport* createInputTransport(const std::string& transportId, const redhawk::PropertyMap& properties);

        std::string transportType(){
            return "custom";
        };

        virtual CF::Properties transportProperties();

        redhawk::PropertyMap getNegotiationProperties(redhawk::ProvidesTransport* providesTransport);

    };


    // Manager class that creates output transport layers for a negotiable port
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


    // Create factory class that registers input/ouptut managers with a negotiation port
    class CustomTransportFactory : public bulkio::BulkioTransportFactory<BULKIO::dataFloat>
    {
    public:

        std::string transportType() {
            return "custom";
        };

        int defaultPriority() {
            const char* sprior = getenv( "BULKIO_CUSTOM_PRIORITY" );
            if ( sprior ) {
                RH_NL_TRACE("bulkio.custom.factory", " env = BULKIO_CUSTOM_PRIORITY val " << sprior );
                return (int)strtol(sprior, NULL, 0);
            }
            return 0;
        };

        virtual ~CustomTransportFactory() {};

        CustomOutputManager* createOutputManager(OutPortType* port);
        CustomInputManager* createInputManager(InPortType* port);
    };

};

#endif /* __custom_transport_h */
