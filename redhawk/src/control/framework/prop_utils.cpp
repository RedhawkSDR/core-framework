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
#include <vector>
#include <cstdlib>
#include <sstream>
#include <map>
#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

#include <ossie/CorbaUtils.h>
#include <ossie/prop_utils.h>
#include <ossie/debug.h>

using namespace ossie;

CREATE_LOGGER(prop_utils)

CF::DataType ossie::convertPropertyToDataType(const Property* prop) {
    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simp = dynamic_cast<const SimpleProperty*>(prop);
        return convertPropertyToDataType(simp);
    } else if (dynamic_cast<const SimpleSequenceProperty*>(prop) != NULL) {
        const SimpleSequenceProperty* simpseq = dynamic_cast<const SimpleSequenceProperty*>(prop);
        return convertPropertyToDataType(simpseq);
    } else if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return convertPropertyToDataType(stru);
    } else if (dynamic_cast<const StructSequenceProperty*>(prop) != NULL) {
        const StructSequenceProperty* struseq = dynamic_cast<const StructSequenceProperty*>(prop);
        return convertPropertyToDataType(struseq);
    }
    return CF::DataType();
}

CF::DataType ossie::overridePropertyValue(const Property* prop, const ComponentProperty* compprop) {
    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simp = dynamic_cast<const SimpleProperty*>(prop);
        return overridePropertyValue(simp, compprop);
    } else if (dynamic_cast<const SimpleSequenceProperty*>(prop) != NULL) {
        const SimpleSequenceProperty* simpseq = dynamic_cast<const SimpleSequenceProperty*>(prop);
        return overridePropertyValue(simpseq, compprop);
    } else if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return overridePropertyValue(stru, compprop);
    } else if (dynamic_cast<const StructSequenceProperty*>(prop) != NULL) {
        const StructSequenceProperty* struseq = dynamic_cast<const StructSequenceProperty*>(prop);
        return overridePropertyValue(struseq, compprop);
    }
    return CF::DataType();
}

CF::DataType ossie::overridePropertyValue(const Property* prop, const ComponentProperty* compprop, const CF::Properties& configureProperties) {
    if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return overridePropertyValue(stru, compprop, configureProperties);
    }
    return CF::DataType();
}

CF::DataType ossie::convertPropertyToDataType(const SimpleProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->getValue() != NULL) {
        std::string value(prop->getValue());
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::string_to_any(value, type);
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const SimpleSequenceProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    if (!prop->isNone()) {
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(prop->getValues(), kind);
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->isNone()) {
        return dataType;
    }

    CF::Properties structval_;
    for (std::vector<SimpleProperty>::const_iterator i = prop->getValue().begin();
         i != prop->getValue().end();
         ++i) {

        CF::DataType dt;
        dt.id = CORBA::string_dup(i->getID());
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(i->getType()));
        if (i->getValue()) {
            dt.value = ossie::string_to_any(i->getValue(), type);
            LOG_TRACE(prop_utils, "setting struct item " << i->getID());
        } else {
            LOG_TRACE(prop_utils, "struct item " << i->getID() << " has no default value");
        }
        structval_.length(structval_.length() + 1);
        structval_[structval_.length() - 1] = dt;
    }
    dataType.value <<= structval_;
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructSequenceProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->isNone()) {
        return dataType;
    }

    const std::vector<StructProperty>& propValues = prop->getValues();
    CORBA::AnySeq values;
    values.length(propValues.size());
    for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
        CF::DataType val = convertPropertyToDataType(&propValues[ii]);
        values[ii] = val.value;
    }
    dataType.value <<= values;
    return dataType;
}

