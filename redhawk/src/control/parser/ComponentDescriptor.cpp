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


#include <string>
#include <iostream>

#include "ossie/exceptions.h"
#include "ossie/ComponentDescriptor.h"
#include "internal/scd-parser.h"

using namespace scd;
using namespace ossie;

ComponentDescriptor::ComponentDescriptor(std::istream& input) throw (ossie::parser_error) : _scd(0) {
    this->load(input);
}

ComponentDescriptor::~ComponentDescriptor() 
{

}

ComponentDescriptor& ComponentDescriptor::operator=(ComponentDescriptor other) {
    // Transfer ownership
    this->_scd = other._scd;
    return *this;
}

void ComponentDescriptor::load(std::istream& input) throw(ossie::parser_error)
{
    _scd = ossie::internalparser::parseSCD(input);
}

const char* ComponentDescriptor::getComponentType() const {
    assert(_scd.get() != 0);
    return _scd->componentType.c_str();
};

bool ComponentDescriptor::isDevice() const 
{
    assert(_scd.get() != 0);
    if (_scd->componentType == "device")
    { return true; }
    else
    { return false; }
};

bool ComponentDescriptor::isResource() const
{
    assert(_scd.get() != 0);
    if (_scd->componentType == "resource")
    { return true; }
    else
    { return false; }
}

bool ComponentDescriptor::isApplication() const
{
    assert(_scd.get() != 0);
    if (_scd->componentType == "application")
    { return true; }
    else
    { return false; }
}


bool ComponentDescriptor::isDomainManager() const
{
    assert(_scd.get() != 0);
    if (_scd->componentType == "domainmanager")
    { return true; }
    else
    { return false; }
}


bool ComponentDescriptor::isDeviceManager() const
{
    assert(_scd.get() != 0);
    if (_scd->componentType == "devicemanager")
    { return true; }
    else
    { return false; }
}


bool ComponentDescriptor::isService() const
{
    assert(_scd.get() != 0);
    if ((_scd->componentType == "log") ||
        (_scd->componentType == "eventservice") ||
        (_scd->componentType == "filemanager") ||
        (_scd->componentType == "filesystem")) {
        return true;
    } else
    { return false; }
}


bool ComponentDescriptor::isConfigurable() const
{
    assert(_scd.get() != 0);
    if ((_scd->componentType == "resource") ||
        (_scd->componentType == "application") ||
        (_scd->componentType == "devicemanager") ||
        (ComponentDescriptor::isDevice ()) || (ComponentDescriptor::isDomainManager ())) {
        return true;
    } else
    { return false; }
}
