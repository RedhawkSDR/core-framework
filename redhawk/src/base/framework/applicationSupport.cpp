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
#include <iostream>
#include <vector>
#include <stdlib.h>

#include "ossie/debug.h"
#include "ossie/SoftPkg.h"
#include <ossie/ossieSupport.h>
#include <ossie/prop_helpers.h>
#include <ossie/Properties.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

using namespace ossie;

static void addProperty(const CF::DataType& dt, CF::Properties& prop)
{
    for (unsigned int ii = 0; ii < prop.length(); ++ii) {
        if (strcmp(dt.id, prop[ii].id) == 0) {
            // Overwrite existing value
            prop[ii].value = dt.value;
            return;
        }
    }

    // New id, add property at end
    unsigned int index = prop.length();
    prop.length(index + 1);
    prop[index] = dt;
}

////////////////////////////////////////////////////
/**
 * UsesDeviceInfo member function definitions
 */
UsesDeviceInfo::UsesDeviceInfo(const std::string& _id, const std::string& _type, const std::vector<ossie::SPD::PropertyRef>& _properties) :
    id(_id),
    type(_type),
    properties(_properties)
{

}

UsesDeviceInfo::~UsesDeviceInfo()
{
}

const std::string& UsesDeviceInfo::getId() const
{
    return id;
}

const std::string& UsesDeviceInfo::getType() const
{
    return type;
}

const std::vector<ossie::SPD::PropertyRef>& UsesDeviceInfo::getProperties() const
{
    return properties;
}

const std::string& UsesDeviceInfo::getAssignedDeviceId() const
{
    return assignedDeviceId;
}

void UsesDeviceInfo::setAssignedDeviceId(const std::string& deviceId)
{
    assignedDeviceId = deviceId;
}

////////////////////////////////////////////////////
/**
 * ComponentImplementationInfo member function definitions
 */
PREPARE_LOGGING(ComponentImplementationInfo);

ComponentImplementationInfo::ComponentImplementationInfo() :
    usesDevices()
{
}

ComponentImplementationInfo::~ComponentImplementationInfo()
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        delete usesDevices[ii];
    }
    usesDevices.clear();
}