static CF::DataType overrideSimpleValue(const SimpleProperty* prop, const std::string& value)
{
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    CORBA::TypeCode_ptr type = ossie::getTypeCode(prop->getType());
    dataType.value = ossie::string_to_any(value, type);
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const SimpleProperty* prop, const ComponentProperty* compprop) {
    const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(compprop);
    if (!simpleref) {
        LOG_WARN(prop_utils, "ignoring attempt to override simple property " << prop->getID() << " because override definition is not a simpleref");
        return convertPropertyToDataType(prop);
    }

    LOG_TRACE(prop_utils, "overriding simple property id " << prop->getID());
    return overrideSimpleValue(prop, simpleref->getValue());
}

CF::DataType ossie::overridePropertyValue(const SimpleSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);
    if (dynamic_cast<const SimpleSequencePropertyRef*>(compprop) != NULL) {
        const SimpleSequencePropertyRef* simpleseqref = dynamic_cast<const SimpleSequencePropertyRef*>(compprop);
        LOG_TRACE(prop_utils, "overriding simpleseq property id " << dataType.id);
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(simpleseqref->getValues(), kind);
    } else {
        LOG_WARN(prop_utils, "ignoring attempt to override simple sequence property " << dataType.id << " because override definition is not a simpleseqref");
    }
    return dataType;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const std::map<std::string, std::string>& values)
{
    const std::vector<SimpleProperty>& simpleProps = prop->getValue();
    LOG_TRACE(prop_utils, "structure has " << simpleProps.size() << " elements");
    CF::Properties structval;
    structval.length(simpleProps.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const SimpleProperty& simple = simpleProps[ii];
        const std::string id = simple.getID();
        std::map<std::string, std::string>::const_iterator itemoverride = values.find(id);
        if (itemoverride == values.end()) {
            LOG_TRACE(prop_utils, "using default value for struct element " << id);
            structval[ii] = convertPropertyToDataType(&simple);
        } else {
            LOG_TRACE(prop_utils, "setting structure element " << id << " to " << itemoverride->second);
            structval[ii] = overrideSimpleValue(&simple, itemoverride->second);
        }
    }

    return structval;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const std::map<std::string, std::string>& values, const CF::Properties& configureProperties)
{
    const std::vector<SimpleProperty>& simpleProps = prop->getValue();
    LOG_TRACE(prop_utils, "structure has " << simpleProps.size() << " elements");
    CF::Properties structval;
    structval.length(simpleProps.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const SimpleProperty& simple = simpleProps[ii];
        const std::string id = simple.getID();
        std::map<std::string, std::string>::const_iterator itemoverride = values.find(id);
        if (itemoverride == values.end()) {
            LOG_TRACE(prop_utils, "using default value for struct element " << id);
            structval[ii] = convertPropertyToDataType(&simple);
        } else {
            LOG_TRACE(prop_utils, "setting structure element " << id << " to " << itemoverride->second);
            std::string value = itemoverride->second;
            if (strncmp(value.c_str(), "__MATH__", 8) == 0) {
                CF::DataType dataType;
                dataType.id = CORBA::string_dup(simple.getID());
                CORBA::TCKind kind = ossie::getTypeKind(simple.getType());
                LOG_TRACE(prop_utils, "Invoking custom OSSIE dynamic allocation property support")
                // Turn propvalue into a string for easy parsing
                std::string mathStatement = value.substr(8);
                if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                    // TODO - implement a more relaxed parser
                    mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                    mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                    std::vector<std::string> args;
                    while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                        args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                        LOG_TRACE(prop_utils, "ARG " << args.back())
                        mathStatement.erase(0, mathStatement.find(',') + 1);
                    }
                    args.push_back(mathStatement);
                    LOG_TRACE(prop_utils, "ARG " << args.back())

                    if (args.size() != 3) {
                        std::ostringstream eout;
                        eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                        throw ossie::PropertyMatchingError(eout.str());
                    }

                    LOG_TRACE(prop_utils, "__MATH__ " << args[0] << " " << args[1] << " " << args[2])

                    double operand;
                    operand = strtod(args[0].c_str(), NULL);

                    // See if there is a property in the component
                    LOG_TRACE(prop_utils, "Attempting to find matching property for " << args[1])
                    const CF::DataType* matchingCompProp = 0;
                    for (unsigned int j = 0; j < configureProperties.length(); j++) {
                        if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                            LOG_TRACE(prop_utils, "Matched property for " << args[1])
                            matchingCompProp = &configureProperties[j];
                        }
                        // See if the property we're looking for is a member of a struct
                        //  **note: this only works because the dtd states that each
                        //          property id is globally unique within the prf
                        const CF::Properties* tmp_ref = NULL;
                        configureProperties[j].value >>= tmp_ref;
                        if (tmp_ref != NULL) {
                            for (unsigned prop_idx = 0; prop_idx<tmp_ref->length(); prop_idx++) {
                                if (strcmp((*tmp_ref)[prop_idx].id, args[1].c_str()) == 0) {
                                    LOG_TRACE(prop_utils, "Matched property for " << args[1])
                                    matchingCompProp = &(*tmp_ref)[prop_idx];
                                }
                            }
                        }
                    }

                    if (matchingCompProp == 0) {
                        std::ostringstream eout;
                        eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                        throw ossie::PropertyMatchingError(eout.str());
                    }

                    std::string math = args[2];
                    CORBA::Any compValue = matchingCompProp->value;
                    dataType.value = ossie::calculateDynamicProp(operand, compValue, math, kind);
                } else {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                    throw ossie::PropertyMatchingError(eout.str());
                }
                structval[ii] = dataType;
            } else {
                structval[ii] = overrideSimpleValue(&simple, itemoverride->second);
            }
        }
    }

    return structval;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        LOG_TRACE(prop_utils, "overriding struct property id " << dataType.id);
        dataType.value <<= overrideStructValues(prop, structref->getValue());
    } else {
        LOG_WARN(prop_utils, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop, const CF::Properties& configureProperties) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        LOG_TRACE(prop_utils, "overriding struct property id " << dataType.id << " (supports __MATH__)");
        dataType.value <<= overrideStructValues(prop, structref->getValue(), configureProperties);
    } else {
        LOG_WARN(prop_utils, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructSequencePropertyRef* structsequenceref = dynamic_cast<const StructSequencePropertyRef*>(compprop);
    if (structsequenceref) {
        LOG_TRACE(prop_utils, "overriding structsequence property id " << dataType.id);

        const std::vector<std::map<std::string, std::string> >& overrideValues = structsequenceref->getValues();
        LOG_TRACE(prop_utils, "structsequence has " << overrideValues.size() << " values");

        CORBA::AnySeq values;
        values.length(overrideValues.size());
        const std::vector<StructProperty>default_values = prop->getValues();
        for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
            StructProperty tmp = prop->getStruct();
            if (ii < default_values.size()) {
                tmp = default_values[ii];
            }
            values[ii] <<= overrideStructValues(&tmp, overrideValues[ii]);
        }
        dataType.value <<= values;
    } else {
        LOG_WARN(prop_utils, "ignoring attempt to override structsequence property " << dataType.id << " because override definition is not a structsequenceref");
    }
    return dataType;
}

