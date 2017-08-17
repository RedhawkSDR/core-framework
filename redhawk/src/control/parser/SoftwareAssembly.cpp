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

#include <boost/foreach.hpp>

#include "ossie/SoftwareAssembly.h"
#include "internal/sad-parser.h"

using namespace ossie;

const ComponentInstantiation* SoftwareAssembly::HostCollocation::getInstantiation(const std::string& refid) const
{
    BOOST_FOREACH(const ComponentPlacement& placement, placements) {
        const ComponentInstantiation* instantiation = placement.getInstantiation(refid);
        if (instantiation) {
            return instantiation;
        }
    }
    return 0;
}

SoftwareAssembly::SoftwareAssembly() :
    _sad(0)
{
}

SoftwareAssembly::SoftwareAssembly(std::istream& input) throw (ossie::parser_error) :
    _sad(0)
{
    this->load(input);
}

void SoftwareAssembly::load(std::istream& input) throw (ossie::parser_error) 
{
    _sad = ossie::internalparser::parseSAD(input);

    // Validate that all componentplacement elements, both in hostcollocation
    // elements and in partitioning, have componentfileref values referencing
    // valid componentfile elements
    BOOST_FOREACH(HostCollocation& collocation, _sad->partitioning.collocations) {
        validateComponentPlacements(collocation.placements);
    }
    validateComponentPlacements(_sad->partitioning.placements);

    // Make sure assemblycontroller is set, and references a real component
    if (_sad->assemblycontroller.empty()) {
        throw ossie::parser_error("assemblycontroller is not set");
    }
    if (!getComponentInstantiation(_sad->assemblycontroller)) {
        throw ossie::parser_error("assemblycontroller has invalid componentinstantiationref '" + _sad->assemblycontroller + "'");
    }

    validateExternalPorts(_sad->externalports);
    validateExternalProperties(_sad->externalproperties);
}

void SoftwareAssembly::validateComponentPlacements(std::vector<ComponentPlacement>& placements)
{
    BOOST_FOREACH(ComponentPlacement& placement, placements) {
        const std::string& file_ref = placement.getFileRefId();
        const ComponentFile* file = getComponentFile(file_ref);
        if (!file) {
            throw ossie::parser_error("componentplacement has invalid componentfileref '" + file_ref + "'");
        }
        placement.filename = file->filename;
    }
}

void SoftwareAssembly::validateExternalPorts(std::vector<SoftwareAssembly::Port>& ports)
{
    BOOST_FOREACH(SoftwareAssembly::Port& port, ports) {
        if (!getComponentInstantiation(port.componentrefid)) {
            throw ossie::parser_error("external port '" + port.getExternalName() + "' has invalid componentrefid '" + port.componentrefid + "'");
        }
    }
}

void SoftwareAssembly::validateExternalProperties(std::vector<SoftwareAssembly::Property>& properties)
{
    BOOST_FOREACH(SoftwareAssembly::Property& property, properties) {
        if (!getComponentInstantiation(property.comprefid)) {
            throw ossie::parser_error("external property '" + property.getExternalID() + "' has invalid comprefid '" + property.comprefid + "'");
        }
    }
}

const std::string& SoftwareAssembly::getID() const {
    assert(_sad.get() != 0);
    return _sad->id;
}

const std::string& SoftwareAssembly::getName() const {
    assert(_sad.get() != 0);
    return _sad->name;
}

const std::vector<ComponentFile>& SoftwareAssembly::getComponentFiles() const {
    assert(_sad.get() != 0);
    return _sad->componentfiles;
}

std::vector<ComponentPlacement> SoftwareAssembly::getAllComponents() const {
    assert(_sad.get() != 0);
    std::vector<ComponentPlacement> result;
    std::copy(_sad->partitioning.placements.begin(),
              _sad->partitioning.placements.end(),
              back_inserter(result));

    std::vector<HostCollocation>::iterator hc_iter;
    for (hc_iter = _sad->partitioning.collocations.begin(); 
        hc_iter != _sad->partitioning.collocations.end(); ++hc_iter) {

        std::copy(hc_iter->placements.begin(),
            hc_iter->placements.end(),
            back_inserter(result));
    }

    return result;
}

const std::vector<ComponentPlacement>& SoftwareAssembly::getComponentPlacements() const
{
    assert(_sad.get() != 0);
    return _sad->partitioning.placements; 
}

const std::vector<SoftwareAssembly::HostCollocation>& SoftwareAssembly::getHostCollocations() const {
    assert(_sad.get() != 0);
    return _sad->partitioning.collocations; 
}

const std::vector<Connection>& SoftwareAssembly::getConnections() const {
    assert(_sad.get() != 0);
    return _sad->connections; 
}

const ComponentFile* SoftwareAssembly::getComponentFile(const std::string& refid) const {
    assert(_sad.get() != 0);
    const std::vector<ComponentFile>& componentFiles = getComponentFiles();
    std::vector<ComponentFile>::const_iterator i;
    for (i = componentFiles.begin(); i != componentFiles.end(); ++i) {
        if (i->getID() == refid) {
            return &(*i);
        }
    }

    return 0;
}

const std::string& SoftwareAssembly::getAssemblyControllerRefId() const {
    assert(_sad.get() != 0);
    return _sad->assemblycontroller;
}

const std::vector<SoftwareAssembly::Port>& SoftwareAssembly::getExternalPorts() const {
    assert(_sad.get() != 0);
    return _sad->externalports;
}

const std::vector<SoftwareAssembly::Property>& SoftwareAssembly::getExternalProperties() const {
    assert(_sad.get() != 0);
    return _sad->externalproperties;
}

const std::vector<SoftwareAssembly::Option>& SoftwareAssembly::getOptions() const {
    assert(_sad.get() != 0);
    return _sad->options;
}

const std::vector<UsesDevice>& SoftwareAssembly::getUsesDevices() const {
    assert(_sad.get() != 0);
    return _sad->usesdevice;
}

const ComponentPlacement * SoftwareAssembly::getAssemblyControllerPlacement() const
{
   BOOST_FOREACH(const ComponentPlacement& placement, _sad->partitioning.placements) {
        const ComponentInstantiation* instantiation = placement.getInstantiation(_sad->assemblycontroller);
        if (instantiation) return &(placement);
    }

    BOOST_FOREACH(HostCollocation& collocation, _sad->partitioning.collocations) {
        BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
            const ComponentInstantiation* instantiation = placement.getInstantiation(_sad->assemblycontroller);
            if (instantiation) return &(placement);
        }
    }
   return NULL;
}

const ComponentInstantiation* SoftwareAssembly::getComponentInstantiation(const std::string& refid) const
{
    BOOST_FOREACH(HostCollocation& collocation, _sad->partitioning.collocations) {
        const ComponentInstantiation* instantiation = collocation.getInstantiation(refid);
        if (instantiation) {
            return instantiation;
        }
    }
    BOOST_FOREACH(const ComponentPlacement& placement, _sad->partitioning.placements) {
        const ComponentInstantiation* instantiation = placement.getInstantiation(refid);
        if (instantiation) {
            return instantiation;
        }
    }
    return 0;
}
