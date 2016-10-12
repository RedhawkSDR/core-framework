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

#ifndef __DEVICE_MANAGER_CONFIGURATION__
#define __DEVICE_MANAGER_CONFIGURATION__

#include <sstream>
#include <istream>
#include <vector>
#include <string>
#include <stdexcept>
#include "ossie/debug.h"
#include "ossie/exceptions.h"
#include "ossie/ossieparser.h"
#include "ossie/componentProfile.h"

namespace ossie {
    class DeviceManagerConfiguration {

    ENABLE_LOGGING

    public:
        /**
         *
         */
        class DCD {
        public:
            std::string id;
            std::string name;
            std::string deviceManagerSoftPkg;
            std::string domainManagerName;
            std::vector<ComponentFile> componentFiles;
            std::vector<ComponentPlacement> componentPlacements;
            std::vector<Connection> connections;
        };

    public:
        /**
         * Create a DeviceManagerConfiguration that has not yet been populated with
         * information from a DCD file.  You must call load() before calling any
         * other functions on this class.
         */
        DeviceManagerConfiguration() : _dcd(0) {}

        /**
         * Create a DeviceManagerConfiguration, parsing the DCD information provided by input.
         */
        DeviceManagerConfiguration(std::istream& input) throw (ossie::parser_error);

        /**
         * Assignment operator.
         */
        DeviceManagerConfiguration& operator=(DeviceManagerConfiguration other)
        {
            _dcd = other._dcd;
            return *this;
        }

    public:
        void load(std::istream& input) throw (ossie::parser_error);

        const char* getID() const;

        const char* getName() const;

        const char* getDeviceManagerSoftPkg() const;

        const char* getDomainManagerName() const;
        
        const std::vector<ComponentFile>& getComponentFiles();

        const std::vector<ComponentPlacement>& getComponentPlacements();

        const std::vector<Connection>& getConnections();

        const char* getFileNameFromRefId(const char* refid);

        const ComponentInstantiation& getComponentInstantiationById(std::string id) throw(std::out_of_range);

    private:
        std::auto_ptr<DCD> _dcd;

    };
}
#endif
