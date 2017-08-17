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

#include "PropertyRef.h"
#include "UsesDevice.h"

namespace ossie {

    class Reservation {
    public:
        std::string kind;
        std::string value;

        const std::string& getKind() const {
            return kind;
        }

        const std::string& getValue() const {
            return value;
        }

        void overloadValue(std::string& new_value) {
            value = new_value;
        }

    };

    inline std::ostream& operator<<(std::ostream& out, const Reservation& resrv)
    {
        out << "Reservation kind: " << resrv.kind;
        return out;
    };

    class SoftwareAssembly {
    public:
        class HostCollocation {
            public:
                std::string id;
                std::string name;
                std::vector<ComponentPlacement> placements;
                std::vector<UsesDeviceRef>      usesdevicerefs;
                std::vector<Reservation>        reservations;

                const std::string& getID() const {
                    return id;
                }

                const std::string& getName() const {
                    return name;
                }

                const std::vector<ComponentPlacement>& getComponents() const {
                    return placements;
                }

                const std::vector<UsesDeviceRef>& getUsesDeviceRefs() const {
                    return usesdevicerefs;
                }

                const std::vector<Reservation>& getReservations() const {
                    return reservations;
                }

                void overloadReservation(std::string &value, int idx) {
                    reservations[idx].overloadValue(value);
                }

                const ComponentInstantiation* getInstantiation(const std::string& refid) const;
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

            const std::string& getExternalName() const
            {
                if (externalname.empty()) {
                    return identifier;
                } else {
                    return externalname;
                }
            }
        };

        class Property {
        public:
            std::string comprefid;
            std::string propid;
            std::string externalpropid;

            const std::string& getExternalID() const
            {
                if (externalpropid.empty()) {
                    return propid;
                } else {
                    return externalpropid;
                }
            }
        };

        class Option {
        public:
            std::string name;
            std::string value;

            const std::string& getName() const
            {
                return name;
            }

            const std::string& getValue() const
            {
                return value;
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
                std::vector<SoftwareAssembly::Option> options;
                std::vector<UsesDevice> usesdevice;
        };
       
        SoftwareAssembly();

        SoftwareAssembly(std::istream& input) throw (ossie::parser_error);

        void load(std::istream& input) throw (ossie::parser_error);

        const std::string& getID() const;

        const std::string& getName() const;

        const std::vector<ComponentFile>& getComponentFiles() const;

        std::vector<ComponentPlacement> getAllComponents() const;

        const std::vector<ComponentPlacement>& getComponentPlacements() const;

        const std::vector<HostCollocation>& getHostCollocations() const;

        const std::vector<Connection>& getConnections() const;

        const ComponentFile* getComponentFile(const std::string& refid) const;

        const std::string& getAssemblyControllerRefId() const;

        const ComponentPlacement* getAssemblyControllerPlacement() const;

        const std::vector<SoftwareAssembly::Port>& getExternalPorts() const;

        const std::vector<SoftwareAssembly::Property>& getExternalProperties() const;

        const std::vector<SoftwareAssembly::Option>& getOptions() const;

        const std::vector<UsesDevice>& getUsesDevices() const;

        const ComponentInstantiation* getComponentInstantiation(const std::string& refid) const;

    protected:
        void validateComponentPlacements(std::vector<ComponentPlacement>& placements);
        void validateExternalPorts(std::vector<Port>& ports);
        void validateExternalProperties(std::vector<Property>& properties);

        std::auto_ptr<SAD> _sad;
    };
}
#endif
