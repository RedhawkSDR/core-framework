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

#include "ossie/SoftwareAssembly.h"
#include "internal/sad-parser.h"

using namespace ossie;

SoftwareAssembly::SoftwareAssembly(std::istream& input) throw (ossie::parser_error) {
    this->load(input);
}

void SoftwareAssembly::load(std::istream& input) throw (ossie::parser_error) 
{
    _sad = ossie::internalparser::parseSAD(input);
}

const char* SoftwareAssembly::getID() const {
    assert(_sad.get() != 0);
    return _sad->id.c_str();
}

const char* SoftwareAssembly::getName() const {
    assert(_sad.get() != 0);
    return _sad->name.c_str();
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

const std::vector<SoftwareAssembly::HostCollocation>& SoftwareAssembly::getHostCollocations() const {
    assert(_sad.get() != 0);
    return _sad->partitioning.collocations; 
}

const std::vector<Connection>& SoftwareAssembly::getConnections() const {
    assert(_sad.get() != 0);
    return _sad->connections; 
}

const char* SoftwareAssembly::getSPDById(const char* refid) const {
    assert(_sad.get() != 0);
    const std::vector<ComponentFile>& componentFiles = getComponentFiles();
    std::vector<ComponentFile>::const_iterator i;
    for (i = componentFiles.begin(); i != componentFiles.end(); ++i) {
        if (strcmp(i->getID(), refid) == 0) {
            return i->getFileName();
        }
    }

    return 0;
}

const char* SoftwareAssembly::getAssemblyControllerRefId() const {
    assert(_sad.get() != 0);
    return _sad->assemblycontroller.c_str();
}

const std::vector<SoftwareAssembly::Port>& SoftwareAssembly::getExternalPorts() const {
    assert(_sad.get() != 0);
    return _sad->externalports;
}
