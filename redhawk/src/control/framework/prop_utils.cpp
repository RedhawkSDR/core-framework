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

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

#include <ossie/CorbaUtils.h>
#include <ossie/prop_utils.h>
#include <ossie/debug.h>
#include <ossie/affinity.h>
#include <ossie/PropertyMap.h>

using namespace ossie;

CREATE_LOGGER(prop_utils)

namespace ossie {
    rh_logger::LoggerPtr proputilsLog;
}

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

CF::DataType ossie::convertPropertyRefToDataType(const ComponentProperty* prop) {
    if (dynamic_cast<const SimplePropertyRef*>(prop) != NULL) {
        const SimplePropertyRef* simpref = dynamic_cast<const SimplePropertyRef*>(prop);
        return convertPropertyToDataType(simpref);
    } else if (dynamic_cast<const SimpleSequencePropertyRef*>(prop) != NULL) {
        const SimpleSequencePropertyRef* simpseqref = dynamic_cast<const SimpleSequencePropertyRef*>(prop);
        return convertPropertyToDataType(simpseqref);
    } else if (dynamic_cast<const StructPropertyRef*>(prop) != NULL) {
        const StructPropertyRef* struref = dynamic_cast<const StructPropertyRef*>(prop);
        return convertPropertyToDataType(struref);
    } else if (dynamic_cast<const StructSequencePropertyRef*>(prop) != NULL) {
        const StructSequencePropertyRef* struseqref = dynamic_cast<const StructSequencePropertyRef*>(prop);
        return convertPropertyToDataType(struseqref);
    }
    return CF::DataType();
}

