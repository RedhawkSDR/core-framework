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

#include<cassert>
#include"ossie/DomainManagerConfiguration.h"
#include"internal/dmd-parser.h"

using namespace ossie;
using namespace dmd;

// The implementation of these functions should come from the XSD produced drivers
// When the XSD changes you will need to update these functions.
PREPARE_LOGGING(DomainManagerConfiguration)

DomainManagerConfiguration::DomainManagerConfiguration(std::istream& input) throw (ossie::parser_error) : _dmd(0) {
    this->load(input);
}

DomainManagerConfiguration::~DomainManagerConfiguration() {

}

DomainManagerConfiguration& DomainManagerConfiguration::operator=(DomainManagerConfiguration other) {
    // Pass ownership
    this->_dmd = other._dmd;
    return *this;
}

const char* DomainManagerConfiguration::getID() const {
    assert(_dmd.get() != 0);
    return _dmd->id.c_str();
}

const char* DomainManagerConfiguration::getName() const {
    assert(_dmd.get() != 0);
    return _dmd->name.c_str();
}

const char* DomainManagerConfiguration::getDomainManagerSoftPkg() const {
    assert(_dmd.get() != 0);
    return _dmd->softpkg.c_str();
}

const char* DomainManagerConfiguration::toString() const {
    assert(_dmd.get() != 0);
    return "";
}

void DomainManagerConfiguration::load(std::istream& input) throw (ossie::parser_error) 
{
    _dmd = ossie::internalparser::parseDMD(input);
}
