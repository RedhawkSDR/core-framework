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


#include "ossie/DeviceManagerConfiguration.h"
#include "internal/dcd-parser.h"

using namespace ossie;

PREPARE_LOGGING(DeviceManagerConfiguration);

DeviceManagerConfiguration::DeviceManagerConfiguration(std::istream& input) throw (ossie::parser_error) {
    this->load(input);
}

void DeviceManagerConfiguration::load(std::istream& input) throw (ossie::parser_error) 
{
    _dcd = ossie::internalparser::parseDCD(input);
}

const char* DeviceManagerConfiguration::getID() const {
    assert(_dcd.get() != 0);
    return _dcd->id.c_str();
}

const char* DeviceManagerConfiguration::getName() const {
    assert(_dcd.get() != 0);
    return _dcd->name.c_str();
}

const char* DeviceManagerConfiguration::getDeviceManagerSoftPkg() const {
    assert(_dcd.get() != 0);
    return _dcd->deviceManagerSoftPkg.c_str();
}

const char* DeviceManagerConfiguration::getDomainManagerName() const {
    assert(_dcd.get() != 0);
    return _dcd->domainManagerName.c_str();
}
    
const std::vector<ComponentFile>& DeviceManagerConfiguration::getComponentFiles() {
    assert(_dcd.get() != 0);
    return _dcd->componentFiles;
}

const std::vector<ComponentPlacement>& DeviceManagerConfiguration::getComponentPlacements() {
    assert(_dcd.get() != 0);
    return _dcd->componentPlacements;
}

const std::vector<Connection>& DeviceManagerConfiguration::getConnections() {
    assert(_dcd.get() != 0);
    return _dcd->connections;
}

const char* DeviceManagerConfiguration::getFileNameFromRefId(const char* refid) {
    assert(_dcd.get() != 0);
    const std::vector<ComponentFile>& componentFiles = getComponentFiles();
    std::vector<ComponentFile>::const_iterator i;
    for (i = componentFiles.begin(); i != componentFiles.end(); ++i) {
        if (strcmp(i->getID(), refid) == 0) {
            return i->getFileName();
        }
    }

    return 0;
}

const ComponentInstantiation& DeviceManagerConfiguration::getComponentInstantiationById(std::string id) throw(std::out_of_range) {
    const std::vector<ComponentPlacement>& componentPlacements = getComponentPlacements();
    std::vector<ComponentPlacement>::const_iterator i;
    for (i = componentPlacements.begin(); i != componentPlacements.end(); ++i) {
        assert(i->getInstantiations().size() > 0);
        const ComponentInstantiation& instantiation = i->getInstantiations().at(0);
        std::string componentid(instantiation.getID());
        if (componentid == id) {
            return instantiation;
        }
    }
    throw std::out_of_range("No instantiation with id " + id);
}