bool ComponentImplementationInfo::checkUsesDevices(ossie::Properties& _prf, CF::Properties& allocProps, unsigned int usesDevIdx, const CF::Properties& configureProperties) const
{
    const std::vector<const Property*>& allocationProps = _prf.getAllocationProperties();

    LOG_DEBUG(ComponentImplementationInfo, "Attempting to match allocation properties");
    // By definition if we have dependencies and the device
    // has no properties, then the match fails
    const std::vector<ossie::SPD::PropertyRef>& props = usesDevices[usesDevIdx]->getProperties();
    if ((props.size() > 0) && (allocationProps.size() == 0)) {
        return false;
    }

    bool result = true;
    for (unsigned int i = 0; i < props.size(); i++) {
        ComponentProperty* dependencyProp = props[i].property;
        std::string propid = dependencyProp->getID();

        LOG_DEBUG(ComponentImplementationInfo, "Trying to match for property id " << propid)
        const Property* matchingProp = 0;
        for (unsigned int j = 0; j < allocationProps.size(); j++) {
            if (strcmp(allocationProps[j]->getID(), propid.c_str()) == 0) {
                matchingProp = allocationProps[j];
            }
        }

        if (matchingProp) {
            LOG_DEBUG(ComponentImplementationInfo, " It's a matching prop");
            CF::DataType depProp = overridePropertyValue(matchingProp, dependencyProp);
            if (matchingProp->isExternal()) {
                const SimpleProperty* simpleMatchingProp = dynamic_cast<const SimpleProperty*>(matchingProp);
                if (simpleMatchingProp == NULL) {
                    CF::DataType depProp;
                    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(dependencyProp);
                    if (structref) {
                        // this is necessary to support the __MATH__ function in structs in dependencies
                        depProp = ossie::overridePropertyValue(matchingProp, dependencyProp, configureProperties);
                    } else {
                        depProp = ossie::overridePropertyValue(matchingProp, dependencyProp);
                    }
                    addProperty(depProp, allocProps);
                } else {
                    // The depProp is the  property that is defined in the components SPD
                    const SimplePropertyRef* simpleDependency = dynamic_cast<const SimplePropertyRef*>(dependencyProp);
                    if (!simpleDependency) {
                        std::ostringstream eout;
                        eout << " Property reference " << dependencyProp->getID() << " does not match type of device property";
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                    CF::DataType depProp;
                    const char* propvalue = simpleDependency->getValue();
                    depProp.id = CORBA::string_dup(dependencyProp->getID());
                    LOG_TRACE(ComponentImplementationInfo, " Matched! " << depProp.id << " value " << propvalue)
                    CORBA::Any capacityDep = ossie::string_to_any(propvalue, getTypeKind(simpleMatchingProp->getType()));

                    if (strncmp(propvalue, "__MATH__", 8) != 0) {
                        depProp.value = capacityDep;
                    } else {
                        LOG_TRACE(ComponentImplementationInfo, "Invoking custom OSSIE dynamic allocation property support")
                        // Turn propvalue into a string for easy parsing
                        std::string mathStatement = std::string(propvalue).substr(8);
                        if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                            // TODO - implement a more relaxed parser
                            mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                            mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                            std::vector<std::string> args;
                            while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                                args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                                LOG_TRACE(ComponentImplementationInfo, "ARG " << args.back())
                                mathStatement.erase(0, mathStatement.find(',') + 1);
                            }
                            args.push_back(mathStatement);
                            LOG_TRACE(ComponentImplementationInfo, "ARG " << args.back())
    
                            if (args.size() != 3) {
                                std::ostringstream eout;
                                eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                                throw ossie::PropertyMatchingError(eout.str());
                            }
    
                            LOG_TRACE(ComponentImplementationInfo, "__MATH__ " << args[0] << " " << args[1] << " " << args[2])
    
                            double operand;
                            operand = strtod(args[0].c_str(), NULL);
    
                            // See if there is a property in the component
                            LOG_TRACE(ComponentImplementationInfo, "Attempting to find matching property for " << args[1])
                            const CF::DataType* matchingCompProp = 0;
                            for (unsigned int j = 0; j < configureProperties.length(); j++) {
                                if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                                    LOG_TRACE(ComponentImplementationInfo, "Matched property for " << args[1])
                                    matchingCompProp = &configureProperties[j];
                                }
                            }
    
                            if (matchingCompProp == 0) {
                                std::ostringstream eout;
                                eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                                throw ossie::PropertyMatchingError(eout.str());
                            }
    
                            //std::string stringvalue = ossie::any_to_string(matchingCompProp->value);
                            //LOG_DEBUG(ImplementationInfo, "Converting " << stringvalue << " " << matchingCompProp->getTypeKind())
                            //compProp = ossie::string_to_any(stringvalue, matchingCompProp->getTypeKind());
                            std::string math = args[2];
                            CORBA::Any compValue = matchingCompProp->value;
                            LOG_TRACE(ComponentImplementationInfo, "Component configuration value " << ossie::any_to_string(compValue))
                            depProp.value = ossie::calculateDynamicProp(operand, compValue, math, getTypeKind(simpleMatchingProp->getType()));
                        } else {
                            std::ostringstream eout;
                            eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                            throw ossie::PropertyMatchingError(eout.str());
                        }
                    }
                    LOG_TRACE(ComponentImplementationInfo, "Adding dependency " << depProp.id << " to be " << ossie::any_to_string(depProp.value))
                    addProperty(depProp, allocProps);
                }
            } else {
                const SimpleProperty* simpleProp = dynamic_cast<const SimpleProperty*>(matchingProp);
                if (!simpleProp) {
                    LOG_ERROR(ComponentImplementationInfo, "Invalid action '" << matchingProp->getAction()
                              << "' for non-simple property " << propid);
                    allocProps.length(0);
                    return false;
                }
                std::string action = simpleProp->getAction();

                LOG_TRACE(ComponentImplementationInfo, propid << "=" << simpleProp->getID() << " value "
                          << simpleProp->getValue() << " " << action << " "
                          << ossie::any_to_string(depProp.value));
                CF::DataType allocProp = ossie::convertPropertyToDataType(simpleProp);
                LOG_TRACE(ComponentImplementationInfo, "Result prior to comparison " << result)
                // Per section D.4.1.1.7 the allocation property is on the left side of the action
                // and the dependency value is on the right side of the action
                result &= ossie::compare_anys(allocProp.value, depProp.value, action);
            }
        } else {
            LOG_TRACE(ComponentImplementationInfo, " It's not a matching prop")
            allocProps.length(0);
            return false;
        }

        if (!result) {
            break;
        }
    }
    return result;
}

void ComponentImplementationInfo::addUsesDevice(UsesDeviceInfo* _usesDevice)
{
    usesDevices.push_back(_usesDevice);
}

const std::vector<UsesDeviceInfo*>& ComponentImplementationInfo::getUsesDevices() const
{
    return usesDevices;
}


////////////////////////////////////////////////////
/**
 * ImplementationInfo member function definitions
 */
PREPARE_LOGGING(ImplementationInfo);

