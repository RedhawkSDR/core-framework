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
#include <vector>
#include <stdlib.h>
#include <omniORB4/CORBA.h>

#include <ossie/debug.h>
#include <ossie/SoftPkg.h>
#include <ossie/Properties.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/prop_utils.h>
#include <ossie/CorbaUtils.h>
#include <ossie/affinity.h>
#include "applicationSupport.h"
#include "PersistenceStore.h"
#include "ossie/PropertyMap.h"


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
/*
 * ComponentInfo member function definitions
 */
PREPARE_CF_LOGGING(ComponentInfo);

ComponentInfo* ComponentInfo::buildComponentInfoFromSPDFile(const SoftPkg* softpkg,
                                                            const ComponentInstantiation* instantiation)
{
    LOG_TRACE(ComponentInfo, "Building component info from softpkg " << softpkg->getName());

    ossie::ComponentInfo* newComponent = new ossie::ComponentInfo(softpkg, instantiation);

    // Extract Properties from the implementation-agnostic PRF file
    // once we match the component to a device we can grab the implementation
    // specific PRF file
    if (softpkg->getProperties()) {
        // Handle component properties
        LOG_TRACE(ComponentInfo, "Adding factory params");
        Properties& prf = *softpkg->getProperties();
        const std::vector<const Property*>& fprop = prf.getFactoryParamProperties();
        for (unsigned int i = 0; i < fprop.size(); i++) {
            newComponent->addFactoryParameter(convertPropertyToDataType(fprop[i]));
        }

        LOG_TRACE(ComponentInfo, "Adding exec params")
        const std::vector<const Property*>& eprop = prf.getExecParamProperties();
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

        const std::vector<const Property*>& prop = prf.getConfigureProperties();
        for (unsigned int i = 0; i < prop.size(); i++) {
            if (!prop[i]->isReadOnly()) {
                LOG_TRACE(ComponentInfo, "Adding configure prop " << prop[i]->getID() << " " << prop[i]->getName() << " " << prop[i]->isReadOnly())
                newComponent->addConfigureProperty(convertPropertyToDataType(prop[i]));
            }
        }

        const std::vector<const Property*>& cprop = prf.getConstructProperties();
        for (unsigned int i = 0; i < cprop.size(); i++) {
          LOG_TRACE(ComponentInfo, "Adding construct prop " << cprop[i]->getID() << " " << cprop[i]->getName() << " " << cprop[i]->isReadOnly());
          bool isExec = false;
          std::string cprop_id(cprop[i]->getID());
          for (unsigned int ei = 0; ei < eprop.size(); ei++) {
              std::string eprop_id(eprop[ei]->getID());
              if (eprop_id == cprop_id) {
                  isExec = true;
                  break;
              }
          }
          if (isExec)
              continue;
          if (cprop[i]->isCommandLine()) {
            if (not cprop[i]->isNone()) {
                newComponent->addExecParameter(convertPropertyToDataType(cprop[i]));
            }
          } else {
            newComponent->addConstructProperty(convertPropertyToDataType(cprop[i]));
          }
        }

    }
    
    LOG_TRACE(ComponentInfo, "Done building component info from soft package " << softpkg->getName());
    return newComponent;
}

ComponentInfo::ComponentInfo(const SoftPkg* softpkg, const ComponentInstantiation* instantiation) :
    spd(softpkg),
    instantiation(instantiation)
{
    // load common affinity property definitions 
    try {
      std::stringstream os(redhawk::affinity::get_property_definitions());
      LOG_TRACE(ComponentInfo, "affinity definitions: " << os.str());
      _affinity_prf.load(os);
    }
    catch(...){
      LOG_WARN(ComponentInfo, "Error loading affinity definitions from library." );
    }
}

ComponentInfo::~ComponentInfo ()
{
}

const std::string& ComponentInfo::getSpdFileName() const
{
    return spd->getSPDFile();
}

const std::string& ComponentInfo::getName() const
{
    return spd->getName();
}

const ComponentInstantiation* ComponentInfo::getInstantiation() const
{
    return instantiation;
}

void ComponentInfo::setUsageName(const char* _usageName)
{
    if (_usageName != 0) {
        usageName = _usageName;
    }
}

void ComponentInfo::setAffinity( const AffinityProperties &affinity_props )
{
  
  for (unsigned int i = 0; i < affinity_props.size(); ++i) {
    const ossie::ComponentProperty* propref = &(affinity_props[i]);
    std::string propId = propref->getID();
    LOG_DEBUG(ComponentInfo, "Affinity property id = " << propId);
    const Property* prop = _affinity_prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
      LOG_WARN(ComponentInfo, "ignoring attempt to override property " << propId << " that does not exist in component");
      continue;
    }

    // add property
    CF::DataType dt = overridePropertyValue(prop, propref);
    addProperty( dt, affinityOptions );
  }

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

void ComponentInfo::addConstructProperty(CF::DataType dt)
{
    addProperty(dt, ctorProperties);
}

