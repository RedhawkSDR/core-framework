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


#ifndef PROPUTILS_H
#define PROPUTILS_H

#include <string>
#include <vector>
#include <ossie/prop_helpers.h>

#include "ossie/Properties.h"
#include "ossie/SoftPkg.h"
#include "ossie/componentProfile.h"

#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

namespace ossie
{
    CF::DataType convertPropertyToDataType(const Property* prop);
    CF::DataType convertPropertyRefToDataType(const ComponentProperty* prop);
    CF::DataType convertPropertyRefToDataType(const ComponentProperty& prop);
    CF::DataType convertPropertyToDataType(const SimpleProperty* prop);
    CF::DataType convertPropertyToDataType(const SimpleSequenceProperty* prop);
    CF::DataType convertPropertyToDataType(const StructProperty* prop);
    CF::DataType convertPropertyToDataType(const StructSequenceProperty* prop);

    CF::DataType convertPropertyToDataType(const SimplePropertyRef* prop);
    CF::DataType convertPropertyToDataType(const SimpleSequencePropertyRef* prop);
    CF::DataType convertPropertyToDataType(const StructPropertyRef* prop);
    CF::DataType convertPropertyToDataType(const StructSequencePropertyRef* prop);
    
    CF::DataType overridePropertyValue(const Property* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const SimpleProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const SimpleSequenceProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const StructProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const StructSequenceProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const Property* prop, const ossie::ComponentProperty* compprop, const CF::Properties& configureProperties);
    CF::DataType overridePropertyValue(const StructProperty* prop, const ossie::ComponentProperty* compprop, const CF::Properties& configureProperties);

    CF::AllocationManager::AllocationResponseType assembleResponse(std::string requestID, std::string allocationID,
    		CF::Properties& allocations, CF::Device_ptr dev, CF::DeviceManager_ptr devMgr);

    bool checkProcessor(const std::vector<std::string>& processorDeps, const std::vector<const Property*>& props);
    bool checkOs(const std::vector<ossie::SPD::NameVersionPair>& osDeps, const std::vector<const Property*>& props);

    CF::DataType convertDataTypeToPropertyType(const CF::DataType& value, const Property* prop);

    CORBA::Any convertAnyToPropertyType(const CORBA::Any& value, const Property* prop);
    CORBA::Any convertAnyToPropertyType(const CORBA::Any& value, const SimpleProperty* prop);
    CORBA::Any convertAnyToPropertyType(const CORBA::Any& value, const SimpleSequenceProperty* prop);
    CORBA::Any convertAnyToPropertyType(const CORBA::Any& value, const StructProperty* prop);
    CORBA::Any convertAnyToPropertyType(const CORBA::Any& value, const StructSequenceProperty* prop);

    void convertComponentProperties( const ossie::ComponentPropertyList &cp_props,
                                     CF::Properties &cf_props );
    
    std::string retrieveParserErrorLineNumber(std::string message);

}

#endif