ImplementationInfo::ImplementationInfo(const SPD::Implementation& spdImpl) :
    id(spdImpl.getID()),
    codeType(),
    localFileName(),
    entryPoint(),
    processorDeps(spdImpl.getProcessors()),
    osDeps(spdImpl.getOsDeps()),
    dependencyProperties()
{
    setLocalFileName(spdImpl.getCodeFile());
    setEntryPoint(spdImpl.getEntryPoint());
    setCodeType(spdImpl.getCodeType());
    setStackSize(spdImpl.code.stacksize.get());
    setPriority(spdImpl.code.priority.get());

    // Create local copies for all of the usesdevice entries for this implementation.
    LOG_TRACE(ImplementationInfo, "Loading component implementation uses device")
    const std::vector<SPD::UsesDevice>& spdUsesDevices = spdImpl.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const SPD::UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                    spdUsesDev.getDependencies());
        addUsesDevice(usesDevice);
    }

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation property dependencies")
    const std::vector<ossie::SPD::PropertyRef>& dependencies = spdImpl.getDependencies();
    std::vector<ossie::SPD::PropertyRef>::const_iterator ii;
    for (ii = dependencies.begin(); ii != dependencies.end(); ++ii) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation property dependency '" << *ii);
        addDependencyProperty(*ii);
    }

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependencies")
    const std::vector<ossie::SPD::SoftPkgRef>& softpkgDependencies = spdImpl.getSoftPkgDependencies();
    std::vector<ossie::SPD::SoftPkgRef>::const_iterator jj;
    for (jj = softpkgDependencies.begin(); jj != softpkgDependencies.end(); ++jj) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependency '" << *jj);
        addSoftPkgDependency(*jj);
    }
}

ImplementationInfo::~ImplementationInfo()
{
}

const std::string& ImplementationInfo::getId() const
{
    return id;
}

const std::vector<std::string>& ImplementationInfo::getProcessorDeps() const
{
    return processorDeps;
}

const std::vector<SPD::SoftPkgRef>& ImplementationInfo::getSoftPkgDependency() const
{
    return softPkgDependencies;
}

const std::vector<ossie::SPD::NameVersionPair>& ImplementationInfo::getOsDeps() const
{
    return osDeps;
}

CF::LoadableDevice::LoadType ImplementationInfo::getCodeType() const
{
    return codeType;
}

const std::string& ImplementationInfo::getLocalFileName() const
{
    return localFileName;
}

const std::string& ImplementationInfo::getEntryPoint() const
{
    return entryPoint;
}

const CORBA::ULong ImplementationInfo::getStackSize() const
{
    return stackSize;
}

const CORBA::ULong ImplementationInfo::getPriority() const
{
    return priority;
}

const bool ImplementationInfo::hasStackSize() const
{
    return _hasStackSize;
}

const bool ImplementationInfo::hasPriority() const
{
    return _hasPriority;
}

const UsesDeviceInfo* ImplementationInfo::getUsesDeviceById(const std::string& id) const
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        if (usesDevices[ii]->getId() == id) {
            return usesDevices[ii];
        }
    }

    return 0;
}

const std::vector<SPD::PropertyRef>& ImplementationInfo::getDependencyProperties() const
{
    return dependencyProperties;
}

void ImplementationInfo::setCodeType(const char* _type)
{
    std::string type(_type);
    if (type == "KernelModule") {
        codeType = CF::LoadableDevice::KERNEL_MODULE;
    } else if (type == "SharedLibrary") {
        codeType = CF::LoadableDevice::SHARED_LIBRARY;
    } else if (type == "Executable") {
        codeType = CF::LoadableDevice::EXECUTABLE;
    } else if (type == "Driver") {
        codeType = CF::LoadableDevice::DRIVER;
    } else {
        LOG_WARN(ImplementationInfo, "Bad code type " << type);
    }
}

void ImplementationInfo::setLocalFileName(const char* fileName)
{
    if (fileName) {
        localFileName = fileName;
    }
}

void ImplementationInfo::setEntryPoint(const char* _entryPoint)
{
    if (_entryPoint) {
        entryPoint = _entryPoint;
    }
}

void ImplementationInfo::setStackSize(const unsigned long long* _stackSize)
{
    _hasStackSize = false;
    if (_stackSize) {
        stackSize = *_stackSize;
        _hasStackSize = true;
    }
}

void ImplementationInfo::setPriority(const unsigned long long* _priority)
{
    _hasPriority = false;
    if (_priority) {
        priority = *_priority;
        _hasPriority = true;
    }
}

void ImplementationInfo::addDependencyProperty(const SPD::PropertyRef& property)
{
    dependencyProperties.push_back(property);
}

void ImplementationInfo::addSoftPkgDependency(const SPD::SoftPkgRef& softpkg)
{
    softPkgDependencies.push_back(softpkg);
}

