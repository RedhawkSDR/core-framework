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

#include <ossie/CorbaUtils.h>
#include <ossie/debug.h>
#include "bulkio.h"
#include "custom_transport.h"

namespace custom_transport {

    // determine if transport should be disabled
    static int  checkDisableTransport() {
            int disable=0;
	    // check environment variable first
            const char *p = getenv("BULKIO_CUSTOM");
            RH_NL_TRACE("bulkio.custom.transport", "env BULKIO_CUSTOM value:" << p );
            if ( p  && (strcmp(p, "disable") == 0)) {
                RH_NL_TRACE("bulkio.custom.transport", "env BULKIO_CUSTOM 'disable' == " << p );
	      disable=1;
            }
	    RH_NL_DEBUG("bulkio.custom.transport", " disable transport: " << disable );
            return disable;
    }

    static redhawk::PropertyMap _getTransportProperties() {
            redhawk::PropertyMap props;
            int res = checkDisableTransport();
            props[ "custom::disable" ]= static_cast<CORBA::Long>(res);
            return props;
	}

    static redhawk::PropertyMap  _getInNegotiationProperties() {
        redhawk::PropertyMap props;
        props["bulkio::custom::data::requestSize"] = static_cast<CORBA::Long>(1000);
        props["bulkio::custom::data::address"] = "0.0.0.0";
        props["bulkio::custom::data::port"] =  static_cast<CORBA::Long>(0);
        props["bulkio::custom::data::protocol"] = "udp";
        return props;
    }

    redhawk::PropertyMap  _getOutNegotiationProperties() {
        redhawk::PropertyMap props;
        props["bulkio::custom::data::protocol"] = "udp";
        return props;
    }


    /**
     *
     *      Input (Provides) Side - Manager, Transport
     *
     */

    CustomInputTransport* CustomInputManager::createInputTransport(const std::string& transportId,
                                                                   const redhawk::PropertyMap& properties) {

        if ( checkDisableTransport() ) {
            RH_NL_DEBUG("bulkio.custom.InManager", "Custom input transport is disable.");
            return 0;
        }

        RH_NL_DEBUG("bulkio.custom.InManager", "CustomInputManager::createInputTransport");
        for ( redhawk::PropertyMap::const_iterator it = properties.begin(); it != properties.end(); it++) {
            RH_NL_DEBUG("bulkio.custom.InManager", "CustomInputManager::createInputTransport key (from uses): "<<it->id);
        }
        return new CustomInputTransport(this->_port, transportId, properties);
    };

    CF::Properties CustomInputManager::transportProperties() {
        RH_NL_DEBUG("bulkio.custom.InManager", "CustomInputTransport::transportProperties" );
        redhawk::PropertyMap props(_getTransportProperties());
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

    redhawk::PropertyMap CustomInputTransport::getNegotiationProperties() {
        RH_NL_DEBUG("bulkio.custom.InTransport", "CustomInputTransport::getNegotiationProperties" );
        redhawk::PropertyMap props(_getInNegotiationProperties());
        return props;
    }


    /**
     *
     *      Output (Uses) Side - Manager, Transport
     *
     */

    CustomOutputTransport* CustomOutputManager::createOutputTransport(PtrType object,
                                                                      const std::string& connectionId,
                                                                      const redhawk::PropertyMap& inputTransportProps)
    {
        if ( checkDisableTransport() ) {
            RH_NL_DEBUG("bulkio.custom.OutManager", "Custom output transport is disable.");
            return 0;
        }

        return new CustomOutputTransport(this->_port, object, connectionId, inputTransportProps );
    }


    redhawk::PropertyMap CustomOutputManager::getNegotiationProperties(redhawk::UsesTransport* transport) {
        CustomOutputTransport* _transport = dynamic_cast<CustomOutputTransport*>(transport);
        if (!_transport) {
            throw redhawk::FatalTransportError("Invalid transport object provided.");
        }

        redhawk::PropertyMap properties;
        properties =  _transport->getNegotiationProperties();
        return properties;
    }

    void CustomOutputManager::setNegotiationResult(redhawk::UsesTransport* transport, const redhawk::PropertyMap& properties) {


        if (!transport) {
            throw redhawk::FatalTransportError("Invalid transport object provided.");
        }

        RH_NL_DEBUG("bulkio.custom.OutManager", "CustomOutputManager::setNegotiationResult");
        for ( redhawk::PropertyMap::const_iterator it = properties.begin(); it != properties.end(); it++) {
            RH_NL_DEBUG("bulkio.custom.OutManager", "CustomOutputManager::setNegotiationResult key (from provides): "<<it->id );
        }
    }

    redhawk::PropertyMap CustomOutputTransport::getNegotiationProperties() {
        RH_NL_DEBUG("bulkio.custom.OutTransport", "CustomOutputTransport::getNegotiationProperties" );
        redhawk::PropertyMap props(_getOutNegotiationProperties());
        return props;
    }

    CF::Properties CustomOutputManager::transportProperties() {
        RH_NL_DEBUG("bulkio.custom.OutManager", "CustomOutputTransport::transportProperties" );
        redhawk::PropertyMap props(_getTransportProperties());
        return props;
    }



    /**
     *
     *      Factory Implementation
     *
     */

    CustomInputManager* CustomTransportFactory::createInputManager(bulkio::InPort<BULKIO::dataFloat>* port) {
        return new CustomInputManager(port);
    }

    CustomOutputManager* CustomTransportFactory::createOutputManager(OutPortType* port)
    {
        return new CustomOutputManager(port);
    };

    static int initializeModule() {
        static CustomTransportFactory factory;
        RH_NL_DEBUG("bulkio.custom.Factory", "Registering Factory with TransportRegistry");
        redhawk::TransportRegistry::RegisterTransport(&factory);
        return 0;
    }

    static int initialized = initializeModule();


};