bool ossie::checkProcessor(const std::vector<std::string>& processorDeps, const std::vector<const Property*>& props)
{
    if (processorDeps.size() == 0) {
        return true;
    }

    // Interpreting D.2.1.6.8, if more than one processor element is provided assume that the implementation
    // can run on any of those...so once we find one match we are successful
    for (unsigned int j = 0; j < processorDeps.size(); j++) {
        bool matchProcessor = true;

        std::string processor = processorDeps[j];
        if (processor != "") {
            matchProcessor = false;
            LOG_TRACE(prop_utils, "Attempting to match processor " << processor << " against " << props.size() << " properties")
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    LOG_TRACE(prop_utils, "Checking property " << matchingProp->getID() << " " << matchingProp->getName())
                    if (strcmp(matchingProp->getName(), "processor_name") == 0) {
                        const char *tmp_value = matchingProp->getValue();
                        std::string dev_processor_name("");
                        if (tmp_value != NULL) {
                            dev_processor_name = tmp_value;
                        }
                        LOG_TRACE(prop_utils, "Performing comparison operation '" << dev_processor_name << "' " << action << " '" << processor << "'")
                        matchProcessor = ossie::perform_action(dev_processor_name, processor, action);
                        if (matchProcessor) break;
                    }
                }
            }
        }
    
        if (matchProcessor) {
            return true;
        }
    }
    return false;
}