bool ImplementationInfo::checkProcessorAndOs(const Properties& _prf) const
{
    bool matchProcessor = checkProcessor(processorDeps, _prf.getAllocationProperties());
    bool matchOs = checkOs(osDeps, _prf.getAllocationProperties());

    if (!matchProcessor) {
        LOG_DEBUG(ImplementationInfo, "Failed to match component processor to device allocation properties");
    }
    if (!matchOs) {
        LOG_DEBUG(ImplementationInfo, "Failed to match component os to device allocation properties");
    }
    return matchProcessor && matchOs;
}

bool ImplementationInfo::checkMatchingDependencies(const Properties& _prf, const std::string& _softwareProfile, const CF::DeviceManager_var& _devMgr) const
{
    const std::vector <const Property*>& allocationProps = _prf.getAllocationProperties();
    ossie::DeviceManagerConfiguration dcdParser;
    std::string deviceManagerProfile = "";

    LOG_DEBUG(ImplementationInfo, "Attempting to match allocation properties")
    // By definition if we have dependencies and the device
    // has no properties, then the match fails
    
    
    if ((dependencyProperties.size() > 0) && (allocationProps.size() == 0)) {
        LOG_TRACE(ImplementationInfo, "Have dependency properties, but no allocation properties");
        return false;
    }

    bool result = true;
    for (unsigned int i = 0; i < dependencyProperties.size(); i++) {
        std::string propid = dependencyProperties[i].property->getID();

        const SimpleProperty* matchingProp = 0;
        LOG_TRACE(ImplementationInfo, "Searching for allocation property that matches id " << propid);
        for (unsigned int j = 0; j < allocationProps.size(); j++) {
            if (strcmp(allocationProps[j]->getID(), propid.c_str()) == 0) {
                LOG_TRACE(ImplementationInfo, "Found for allocation property that matches id " << propid);
                if (allocationProps[j]->isExternal()) {
                    LOG_TRACE(ImplementationInfo, "Skipping property with action 'external'");
                } else if (dynamic_cast<const SimpleProperty*>(allocationProps[j]) != NULL) {
                    matchingProp = dynamic_cast<const SimpleProperty*>(allocationProps[j]);
                } else {
                    LOG_ERROR(ImplementationInfo, "Ignoring matching property ; only simple properties can be used for matching")
                }
                break;
            }
        }

        if (matchingProp) {
            if (matchingProp->isNone()) {
                LOG_WARN(ImplementationInfo, "Matching for property '" << propid << "' failed; "
                         << "device PRF file provided property with no value");
                result = false;
            } else {
                LOG_TRACE(ImplementationInfo, "Performing match for " << propid);
                std::string action = matchingProp->getAction();

                const SimplePropertyRef* dependency = dynamic_cast<const SimplePropertyRef*>(dependencyProperties[i].property);
                if (!dependency) {
                    LOG_ERROR(ImplementationInfo, "Matching propertyref is not a simple property; ignoring");
                    continue;
                }

                const char* propvalue = dependency->getValue();
                const char* matchingpropvalue;
                if (deviceManagerProfile == "") {
                    deviceManagerProfile = CORBA::string_dup(_devMgr->deviceConfigurationProfile());
                    File_stream _dcd(_devMgr->fileSys(), deviceManagerProfile.c_str());
                    dcdParser.load(_dcd);
                    _dcd.close();
                    std::vector<ComponentFile> componentFiles = dcdParser.getComponentFiles();
                    std::vector<ComponentFile>::iterator p = componentFiles.begin();
                    std::string placement_id = "";
                    while (p != componentFiles.end()) {
                        if ((*p).filename == _softwareProfile) {
                            placement_id = (*p).id;
                            break;
                        }
                        p++;
                    }
                    std::vector<ComponentPlacement> componentPlacements = dcdParser.getComponentPlacements();
                    std::vector<ComponentPlacement>::iterator pl = componentPlacements.begin();
                    std::vector<ComponentProperty*> properties;
                    properties.resize(0);
                    while (pl != componentPlacements.end()) {
                        if ((*pl)._componentFileRef == placement_id) {
                            properties = (*pl).instantiations[0].properties;
                            break;
                        }
                        pl++;
                    }
                    std::vector<ComponentProperty*>::iterator p_prop = properties.begin();
                    while (p_prop != properties.end()) {
                        if ((*p_prop)->_id == propid) {
                            const SimplePropertyRef* overload = dynamic_cast<const SimplePropertyRef*>(*p_prop);
                            matchingpropvalue = overload->getValue();
                            break;
                        }
                        p_prop++;
                    }
                    if (p_prop == properties.end()) {
                        matchingpropvalue = matchingProp->getValue();
                    }
                } else {
                    matchingpropvalue = matchingProp->getValue();
                }

                LOG_TRACE(ImplementationInfo, "Match operation " << matchingpropvalue << " " << action << " " << propvalue);
                CORBA::Any allocProp = ossie::string_to_any(matchingpropvalue, getTypeKind(matchingProp->getType()));
                CORBA::Any depProp = ossie::string_to_any(propvalue, getTypeKind(matchingProp->getType()));
                LOG_TRACE(ImplementationInfo, "Result prior to comparison " << result);
                // Per section D.4.1.1.7 the allocation property is on the left side of the action
                // and the dependency value is on the right side of the action
                result &= ossie::compare_anys(allocProp, depProp, action);
                LOG_TRACE(ImplementationInfo, "Result after to comparison " << result);
            }
        }

        if (!result) {
            break;
        }
    }
    LOG_DEBUG(ImplementationInfo, "Done matching allocation properties")
    return result;
}

