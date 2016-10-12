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
// clone method for boost pointer containers
//

ComponentProperty *ossie::new_clone(const ComponentProperty &a) {
  return a.clone();
}


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
const char* ComponentProperty::getID() const {
    return _id.c_str();
}


//
// SimplePropertyRef
//
const char* SimplePropertyRef::getValue() const {
    return _value.c_str();
}

ComponentProperty* SimplePropertyRef::_clone() const {
    return new SimplePropertyRef(*this);
}

const std::string SimplePropertyRef::_asString() const {
    return "simpleref " +  _id;
}

//
// SimpleSequencePropertyRef
//
ComponentProperty* SimpleSequencePropertyRef::_clone() const {
    return new SimpleSequencePropertyRef(*this);
}

const SimpleSequencePropertyRef::ValuesList & SimpleSequencePropertyRef::getValues() const {
    return _values;
}

const std::string SimpleSequencePropertyRef::_asString() const {
    return "simpleseqref " +  _id;
}

//
// StructPropertyRef
//
StructPropertyRef::~StructPropertyRef() {
}


ComponentProperty* StructPropertyRef::_clone() const {
    return new StructPropertyRef(*this);
}

const StructPropertyRef::ValuesMap & StructPropertyRef::getValue() const {
    return _values;
}

const std::string StructPropertyRef::_asString() const {
    return "structref " +  _id;
}

//
// StructSequencePropertyRef
//
StructSequencePropertyRef::~StructSequencePropertyRef() {
}


ComponentProperty* StructSequencePropertyRef::_clone() const {
    return new StructSequencePropertyRef(*this);
}

const StructSequencePropertyRef::ValuesList & StructSequencePropertyRef::getValues() const {
    return _values;
}

const std::string StructSequencePropertyRef::_asString() const {
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
    affinityProperties  = other.affinityProperties;
    loggingConfig  = other.loggingConfig;
    properties = other.properties;

}

ComponentInstantiation& ComponentInstantiation::operator=(const ComponentInstantiation &other) {
    instantiationId = other.instantiationId;
    _startOrder = other._startOrder;
    usageName = other.usageName;
    namingservicename = other.namingservicename;
    properties = other.properties;
    affinityProperties  = other.affinityProperties;
    loggingConfig  = other.loggingConfig;
    return *this;
}

ComponentInstantiation::~ComponentInstantiation() {
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

const ossie::ComponentPropertyList & ComponentInstantiation::getProperties() const {
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


const ComponentInstantiation::AffinityProperties &ComponentInstantiation::getAffinity() const {
  return affinityProperties;
}

const ComponentInstantiation::LoggingConfig &ComponentInstantiation::getLoggingConfig() const {
  return loggingConfig;
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
