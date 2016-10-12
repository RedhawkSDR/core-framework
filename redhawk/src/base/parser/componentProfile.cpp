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

#include "ossie/componentProfile.h"

using namespace ossie;

//
// ComponentFile
//
const char* ComponentFile::getFileName() const {
    return filename.c_str();
};

const char* ComponentFile::getID() const {
    return id.c_str();
};

//
// ComponentProperty
//
ComponentProperty::~ComponentProperty() {
};

const char* ComponentProperty::getID() const {
    return _id.c_str();
}

//
// SimplePropertyRef
//
const char* SimplePropertyRef::getValue() const {
    return _value.c_str();
}

ComponentProperty* SimplePropertyRef::clone() const {
    return new SimplePropertyRef(*this);
}

const std::string SimplePropertyRef::asString() const {
    return "simpleref " +  _id;
}

//
// SimpleSequencePropertyRef
//
ComponentProperty* SimpleSequencePropertyRef::clone() const {
    return new SimpleSequencePropertyRef(*this);
}

const std::vector<std::string>& SimpleSequencePropertyRef::getValues() const {
    return _values;
}

const std::string SimpleSequencePropertyRef::asString() const {
    return "simpleseqref " +  _id;
}

//
// StructPropertyRef
//
ComponentProperty* StructPropertyRef::clone() const {
    return new StructPropertyRef(*this);
}

const std::map<std::string, std::string>& StructPropertyRef::getValue() const {
    return _values;
}

const std::string StructPropertyRef::asString() const {
    return "structref " +  _id;
}

//
// StructSequencePropertyRef
//
ComponentProperty* StructSequencePropertyRef::clone() const {
    return new StructSequencePropertyRef(*this);
}

const std::vector<std::map<std::string, std::string> >& StructSequencePropertyRef::getValues() const {
    return _values;
}

const std::string StructSequencePropertyRef::asString() const {
    return "structseqref " +  _id;
}

//
// ComponentInstantiation
//
ComponentInstantiation::ComponentInstantiation() 
{
}

ComponentInstantiation::ComponentInstantiation(const ComponentInstantiation& other) {
    instantiationId = other.instantiationId;
    _startOrder = other._startOrder;
    usageName = other.usageName;
    namingservicename = other.namingservicename;
    std::vector<ComponentProperty*>::const_iterator i;
    for (i = other.properties.begin(); i != other.properties.end(); ++i) {
        properties.push_back((*i)->clone());
    }
}

ComponentInstantiation& ComponentInstantiation::operator=(ComponentInstantiation other) {
    instantiationId = other.instantiationId;
    _startOrder = other._startOrder;
    usageName = other.usageName;
    namingservicename = other.namingservicename;
    properties.resize(other.properties.size());
    properties.swap(other.properties);
    return *this;
}

ComponentInstantiation::~ComponentInstantiation() {
    std::vector<ComponentProperty*>::iterator i;
    for (i = properties.begin(); i != properties.end(); ++i) {
        delete *i;
    }
    properties.clear();
}

const char* ComponentInstantiation::getID() const {
    return instantiationId.c_str();
}

const char* ComponentInstantiation::getStartOrder() const {
    return _startOrder.c_str();
}

const char* ComponentInstantiation::getUsageName() const {
    if (usageName.isSet()) {
        return usageName->c_str();
    } else {
        return 0;
    }
}

const std::vector<ComponentProperty*>& ComponentInstantiation::getProperties() const {
    return properties;
}

bool ComponentInstantiation::isNamingService() const {
    return namingservicename.isSet();
}

const char* ComponentInstantiation::getFindByNamingServiceName() const {
    if (namingservicename.isSet()) {
        return namingservicename->c_str();
    } else {
        return 0;
    }
}

//
// ComponentPlacement
//
const char* ComponentPlacement::getDeployOnDeviceID() const {
    if (deployOnDeviceID.isSet()) {
        return deployOnDeviceID->c_str();
    } else {
        return 0;
    }
}

const char* ComponentPlacement::getCompositePartOfDeviceID() const {
    if (compositePartOfDeviceID.isSet()) {
        return compositePartOfDeviceID->c_str();
    } else {
        return 0;
    }
}

const std::string ComponentPlacement::getDPDFile() const {
    if (DPDFile.isSet()) {
        return DPDFile->c_str();
    } else {
        return 0;
    }
}

const std::vector<ComponentInstantiation>& ComponentPlacement::getInstantiations() const {
    return instantiations;
};

const char* ComponentPlacement::getFileRefId() const {
    return _componentFileRef.c_str();
}

bool ComponentPlacement::isDeployOn() const {
    return deployOnDeviceID.isSet();
}

bool ComponentPlacement::isCompositePartOf() const {
    return compositePartOfDeviceID.isSet();
}

bool ComponentPlacement::isDomainManager() const {
    return false;
}