CF::Properties ImplementationInfo::getAllocationProperties(const Properties& _prf, const CF::Properties& configureProperties) const
    throw (ossie::PropertyMatchingError)
{
    CF::Properties allocProps;
    const std::vector<const Property*>& allocationProps = _prf.getAllocationProperties();

    LOG_TRACE(ImplementationInfo, "Attempting to grab external allocation properties, component has " << dependencyProperties.size() << " dependencies")
    // By definition if we have dependencies and the device
    // has no properties, then the match fails
    if ((dependencyProperties.size() > 0) && (allocationProps.size() == 0)) {
        std::ostringstream eout;
        eout << "device has no allocation properties for the component to match";
        throw ossie::PropertyMatchingError(eout.str());
    }

    for (unsigned int i = 0; i < dependencyProperties.size(); i++) {
        ComponentProperty* dependency = dependencyProperties[i].property;
        std::string propid = dependency->getID();

        LOG_TRACE(ImplementationInfo, "Trying to match for property id " << propid)
        const Property* matchingProp = 0;
        for (unsigned int j = 0; j < allocationProps.size(); j++) {
            if (strcmp(allocationProps[j]->getID(), propid.c_str()) == 0) {
                matchingProp = allocationProps[j];
            }
        }

        if (!matchingProp) {
            std::ostringstream eout;
            eout << " failing dependency; failed to find matching property in device for dependency; id = " << propid;
            throw ossie::PropertyMatchingError(eout.str());
        } else if (matchingProp->isExternal()) {
            const SimpleProperty* simpleMatchingProp = dynamic_cast<const SimpleProperty*>(matchingProp);
            // The matchingProp is the property that is defined in the devices PRF
            if (simpleMatchingProp == NULL) {
                CF::DataType depProp;
                const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(dependency);
                if (structref) {
                    // this is necessary to support the __MATH__ function in structs in dependencies
                    depProp = ossie::overridePropertyValue(matchingProp, dependency, configureProperties);
                } else {
                    depProp = ossie::overridePropertyValue(matchingProp, dependency);
                }
                addProperty(depProp, allocProps);
            } else {
                // The depProp is the  property that is defined in the components SPD
                const SimplePropertyRef* simpleDependency = dynamic_cast<const SimplePropertyRef*>(dependency);
                if (!simpleDependency) {
                    std::ostringstream eout;
                    eout << " Property reference " << dependency->getID() << " does not match type of device property";
                    throw ossie::PropertyMatchingError(eout.str());
                }

                CF::DataType depProp;
                const char* propvalue = simpleDependency->getValue();
                depProp.id = CORBA::string_dup(dependency->getID());

                LOG_TRACE(ImplementationInfo, " Matched! " << depProp.id << " value " << propvalue)


                CORBA::Any capacityDep = ossie::string_to_any(propvalue, getTypeKind(simpleMatchingProp->getType()));
                if (strncmp(propvalue, "__MATH__", 8) != 0) {
                    depProp.value = capacityDep;
                } else {
                    LOG_TRACE(ImplementationInfo, "Invoking custom OSSIE dynamic allocation property support")
                    // Turn propvalue into a string for easy parsing
                    std::string mathStatement = std::string(propvalue).substr(8);
                    if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                        // TODO - implement a more relaxed parser
                        mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                        mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                        std::vector<std::string> args;
                        while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                            args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                            LOG_TRACE(ImplementationInfo, "ARG " << args.back())
                            mathStatement.erase(0, mathStatement.find(',') + 1);
                        }
                        args.push_back(mathStatement);
                        LOG_TRACE(ImplementationInfo, "ARG " << args.back())

                        if (args.size() != 3) {
                            std::ostringstream eout;
                            eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                            throw ossie::PropertyMatchingError(eout.str());
                        }

                        LOG_TRACE(ImplementationInfo, "__MATH__ " << args[0] << " " << args[1] << " " << args[2])

                        double operand;
                        operand = strtod(args[0].c_str(), NULL);

                        // See if there is a property in the component
                        LOG_TRACE(ImplementationInfo, "Attempting to find matching property for " << args[1])
                        const CF::DataType* matchingCompProp = 0;
                        for (unsigned int j = 0; j < configureProperties.length(); j++) {
                            if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                                LOG_TRACE(ImplementationInfo, "Matched property for " << args[1])
                                matchingCompProp = &configureProperties[j];
                            }
                        }

                        if (matchingCompProp == 0) {
                            std::ostringstream eout;
                            eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                            throw ossie::PropertyMatchingError(eout.str());
                        }

                        //std::string stringvalue = ossie::any_to_string(matchingCompProp->value);
                        //LOG_DEBUG(ImplementationInfo, "Converting " << stringvalue << " " << matchingCompProp->getTypeKind())
                        //compProp = ossie::string_to_any(stringvalue, matchingCompProp->getTypeKind());
                        std::string math = args[2];
                        CORBA::Any compValue = matchingCompProp->value;
                        LOG_TRACE(ImplementationInfo, "Component configuration value " << ossie::any_to_string(compValue))
                        depProp.value = ossie::calculateDynamicProp(operand, compValue, math, getTypeKind(simpleMatchingProp->getType()));
                    } else {
                        std::ostringstream eout;
                        eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                }
                LOG_TRACE(ImplementationInfo, "Adding dependency " << depProp.id << " to be " << ossie::any_to_string(depProp.value))
                addProperty(depProp, allocProps);
            }
        }
    }
    return allocProps;
}


