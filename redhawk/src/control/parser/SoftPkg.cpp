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

#include <sstream>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <ossie/SoftPkg.h>
#include <ossie/Properties.h>
#include <ossie/ComponentDescriptor.h>

#include "internal/spd-parser.h"

using namespace ossie;

SoftPkg::SoftPkg() :
    _spd(0),
    _spdFile()
{
}

SoftPkg::SoftPkg(std::istream& input, const std::string& spdFile) throw (ossie::parser_error) {
    this->load(input, spdFile);
}

void SoftPkg::load(std::istream& input, const std::string& spdFile) throw (ossie::parser_error) 
{
    _spd = ossie::internalparser::parseSPD(input);

    _spdFile = spdFile;
    _spdPath  = spdFile.substr(0, _spdFile.find_last_of('/'));

    // Convert relative paths to absolute paths
    // This feels awkward here, but seems to be the best place to do it
    if (_spd->properties.isSet() && (*_spd->properties)[0] != '/') {
        _spd->properties = _spdPath + "/" + (*_spd->properties);
    }

    if (_spd->descriptor.isSet() && (*_spd->descriptor)[0] != '/') {
        _spd->descriptor = _spdPath + "/" + (*_spd->descriptor);
    }
   
    std::vector<SPD::Implementation>::iterator ii;
    for (ii = _spd->implementations.begin(); ii != _spd->implementations.end(); ++ii) {
        if (ii->prfFile.isSet() && (*ii->prfFile)[0] != '/') {
            ii->prfFile = _spdPath + "/" + (*ii->prfFile);
        }
    }
}

void SoftPkg::loadProperties(std::istream& input)
{
    _properties = boost::make_shared<Properties>();
    _properties->load(input);
}

void SoftPkg::loadDescriptor(std::istream& input)
{
    _descriptor = boost::make_shared<ComponentDescriptor>();
    _descriptor->load(input);
}

const SPD::Implementation* SoftPkg::getImplementation(const std::string& id) const
{
    assert(_spd.get() != 0);
    BOOST_FOREACH(const SPD::Implementation& implementation, _spd->implementations) {
        if (id == implementation.getID()) {
            return &implementation;
        }
    }

    return 0;
}

const std::string SPD::SoftPkgRef::asString() const {
    std::ostringstream out;
    out << "SoftPkgRef localfile: " << this->localfile << " implref: " << this->implref;
    return out.str();
}

std::ostream& ossie::operator<<(std::ostream& out, SPD::Code::CodeType type)
{
    switch (type) {
    case SPD::Code::EXECUTABLE:
        out << "Executable";
        break;
    case SPD::Code::KERNEL_MODULE:
        out << "KernelModule";
        break;
    case SPD::Code::SHARED_LIBRARY:
        out << "SharedLibrary";
        break;
    case SPD::Code::DRIVER:
        out << "Driver";
        break;
    default:
        break;
    }
    return out;
}

std::ostream& ossie::operator<<(std::ostream& out, const SPD::Code& code)
{
    out << "localfile: " << code.localfile << " type: " << code.type << " entrypoint: " << code.entrypoint;
    return out;
}