bool ossie::checkOs(const std::vector<ossie::SPD::NameVersionPair>& osDeps, const std::vector<const Property*>& props)
{

    if (osDeps.size() == 0) {
        return true;
    }

    // Interpreting D.2.1.6.7, if more than one processor element is provided assume that the implementation
    // can run on any of those...so once we find one match we are successful
    for (unsigned int j = 0; j < osDeps.size(); j++) {
        bool matchOs = true;
        bool matchOsVersion = true;

        std::string os = osDeps[j].first;
        std::string osVersion = osDeps[j].second;

        if (os != "") {
            matchOs = false;
            LOG_TRACE(prop_utils, "Attempting to match os " << os << " PropertySet Size:" << props.size());
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    LOG_TRACE(prop_utils, "Examine Property: name: " << matchingProp->getName() << " value:" <<
                              matchingProp->getValue() );
                    if (strcmp(matchingProp->getName(), "os_name") == 0) {
                        const char *tmp_dev_os_name = matchingProp->getValue();
                        std::string dev_os_name("");
                        if (tmp_dev_os_name != NULL) {
                            dev_os_name = tmp_dev_os_name;
                        }
                        LOG_TRACE(prop_utils, "Performing comparison operation " << dev_os_name << " " << action << " " << os);
                        matchOs = ossie::perform_action(dev_os_name, os, action);
                        LOG_TRACE(prop_utils, "Performing comparison operation " << dev_os_name << " " << action << " " << os << " RESULT:" << matchOs);
                        if (matchOs) {
                        	break;
                        }
                    }
                }
            }
        }

        LOG_TRACE(prop_utils, "Attempting to match os version");
        if (osVersion != "") {
            matchOsVersion = false;
            LOG_TRACE(prop_utils, "Attempting to match os version" << osVersion)
                    for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    if (strcmp(matchingProp->getName(), "os_version") == 0) {
                        const char *tmp_dev_os_version = matchingProp->getValue();
                        std::string dev_os_version("");
                        if (tmp_dev_os_version != NULL) {
                            dev_os_version = tmp_dev_os_version;
                        }
                        LOG_TRACE(prop_utils, "Performing comparison operation " << dev_os_version << " " << action << " " << osVersion);
                        matchOsVersion = ossie::perform_action(dev_os_version, osVersion, action);
                        LOG_TRACE(prop_utils, "Performing comparison operation " << dev_os_version << " " << action << " " << osVersion << " RESULT:" << matchOsVersion);
                        if (matchOsVersion) break;
                    }
                }
            }
        } else {
        }

        if (matchOs && matchOsVersion) {
            return true;
        }
    }

    return false;
}

CF::AllocationManager::AllocationResponseType ossie::assembleResponse(std::string requestID, std::string allocationID,
		CF::Properties& allocations, CF::Device_ptr dev, CF::DeviceManager_ptr devMgr)
{
	CF::AllocationManager::AllocationResponseType resp;
	resp.requestID = CORBA::string_dup(requestID.c_str());
    resp.allocationID = CORBA::string_dup(allocationID.c_str());
	resp.allocationProperties = allocations;
	resp.allocatedDevice = CF::Device::_duplicate(dev);
	resp.allocationDeviceManager = CF::DeviceManager::_duplicate(devMgr);

	return resp;
}