////////////////////////////////////////////////////
/**
 * ComponentInfo member function definitions
 */
PREPARE_LOGGING(ComponentInfo);

ComponentInfo* ComponentInfo::buildComponentInfoFromSPDFile(CF::FileManager_ptr fileMgr, const char* spdFileName)
{
    LOG_TRACE(ComponentInfo, "Building component info from file " << spdFileName);

    ossie::ComponentInfo* newComponent = new ossie::ComponentInfo();
    CF::File_var _spd;

    try {
        File_stream _spd(fileMgr, spdFileName);
        newComponent->spd.load(_spd, spdFileName);
        _spd.close();
    } catch (ossie::parser_error& e) {
        LOG_ERROR(ComponentInfo, "building component info problem; error parsing spd; " << e.what());
        delete newComponent;
        return 0;
    } catch( ... ) {
        LOG_ERROR(ComponentInfo, "building component info problem; unknown error parsing spd;");
        delete newComponent;
        return 0;
    }

    if (newComponent->spd.getSCDFile() != 0) {
        try {
            File_stream _scd(fileMgr, newComponent->spd.getSCDFile());
            newComponent->scd.load(_scd);
            _scd.close();
        } catch (ossie::parser_error& e) {
            LOG_ERROR(ComponentInfo, "building component info problem; error parsing scd; " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "building component info problem; unknown error parsing scd;");
            delete newComponent;
            return 0;
        }
    }


    if (newComponent->spd.getPRFFile() != 0) {
        LOG_DEBUG(ComponentInfo, "Loading component properties from " << newComponent->spd.getPRFFile());
        try {
            File_stream _prf(fileMgr, newComponent->spd.getPRFFile());
            LOG_DEBUG(ComponentInfo, "Parsing component properties");
            newComponent->prf.load(_prf);
            LOG_TRACE(ComponentInfo, "Closing PRF file")
            _prf.close();
        } catch (ossie::parser_error& e) {
            LOG_ERROR(ComponentInfo, "building component info problem; error parsing prf; " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "building component info problem; unknown error parsing prf;");
            delete newComponent;
            return 0;
        }
    }

    newComponent->setName(newComponent->spd.getSoftPkgName());
    newComponent->setIsScaCompliant(newComponent->spd.isScaCompliant());

    // Extract implementation data from SPD file
    const std::vector <SPD::Implementation>& spd_i = newComponent->spd.getImplementations();

    // Assume only one implementation, use first available result [0]
    for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
        const SPD::Implementation& spdImpl = spd_i[implCount];
        LOG_TRACE(ComponentInfo, "Adding implementation " << spdImpl.getID());
        ImplementationInfo* newImpl = new ImplementationInfo(spdImpl);
        newComponent->addImplementation(newImpl);
    }

    // Create local copies for all of the usesdevice entries for this implementation.
    const std::vector<SPD::UsesDevice>& spdUsesDevices = newComponent->spd.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const SPD::UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                                                        spdUsesDev.getDependencies());
        newComponent->addUsesDevice(usesDevice);
    }

    // Extract Properties from the implementation-agnostic PRF file
    // once we match the component to a device we can grab the implementation
    // specific PRF file
    if (newComponent->spd.getPRFFile() != 0) {
        // Handle component properties
        LOG_TRACE(ComponentInfo, "Adding factory params")
        const std::vector<const Property*>& fprop = newComponent->prf.getFactoryParamProperties();
        for (unsigned int i = 0; i < fprop.size(); i++) {
            newComponent->addFactoryParameter(convertPropertyToDataType(fprop[i]));
        }

        LOG_TRACE(ComponentInfo, "Adding exec params")
        const std::vector<const Property*>& eprop = newComponent->prf.getExecParamProperties();
        for (unsigned int i = 0; i < eprop.size(); i++) {
            if (std::string(eprop[i]->getMode()) != "readonly") {
                LOG_TRACE(ComponentInfo, "Adding exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
                newComponent->addExecParameter(convertPropertyToDataType(eprop[i]));
            } else {
                LOG_TRACE(ComponentInfo, "Ignoring readonly exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
            }
        }

        // According to SCA 2.2.2 Appendix D 4.1.1.6 the only properties
        // that are to be sent to allocateCapacity are device
        // properties that are external.  Furthermore, the SPD list's its
        // needs, not in the PRF file, but in the SPD file <dependencies>
        // element
        // prop = prf->getMatchingProperties();
        //for (unsigned int i=0; i < prop->size(); i++) {
        //    newComponent->addAllocationCapacity((*prop)[i]->getDataType());
        //}

        const std::vector<const Property*>& prop = newComponent->prf.getConfigureProperties();
        for (unsigned int i = 0; i < prop.size(); i++) {
            if (!prop[i]->isReadOnly()) {
                LOG_TRACE(ComponentInfo, "Adding configure prop " << prop[i]->getID() << " " << prop[i]->getName() << " " << prop[i]->isReadOnly())
                newComponent->addConfigureProperty(convertPropertyToDataType(prop[i]));
            }
        }
    }

    LOG_TRACE(ComponentInfo, "Done building component info from file " << spdFileName);
    return newComponent;
}

ComponentInfo::ComponentInfo() :
    _isAssemblyController(false),
    _isScaCompliant(true),
    assignedDeviceId(),
    selectedImplementation(0)/*,
                                 entryPoint(0),
                                 processor(""),
                                 os(""),
                                 osVersion("")*/
{
}

ComponentInfo::~ComponentInfo ()
{
    for (std::vector<ImplementationInfo*>::iterator ii = implementations.begin(); ii != implementations.end(); ++ii) {
        delete *ii;
    }
}

void ComponentInfo::setName(const char* _name)
{
    name = _name;
}

void ComponentInfo::setIdentifier(const char* _identifier, std::string instance_id)
{
    identifier = _identifier;
    // Per the SCA spec, the identifier is the instantiation ID:waveform_name
    instantiationId = instance_id;
}

void ComponentInfo::setAssignedDeviceId(const char* _assignedDeviceId)
{
    assignedDeviceId = _assignedDeviceId;
}

void ComponentInfo::setSelectedImplementation(const ImplementationInfo* implementation)
{
    // TODO: Verify implementation matches an existing one.
    selectedImplementation = implementation;
}

const UsesDeviceInfo* ComponentInfo::getUsesDeviceById(const std::string& id) const
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        if (usesDevices[ii]->getId() == id) {
            return usesDevices[ii];
        }
    }

    if (selectedImplementation) {
        return selectedImplementation->getUsesDeviceById(id);
    }

    return 0;
}

