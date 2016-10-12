/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __SOFTWARE_ASSEMBLY_H__
#define __SOFTWARE_ASSEMBLY_H__

#include<memory>
#include<istream>
#include <boost/shared_ptr.hpp>
#include"ossie/componentProfile.h"
#include"ossie/exceptions.h"

namespace ossie {
    class SoftwareAssembly {
    public:
        class HostCollocation {
            public:
                std::string id;
                std::string name;
                std::vector<ComponentPlacement> placements;

                const char* getID() const {
                    return id.c_str();
                }

                const char* getName() const {
                    return name.c_str();
                }

                const std::vector<ComponentPlacement>& getComponents() const {
                    return placements;
                }
        };

        class Partitioning {
            public:
                std::vector<ComponentPlacement> placements; // Contains all placements, even those in host collocations
                std::vector<HostCollocation> collocations;
        };

        class Port {
            public:
                typedef enum {
                    NONE = 0,
                    USESIDENTIFIER,
                    PROVIDESIDENTIFIER,
                    SUPPORTEDIDENTIFIER
                } port_type;

                std::string componentrefid;
                std::string identifier;
                std::string externalname;
                port_type type;
        };

        class Property {
        public:
            std::string comprefid;
            std::string propid;
            std::string externalpropid;
        };

        class PropertyRef {
        public:
            PropertyRef (ComponentProperty* prop) :
                property (prop)
            {
            }

            PropertyRef(const ComponentProperty &prop) :
                property(prop.clone())
            {
            }

            PropertyRef (const PropertyRef& copy) :
                property (copy.property->clone())
            {
            }

            virtual ~PropertyRef ()
            {
              
            }

            std::string refId;
            boost::shared_ptr< ossie::ComponentProperty > property;

        };

        class UsesDevice {
        public:
            std::string id;
            std::string type;
            std::vector<PropertyRef> dependencies;

            const char* getId() const {
                return id.c_str();
            }

            const char* getType() const {
                return type.c_str();
            }

            const std::vector<PropertyRef>& getDependencies() const {
                return dependencies;
            }
        };

        class SAD {
            public:
                std::string id;
                std::string name;
                std::string assemblycontroller;
                Partitioning partitioning;
                std::vector<Connection> connections;
                std::vector<ComponentFile> componentfiles;
                std::vector<SoftwareAssembly::Port> externalports;
                std::vector<SoftwareAssembly::Property> externalproperties;
                std::vector<SoftwareAssembly::UsesDevice> usesdevice;
        };
       
        SoftwareAssembly() : _sad(0) {}

        SoftwareAssembly(std::istream& input) throw (ossie::parser_error);

        void load(std::istream& input) throw (ossie::parser_error);

        const char* getID() const;

        const char* getName() const;

        const std::vector<ComponentFile>& getComponentFiles() const;

        std::vector<ComponentPlacement> getAllComponents() const;

        const std::vector<HostCollocation>& getHostCollocations() const;

        const std::vector<Connection>& getConnections() const;

        const char* getSPDById(const char* refid) const;

        const char* getAssemblyControllerRefId() const;

        const std::vector<SoftwareAssembly::Port>& getExternalPorts() const;

        const std::vector<SoftwareAssembly::Property>& getExternalProperties() const;

        const std::vector<SoftwareAssembly::UsesDevice>& getUsesDevices() const;

    protected:
        std::auto_ptr<SAD> _sad;
    };
}
#endif