CF::DataType ossie::convertPropertyToDataType(const SimplePropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    
    if (prop->getValue() != NULL) {
        std::string value(prop->getValue());
        dataType.value <<= value;
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const SimpleSequencePropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    dataType.value = ossie::strings_to_any(prop->getValues(), CORBA::tk_string);
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructPropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    
    CF::Properties structval_;
    for (std::map<std::string, std::string>::const_iterator i = prop->getValue().begin(); i != prop->getValue().end(); ++i) {
        CF::DataType dt;
        dt.id = CORBA::string_dup((*i).first.c_str());
        CORBA::TypeCode *tc = ossie::corba::TypeCode<std::string>();
        dt.value = ossie::string_to_any((*i).second.c_str(), tc);
        LOG_TRACE(prop_utils, "setting struct item " << (*i).first);
        structval_.length(structval_.length() + 1);
        structval_[structval_.length() - 1] = dt;
    }
    dataType.value <<= structval_;
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructSequencePropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    
    const std::vector<std::map<std::string, std::string> > propValues = prop->getValues();
    CORBA::AnySeq values;
    values.length(propValues.size());
    for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
        CF::DataType tmp_struct;
        tmp_struct.id = CORBA::string_dup("");
        CF::Properties structval_;
        for (std::map<std::string, std::string>::const_iterator i = propValues[ii].begin(); i != propValues[ii].end(); ++i) {
            CF::DataType dt;
            dt.id = CORBA::string_dup((*i).first.c_str());
            CORBA::TypeCode *tc = ossie::corba::TypeCode<std::string>();
            dt.value = ossie::string_to_any((*i).second.c_str(), tc);
            LOG_TRACE(prop_utils, "setting struct item " << (*i).first);
            structval_.length(structval_.length() + 1);
            structval_[structval_.length() - 1] = dt;
        }
        tmp_struct.value <<= structval_;
        values[ii] <<= tmp_struct;
    }
    dataType.value <<= values;
    return dataType;
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const Property* property)
{
    if (dynamic_cast<const SimpleProperty*>(property)) {
        return convertAnyToPropertyType(value, dynamic_cast<const SimpleProperty*>(property));
    } else if (dynamic_cast<const SimpleSequenceProperty*>(property)) {
        return convertAnyToPropertyType(value, dynamic_cast<const SimpleSequenceProperty*>(property));
    } else if (dynamic_cast<const StructProperty*>(property)) {
        return convertAnyToPropertyType(value, dynamic_cast<const StructProperty*>(property));
    } else if (dynamic_cast<const StructSequenceProperty*>(property)) {
        return convertAnyToPropertyType(value, dynamic_cast<const StructSequenceProperty*>(property));
    }
    return CORBA::Any();
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const SimpleProperty* property)
{
    // Convert the input Any to the property's data type via string
    return ossie::string_to_any(ossie::any_to_string(value), ossie::getTypeCode(property->getType()));
}

CF::DataType ossie::convertDataTypeToPropertyType(const CF::DataType& value, const Property* property)
{
    CF::DataType result;
    result.id = value.id;
    result.value = convertAnyToPropertyType(value.value, property);
    return result;
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const SimpleSequenceProperty* property)
{
    // TODO
    return CORBA::Any();
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const StructProperty* property)
{
    CORBA::Any result;
    const CF::Properties *depProps;
    if (value >>= depProps) {
        CF::Properties tmp_props;
        std::vector<ossie::SimpleProperty> structval = property->getValue();
        for (unsigned int index = 0; index < depProps->length(); ++index) {
            const CF::DataType& item = (*depProps)[index];
            const std::string propid(item.id);
            const ossie::SimpleProperty* field = property->getField(propid);
            if (field) {
                ossie::corba::push_back(tmp_props, convertDataTypeToPropertyType(item, field));
            }
        }
        result <<= tmp_props;
    }
    return result;
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const StructSequenceProperty* property)
{
    return CORBA::Any();
}