void ComponentInfo::setImplPRFFile(const char* _PRFFile)
{
    implPRF = _PRFFile;
}

void ComponentInfo::setSpdFileName(const char* spdFileName)
{
    SpdFileName = spdFileName;
}

void ComponentInfo::addImplementation(ImplementationInfo* impl)
{
    implementations.push_back(impl);
}

void ComponentInfo::setNamingService(const bool _isNamingService)
{
    isNamingService = _isNamingService;
}

void ComponentInfo::setNamingServiceName(const char* _namingServiceName)
{
    namingServiceName = _namingServiceName;
}

void ComponentInfo::setUsageName(const char* _usageName)
{
    if (_usageName != 0) {
        usageName = _usageName;
    }
}

void ComponentInfo::setIsAssemblyController(bool _isAssemblyController)
{
    this->_isAssemblyController = _isAssemblyController;
}

void ComponentInfo::setIsScaCompliant(bool _isScaCompliant)
{
    this->_isScaCompliant = _isScaCompliant;
}

void ComponentInfo::addFactoryParameter(CF::DataType dt)
{
    addProperty(dt, factoryParameters);
}

void ComponentInfo::addExecParameter(CF::DataType dt)
{
    addProperty(dt, execParameters);
}

void ComponentInfo::addConfigureProperty(CF::DataType dt)
{
    addProperty(dt, configureProperties);
}