CF::DataType ossie::convertPropertyRefToDataType(const ComponentProperty& prop) {
  return convertPropertyRefToDataType(&prop);
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
        std::string tmp = static_cast<std::string>(prop->getType());
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(prop->getValues(), kind, type);
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
    const PropertyList& propValue = prop->getValue();
    bool nilSeq = false;
    bool nonNilVal = false;
    std::vector<unsigned int> idxs;
    for (ossie::PropertyList::const_iterator i = propValue.begin(); i != propValue.end(); ++i) {
        if (i->isNone()) {
            if (dynamic_cast<const ossie::SimpleSequenceProperty*>(&(*i)) != NULL) {
                nilSeq = true;
                idxs.push_back(structval_.length());
            }
        } else {
            if (dynamic_cast<const ossie::SimpleProperty*>(&(*i)) != NULL)
                nonNilVal = true;
        }
        CF::DataType dt;
        dt = convertPropertyToDataType(&(*i));
        structval_.length(structval_.length() + 1);
        structval_[structval_.length() - 1] = dt;
    }
    // there's a nil sequence value and a non-nil simple. The nil sequence needs to be a zero-length sequence
    if (nilSeq and nonNilVal) {
        std::vector<std::string> empty_string_vector;
        for (std::vector<unsigned int>::iterator idx=idxs.begin();idx!=idxs.end();++idx) {
            const ossie::SimpleSequenceProperty* _type = dynamic_cast<const ossie::SimpleSequenceProperty*>(&propValue[*idx]);
            structval_[*idx].value = ossie::strings_to_any(empty_string_vector, ossie::getTypeKind(_type->getType()), NULL);
        }
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

static CF::DataType overrideSimpleSequenceValue(const SimpleSequenceProperty* prop, const std::vector<std::string>& values)
{
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
    CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(prop->getType()));
    dataType.value = ossie::strings_to_any(values, kind, type);
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const SimpleProperty* prop, const ComponentProperty* compprop) {
    const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(compprop);
    if (!simpleref) {
        RH_WARN(proputilsLog, "ignoring attempt to override simple property " << prop->getID() << " because override definition is not a simpleref");
        return convertPropertyToDataType(prop);
    }

    RH_TRACE(proputilsLog, "overriding simple property id " << prop->getID());
    return overrideSimpleValue(prop, simpleref->getValue());
}

CF::DataType ossie::overridePropertyValue(const SimpleSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);
    if (dynamic_cast<const SimpleSequencePropertyRef*>(compprop) != NULL) {
        const SimpleSequencePropertyRef* simpleseqref = dynamic_cast<const SimpleSequencePropertyRef*>(compprop);
        RH_TRACE(proputilsLog, "overriding simpleseq property id " << dataType.id);
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(simpleseqref->getValues(), kind, type);
    } else {
        RH_WARN(proputilsLog, "ignoring attempt to override simple sequence property " << dataType.id << " because override definition is not a simpleseqref");
    }
    return dataType;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const ossie::ComponentPropertyMap & values)
{
    const PropertyList& props = prop->getValue();
    RH_TRACE(proputilsLog, "structure has " << props.size() << " elements");
    CF::Properties structval;
    structval.length(props.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const Property* property = &props[ii];
        const std::string id = property->getID();
        ossie::ComponentPropertyMap::const_iterator itemoverride = values.find(id);
        if (itemoverride == values.end()) {
            RH_TRACE(proputilsLog, "using default value for struct element " << id);
            structval[ii] = convertPropertyToDataType(property);
        } else {
            if (dynamic_cast<const SimpleProperty*>(property) != NULL) {
                RH_TRACE(proputilsLog, "setting structure element " << id << " to " << (itemoverride->second)[0]);
                structval[ii] = overrideSimpleValue(dynamic_cast<const SimpleProperty*>(property), static_cast<const SimplePropertyRef*>(itemoverride->second)->getValue());
            } else if (dynamic_cast<const SimpleSequenceProperty*>(property) != NULL) {
                RH_TRACE(proputilsLog, "setting structure element " << id);
                structval[ii] = overrideSimpleSequenceValue(dynamic_cast<const SimpleSequenceProperty*>(property), static_cast<const SimpleSequencePropertyRef*>(itemoverride->second)->getValues()); 
            }
        }
    }

    return structval;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const ossie::ComponentPropertyMap & values, const CF::Properties& configureProperties)
{
    const PropertyList& props = prop->getValue();
    RH_TRACE(proputilsLog, "structure has " << props.size() << " elements");
    CF::Properties structval;
    structval.length(props.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const Property* property = &props[ii];
        const std::string id = property->getID();
        ossie::ComponentPropertyMap::const_iterator itemoverride = values.find(id);
        if (dynamic_cast<const SimplePropertyRef*>(itemoverride->second) != NULL) {
            if (itemoverride == values.end()) {
                RH_TRACE(proputilsLog, "using default value for struct element " << id);
                structval[ii] = convertPropertyToDataType(property);
            } else {
                RH_TRACE(proputilsLog, "setting structure element " << id << " to " << static_cast<const SimplePropertyRef*>(itemoverride->second)->getValue());
                std::string value = static_cast<const SimplePropertyRef*>(itemoverride->second)->getValue();
                if (strncmp(value.c_str(), "__MATH__", 8) == 0) {
                    CF::DataType dataType;
		    const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(property);
                    dataType.id = CORBA::string_dup(simple->getID());
                    CORBA::TCKind kind = ossie::getTypeKind(simple->getType());
                    RH_TRACE(proputilsLog, "Invoking custom OSSIE dynamic allocation property support")
                    // Turn propvalue into a string for easy parsing
                    std::string mathStatement = value.substr(8);
                    if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                        mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                        mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                        std::vector<std::string> args;
                        while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                            args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                            RH_TRACE(proputilsLog, "ARG " << args.back())
                            mathStatement.erase(0, mathStatement.find(',') + 1);
                        }
                        args.push_back(mathStatement);
                        RH_TRACE(proputilsLog, "ARG " << args.back())

                        if (args.size() != 3) {
                            std::ostringstream eout;
                            eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                            throw ossie::PropertyMatchingError(eout.str());
                        }

                        RH_TRACE(proputilsLog, "__MATH__ " << args[0] << " " << args[1] << " " << args[2])

                        double operand;
                        operand = strtod(args[0].c_str(), NULL);

                        // See if there is a property in the component
                        RH_TRACE(proputilsLog, "Attempting to find matching property for " << args[1])
                        const CF::DataType* matchingCompProp = 0;
                        for (unsigned int j = 0; j < configureProperties.length(); j++) {
                            if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                                RH_TRACE(proputilsLog, "Matched property for " << args[1])
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
                                        RH_TRACE(proputilsLog, "Matched property for " << args[1])
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
                    structval[ii] = overrideSimpleValue(static_cast<const SimpleProperty*>(property), static_cast<const SimplePropertyRef*>(itemoverride->second)->getValue());
                }
            } 
        } else if (dynamic_cast<const SimpleSequencePropertyRef*>(itemoverride->second) != NULL) {
	    if (itemoverride == values.end()) {
                RH_TRACE(proputilsLog, "using default value for struct element " << id);
                structval[ii] = convertPropertyToDataType(property);
            } else {
		structval[ii] = overrideSimpleSequenceValue(static_cast<const SimpleSequenceProperty*>(property), static_cast<const SimpleSequencePropertyRef*>(itemoverride->second)->getValues());
            }
	}
    }

    return structval;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        RH_TRACE(proputilsLog, "overriding struct property id " << dataType.id);
        dataType.value <<= overrideStructValues(prop, structref->getValue());
    } else {
        RH_WARN(proputilsLog, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop, const CF::Properties& configureProperties) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        RH_TRACE(proputilsLog, "overriding struct property id " << dataType.id << " (supports __MATH__)");
        dataType.value <<= overrideStructValues(prop, structref->getValue(), configureProperties);
    } else {
        RH_WARN(proputilsLog, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructSequencePropertyRef* structsequenceref = dynamic_cast<const StructSequencePropertyRef*>(compprop);
    if (structsequenceref) {
        RH_TRACE(proputilsLog, "overriding structsequence property id " << dataType.id);

        const StructSequencePropertyRef::ValuesList& overrideValues = structsequenceref->getValues();
        RH_TRACE(proputilsLog, "structsequence has " << overrideValues.size() << " values");

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
        RH_WARN(proputilsLog, "ignoring attempt to override structsequence property " << dataType.id << " because override definition is not a structsequenceref");
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
            RH_TRACE(proputilsLog, "Attempting to match processor " << processor << " against " << props.size() << " properties")
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    RH_TRACE(proputilsLog, "Checking property " << matchingProp->getID() << " " << matchingProp->getName())
                    if (strcmp(matchingProp->getName(), "processor_name") == 0) {
                        const char *tmp_value = matchingProp->getValue();
                        std::string dev_processor_name("");
                        if (tmp_value != NULL) {
                            dev_processor_name = tmp_value;
                        }
                        RH_TRACE(proputilsLog, "Performing comparison operation '" << dev_processor_name << "' " << action << " '" << processor << "'")
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
            RH_TRACE(proputilsLog, "Attempting to match os " << os << " PropertySet Size:" << props.size());
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    RH_TRACE(proputilsLog, "Examine Property: name: " << matchingProp->getName() << " value:" <<
                              matchingProp->getValue() );
                    if (strcmp(matchingProp->getName(), "os_name") == 0) {
                        const char *tmp_dev_os_name = matchingProp->getValue();
                        std::string dev_os_name("");
                        if (tmp_dev_os_name != NULL) {
                            dev_os_name = tmp_dev_os_name;
                        }
                        RH_TRACE(proputilsLog, "Performing comparison operation " << dev_os_name << " " << action << " " << os);
                        matchOs = ossie::perform_action(dev_os_name, os, action);
                        RH_TRACE(proputilsLog, "Performing comparison operation " << dev_os_name << " " << action << " " << os << " RESULT:" << matchOs);
                        if (matchOs) {
                        	break;
                        }
                    }
                }
            }
        }

        RH_TRACE(proputilsLog, "Attempting to match os version");
        if (osVersion != "") {
            matchOsVersion = false;
            RH_TRACE(proputilsLog, "Attempting to match os version" << osVersion)
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
                        RH_TRACE(proputilsLog, "Performing comparison operation " << dev_os_version << " " << action << " " << osVersion);
                        matchOsVersion = ossie::perform_action(dev_os_version, osVersion, action);
                        RH_TRACE(proputilsLog, "Performing comparison operation " << dev_os_version << " " << action << " " << osVersion << " RESULT:" << matchOsVersion);
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
    dataType.id = prop->getID().c_str();
    
    if (prop->getValue() != NULL) {
        std::string value(prop->getValue());
        dataType.value <<= value;
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const SimpleSequencePropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = prop->getID().c_str();
    dataType.value = ossie::strings_to_any(prop->getValues(), CORBA::tk_string, NULL);
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructPropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = prop->getID().c_str();
    
    CF::Properties structval_;
    StructPropertyRef::ValuesMap::const_iterator i;
    for (i = prop->getValue().begin(); i != prop->getValue().end(); ++i) {
        CF::DataType dt;
        dt = convertPropertyRefToDataType((*i).second);
        RH_TRACE(proputilsLog, "setting struct item " << (*i).first);
	ossie::corba::push_back(structval_, dt);
    }
    dataType.value <<= structval_;
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructSequencePropertyRef* prop) {
    CF::DataType dataType;
    dataType.id = prop->getID().c_str();
    
    const StructSequencePropertyRef::ValuesList propValues = prop->getValues();
    CORBA::AnySeq values;
    values.length(propValues.size());
    for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
        CF::DataType tmp_struct;
        tmp_struct.id = CORBA::string_dup("");
        CF::Properties structval_;
        ossie::ComponentPropertyMap::const_iterator i;
        for (i = propValues[ii].begin(); i != propValues[ii].end(); ++i) {
            CF::DataType dt;
            dt = convertPropertyRefToDataType((*i).second);
            RH_TRACE(proputilsLog, "setting struct item " << (*i).first);
            ossie::corba::push_back(structval_, dt);
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
    CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(property->getType()));
    CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(property->getType()));
    return ossie::strings_to_any(ossie::any_to_strings(value), kind, type);
}

CORBA::Any ossie::convertAnyToPropertyType(const CORBA::Any& value, const StructProperty* property)
{
    CORBA::Any result;
    const CF::Properties *depProps;
    if (value >>= depProps) {
        CF::Properties tmp_props;
        for (unsigned int index = 0; index < depProps->length(); ++index) {
            const CF::DataType& item = (*depProps)[index];
            const std::string propid(item.id);
            const ossie::Property* field = property->getField(propid);
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
    CORBA::Any result;
    const CORBA::AnySeq* seq;
    if (value >>= seq) {
        CORBA::AnySeq tmp_seq;
        const StructProperty& structdef = property->getStruct();
        for (CORBA::ULong index = 0; index < seq->length(); ++index) {
            ossie::corba::push_back(tmp_seq, convertAnyToPropertyType((*seq)[index], &structdef));
        }
        result <<= tmp_seq;
    }
    return result;
}



void ossie::convertComponentProperties( const ossie::ComponentPropertyList &cp_props,
                                 CF::Properties &cf_props ) 
{
  ossie::ComponentPropertyList::const_iterator piter = cp_props.begin();
  for ( ; piter != cp_props.end(); piter++ ) {
    cf_props.length(cf_props.length()+1);
    CF::DataType dt = ossie::convertPropertyRefToDataType( *piter );
    cf_props[cf_props.length()-1] = dt;
  }
}

void ossie::convertComponentProperties( const ossie::ComponentPropertyList &cp_props,
                                        redhawk::PropertyMap &cf_props )
{
  ossie::ComponentPropertyList::const_iterator piter = cp_props.begin();
  for ( ; piter != cp_props.end(); piter++ ) {
    CF::DataType dt = ossie::convertPropertyRefToDataType( *piter );
    cf_props.push_back(dt);
  }
}

    
std::string ossie::retrieveParserErrorLineNumber(std::string message) {
    size_t begin_n_line = message.find_first_of(':');
    size_t end_n_line = std::string::npos;
    if (begin_n_line != std::string::npos) {
        end_n_line = message.find_first_of(':', begin_n_line+1);
    }
    std::string ret_message;
    if (end_n_line != std::string::npos) {
        ret_message = "Error occurred on or around line ";
        ret_message += message.substr(begin_n_line+1,end_n_line-begin_n_line-1);
        ret_message += ".";
    }
    return ret_message;
}

bool ossie::structContainsMixedNilValues(const CF::Properties& properties)
{
    const redhawk::PropertyMap& fields = redhawk::PropertyMap::cast(properties);
    bool nils = false;
    bool values = false;
    for (redhawk::PropertyMap::const_iterator prop = fields.begin(); prop != fields.end(); ++prop) {
        if (prop->getValue().isNil()) {
            nils = true;
        } else {
            values = true;
        }
        if (nils && values) {
            return true;
        }
    }
    return false;
}

CF::Properties ossie::getPartialStructs(const CF::Properties& properties)
{
    CF::Properties partials;
    const redhawk::PropertyMap& configProps = redhawk::PropertyMap::cast(properties);
    for (redhawk::PropertyMap::const_iterator prop = configProps.begin(); prop != configProps.end(); ++prop) {
        redhawk::Value::Type type = prop->getValue().getType();
        if (type == redhawk::Value::TYPE_PROPERTIES) {
            // Property is a struct
            if (ossie::structContainsMixedNilValues(prop->getValue().asProperties())) {
                ossie::corba::push_back(partials, *prop);
            }
        } else if (type == redhawk::Value::TYPE_VALUE_SEQUENCE) {
            // Property is a struct sequence
            const redhawk::ValueSequence& sequence = prop->getValue().asSequence();
            for (redhawk::ValueSequence::const_iterator item = sequence.begin(); item != sequence.end(); ++item) {
                if (item->getType() == redhawk::Value::TYPE_PROPERTIES) {
                    if (ossie::structContainsMixedNilValues(item->asProperties())) {
                        ossie::corba::push_back(partials, *prop);
                        continue;
                    }
                }
            }
        }
    }
    return partials;
}

CF::Properties ossie::getAffinityOptions(const ComponentInstantiation::AffinityProperties& affinityProps)
{
    // Store parsed affinity properties as a static singleton, protecting the
    // load with a mutex; if the definitions are not set
    static boost::scoped_ptr<ossie::Properties> definitions;
    static boost::mutex mutex;
    if (!definitions) {
        boost::mutex::scoped_lock lock(mutex);
        if (!definitions) {
            // Set the singleton first, under the assumption that if load fails
            // once it will always fail, so that it doesn't re-try every time
            definitions.reset(new ossie::Properties());
            try {
                std::stringstream xml(redhawk::affinity::get_property_definitions());
                RH_TRACE(proputilsLog, "Loading affinity definitions: " << xml.str());
                definitions->load(xml);
            } catch (...) {
                RH_WARN(proputilsLog, "Error loading affinity defintions from library");
            }
        }
    }

    CF::Properties options;
    BOOST_FOREACH(const ossie::ComponentProperty& propref, affinityProps) {
        const Property* prop = definitions->getProperty(propref.getID());
        if (prop) {
            CF::DataType dt = overridePropertyValue(prop, &propref);
            ossie::corba::push_back(options, dt);
        } else {
            RH_WARN(proputilsLog, "Ignoring unknown affinity property " << propref.getID());
        } 
    }
    return options;
}