void ComponentInfo::overrideProperty(const ossie::ComponentProperty* propref) {
    std::string propId = propref->getID();
    LOG_TRACE(ComponentInfo, "Instantiation property id = " << propId);
    const Property* prop = 0;
    if (spd->getProperties()) {
        prop = spd->getProperties()->getProperty(propId);
    }
    // Without a prop, we don't know how to convert the strings to the property any type
    if (!prop) {
        LOG_WARN(ComponentInfo, "ignoring attempt to override property " << propId << " that does not exist in component")
        return;
    }

    CF::DataType dt = overridePropertyValue(prop, propref);
    overrideProperty(dt.id, dt.value);
}

void ComponentInfo::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = 0;
    if (spd->getProperties()) {
        prop = spd->getProperties()->getProperty(id);
        if (prop && prop->isReadOnly()) {
            LOG_WARN(ComponentInfo, "ignoring attempt to override readonly property " << id);
            return;
        }
    }
    process_overrides(&ctorProperties, id, value);
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

const std::string& ComponentInfo::getInstantiationIdentifier() const
{
    return instantiation->getID();
}

bool ComponentInfo::isResource() const
{
    return spd->getDescriptor()->isResource();
}

bool ComponentInfo::isConfigurable() const
{
    return spd->getDescriptor()->isConfigurable();
}


bool ComponentInfo::isAssemblyController() const
{
    return instantiation->isAssemblyController();
}

bool ComponentInfo::isScaCompliant() const
{
    return spd->isScaCompliant();
}

bool ComponentInfo::checkStruct(const CF::Properties &props) const
{
    const redhawk::PropertyMap& tmpProps = redhawk::PropertyMap::cast(props);
    int state = 0; // 1 set, -1 nil
    for (redhawk::PropertyMap::const_iterator tmpP = tmpProps.begin(); tmpP != tmpProps.end(); tmpP++) {
        if (tmpProps[ossie::corba::returnString(tmpP->id)].isNil()) {
            if (state == 0) {
                state = -1;
            } else {
                if (state == 1) {
                    return true;
                }
            }
        } else {
            if (state == 0) {
                state = 1;
            } else {
                if (state == -1) {
                    return true;
                }
            }
        }
    }
    return false;
}

CF::Properties ComponentInfo::iteratePartialStruct(const CF::Properties &props) const
{
    CF::Properties retval;
    const redhawk::PropertyMap& configProps = redhawk::PropertyMap::cast(props);
    for (redhawk::PropertyMap::const_iterator cP = configProps.begin(); cP != configProps.end(); cP++) {
        const ossie::Property* prop = spd->getProperties()->getProperty(ossie::corba::returnString(cP->id));
        if (dynamic_cast<const ossie::StructProperty*>(prop)) {
            CF::Properties* tmp;
            if (!(cP->value >>= tmp))
                continue;
            if (this->checkStruct(*tmp))
                return *tmp;
        } else if (dynamic_cast<const ossie::StructSequenceProperty*>(prop)) {
            CORBA::AnySeq* anySeqPtr;
            if (!(cP->value >>= anySeqPtr)) {
                continue;
            }
            CORBA::AnySeq& anySeq = *anySeqPtr;
            for (CORBA::ULong ii = 0; ii < anySeq.length(); ++ii) {
                CF::Properties* tmp;
                if (!(anySeq[ii] >>= tmp))
                    continue;
                if (this->checkStruct(*tmp))
                    return *tmp;
            }
        } else {
            continue;
        }
    }
    return retval;
}

CF::Properties ComponentInfo::containsPartialStructConfig() const
{
    return this->iteratePartialStruct(configureProperties);
}

CF::Properties ComponentInfo::containsPartialStructConstruct() const
{
    return this->iteratePartialStruct(ctorProperties);
}

CF::Properties ComponentInfo::getNonNilConfigureProperties() const
{
    return ossie::getNonNilConfigureProperties(configureProperties);
}

CF::Properties ComponentInfo::getInitializeProperties() const
{
    return ossie::getNonNilProperties(ctorProperties);
}

CF::Properties ComponentInfo::getAffinityOptions() const
{
    return affinityOptions;
}

CF::Properties ComponentInfo::getConfigureProperties() const
{
    return configureProperties;
}


CF::Properties ComponentInfo::getConstructProperties() const
{
    return ctorProperties;
}


CF::Properties ComponentInfo::getOptions()
{
    return options;
}

CF::Properties ComponentInfo::getExecParameters()
{
    return execParameters;
}

CF::Properties ComponentInfo::getCommandLineParameters() const
{
    CF::Properties retval = execParameters;
    while (true) {
        unsigned int i;
        for (i=0; i<retval.length(); i++) {
            CORBA::TypeCode_var typecode = retval[i].value.type();
            if (typecode->kind() == CORBA::tk_null) {
                break;
            }
        }
        if (i == retval.length()) {
            break;
        }
        for (unsigned int j=i+1; j<retval.length(); j++) {
            retval[j-1] = retval[j];
        }
        retval.length(retval.length()-1);
    }
    return retval;
}