void ComponentInfo::overrideProperty(const ossie::ComponentProperty* propref) {
    std::string propId = propref->getID();
    LOG_TRACE(ComponentInfo, "Instantiation property id = " << propId)
    const Property* prop = prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ComponentInfo, "ignoring attempt to override property " << propId << " that does not exist in component")
        return;
    }

    CF::DataType dt = overridePropertyValue(prop, propref);
    overrideProperty(dt.id, dt.value);
}

void ComponentInfo::overrideSimpleProperty(const char* id, const std::string value)
{
    const Property* prop = prf.getProperty(id);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ComponentInfo, "ignoring attempt to override property " << id << " that does not exist in component")
        return;
    }

    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(prop);
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(simple->getType()));
        CORBA::Any val = ossie::string_to_any(value, kind);
        overrideProperty(id, val);
    } else {
        LOG_WARN(ComponentInfo, "attempt to override non-simple property with string value");
    }
}

void ComponentInfo::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = prf.getProperty(id);
    if (prop != NULL) {
        if (prop->isReadOnly()) {
            LOG_WARN(ComponentInfo, "ignoring attempt to override readonly property " << id);
            return;
        }
    }
    process_overrides(&configureProperties, id, value);
    process_overrides(&options, id, value);
    process_overrides(&factoryParameters, id, value);
    process_overrides(&execParameters, id, value);
}



void ComponentInfo::process_overrides(CF::Properties* props, const char* id, CORBA::Any value)
{
    LOG_DEBUG(ComponentInfo, "Attempting to override property " << id);
    for (unsigned int i = 0; i < (*props).length(); ++i ) {
        if (strcmp(id, (*props)[i].id) == 0) {
            LOG_DEBUG(ComponentInfo, "Overriding property " << id << " with value " << ossie::any_to_string(value));
            (*props)[i].value = value;
        }
    }
    return;
}

void ComponentInfo::setResourcePtr(CF::Resource_ptr _rsc)
{
    rsc = CF::Resource::_duplicate(_rsc);
}

const char* ComponentInfo::getName()
{
    return name.c_str();
}

const char* ComponentInfo::getInstantiationIdentifier()
{
    return instantiationId.c_str();
}

const char* ComponentInfo::getIdentifier()
{
    return identifier.c_str();
}

const char* ComponentInfo::getAssignedDeviceId()
{
    return assignedDeviceId.c_str();
}

const ImplementationInfo* ComponentInfo::getSelectedImplementation() const
{
    return selectedImplementation;
}

const std::vector<ImplementationInfo*>& ComponentInfo::getImplementations() const
{
    return implementations;
}

const char* ComponentInfo::getImplPRFFile()
{
    return implPRF.c_str();
}

const bool  ComponentInfo::getNamingService()
{
    return isNamingService;
}

const char* ComponentInfo::getUsageName()
{
    return usageName.c_str();
}

const char* ComponentInfo::getSpdFileName()
{
    return SpdFileName.c_str();
}

const char* ComponentInfo::getNamingServiceName()
{
    return namingServiceName.c_str();
}

const bool  ComponentInfo::isResource()
{
    return scd.isResource();
}

const bool  ComponentInfo::isConfigurable()
{
    return scd.isConfigurable();
}

const bool  ComponentInfo::isAssemblyController()
{
    return _isAssemblyController;
}

const bool  ComponentInfo::isScaCompliant()
{
    return _isScaCompliant;
}

bool ComponentInfo::isAssignedToDevice() const
{
    return (!assignedDeviceId.empty());
}

CF::Properties ComponentInfo::getNonNilConfigureProperties()
{
    return ossie::getNonNilConfigureProperties(configureProperties);
}

CF::Properties ComponentInfo::getConfigureProperties()
{
    return configureProperties;
}

CF::Properties ComponentInfo::getOptions()
{
    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    //  unfortunately this can't happen until an implementation has been chosen
    if (selectedImplementation) {
        if (selectedImplementation->hasStackSize()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("STACK_SIZE");  // 3.1.3.3.3.3.6
            options[options.length()-1].value <<= selectedImplementation->getStackSize();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
        if (selectedImplementation->hasPriority()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("PRIORITY");  // 3.1.3.3.3.3.7
            options[options.length()-1].value <<= selectedImplementation->getPriority();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
    }

    return options;
}

CF::Properties ComponentInfo::getExecParameters()
{
    return execParameters;
}

CF::Resource_ptr ComponentInfo::getResourcePtr()
{
    return CF::Resource::_duplicate(rsc);
}

