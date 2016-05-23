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
 * UsesDeviceInfo member function definitions
 */
UsesDeviceInfo::UsesDeviceInfo(const std::string& _id, const std::string& _type, const std::vector<ossie::PropertyRef>& _properties) :
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

const std::vector<ossie::PropertyRef>& UsesDeviceInfo::getProperties() const
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

void UsesDeviceInfo::clearAssignedDeviceId()
{
    assignedDeviceId.clear();
}

////////////////////////////////////////////////////
/*
 * UsesDeviceContext member function definitions
 */
PREPARE_CF_LOGGING(UsesDeviceContext);

UsesDeviceContext::UsesDeviceContext() :
    usesDevices()
{
}

UsesDeviceContext::~UsesDeviceContext()
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        delete usesDevices[ii];
    }
    usesDevices.clear();
}

void UsesDeviceContext::addUsesDevice(UsesDeviceInfo* _usesDevice)
{
    usesDevices.push_back(_usesDevice);
}

const std::vector<UsesDeviceInfo*>& UsesDeviceContext::getUsesDevices() const
{
    return usesDevices;
}

const UsesDeviceInfo* UsesDeviceContext::getUsesDeviceById(const std::string& id) const
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        if (usesDevices[ii]->getId() == id) {
            return usesDevices[ii];
        }
    }

    return 0;
}


////////////////////////////////////////////////////
/*
 * ImplementationInfo member function definitions
 */
PREPARE_CF_LOGGING(ImplementationInfo);

ImplementationInfo::ImplementationInfo(const SPD::Implementation& spdImpl) :
    id(spdImpl.getID()),
    codeType(),
    localFileName(spdImpl.getCodeFile()),
    entryPoint(),
    processorDeps(spdImpl.getProcessors()),
    osDeps(spdImpl.getOsDeps()),
    dependencyProperties()
{
    setEntryPoint(spdImpl.getEntryPoint());
    setCodeType(spdImpl.getCodeType());
    setStackSize(spdImpl.code.stacksize.get());
    setPriority(spdImpl.code.priority.get());

    // Create local copies for all of the usesdevice entries for this implementation.
    LOG_TRACE(ImplementationInfo, "Loading component implementation uses device")
    const std::vector<UsesDevice>& spdUsesDevices = spdImpl.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                    spdUsesDev.getDependencies());
        addUsesDevice(usesDevice);
    }

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation property dependencies")
    const std::vector<ossie::PropertyRef>& dependencies = spdImpl.getDependencies();
    std::vector<ossie::PropertyRef>::const_iterator ii;
    for (ii = dependencies.begin(); ii != dependencies.end(); ++ii) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation property dependency '" << *ii);
        addDependencyProperty(*ii);
    }
}

ImplementationInfo::~ImplementationInfo()
{
    for (std::vector<SoftpkgInfo*>::iterator ii = softPkgDependencies.begin(); ii != softPkgDependencies.end(); ++ii) {
        delete (*ii);
    }
}

ImplementationInfo* ImplementationInfo::buildImplementationInfo(CF::FileSystem_ptr fileSys, const SPD::Implementation& spdImpl)
{
    ImplementationInfo* impl = new ImplementationInfo(spdImpl);

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependencies")
    const std::vector<ossie::SPD::SoftPkgRef>& softpkgDependencies = spdImpl.getSoftPkgDependencies();
    std::vector<ossie::SPD::SoftPkgRef>::const_iterator jj;
    for (jj = softpkgDependencies.begin(); jj != softpkgDependencies.end(); ++jj) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependency '" << *jj);
        std::auto_ptr<SoftpkgInfo> softpkg(SoftpkgInfo::buildSoftpkgInfo(fileSys, jj->localfile.c_str()));
        impl->addSoftPkgDependency(softpkg.release());
    }

    return impl;
}

const std::string& ImplementationInfo::getId() const
{
    return id;
}

const std::vector<std::string>& ImplementationInfo::getProcessorDeps() const
{
    return processorDeps;
}

const std::vector<SoftpkgInfo*>& ImplementationInfo::getSoftPkgDependency() const
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

const std::vector<PropertyRef>& ImplementationInfo::getDependencyProperties() const
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

void ImplementationInfo::addDependencyProperty(const PropertyRef& property)
{
    dependencyProperties.push_back(property);
}

void ImplementationInfo::addSoftPkgDependency(SoftpkgInfo* softpkg)
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


PREPARE_CF_LOGGING(SoftpkgInfo);

SoftpkgInfo::SoftpkgInfo(const std::string& spdFileName):
    _spdFileName(spdFileName)
{
}

SoftpkgInfo::~SoftpkgInfo()
{
    for (ImplementationInfo::List::iterator ii = _implementations.begin(); ii != _implementations.end(); ++ii) {
        delete *ii;
    }
}

const std::string& SoftpkgInfo::getSpdFileName() const
{
    return _spdFileName;
}

const std::string& SoftpkgInfo::getName() const
{
    return _name;
}

SoftpkgInfo* SoftpkgInfo::buildSoftpkgInfo(CF::FileSystem_ptr fileSys, const char* spdFileName)
{
    LOG_TRACE(SoftpkgInfo, "Building soft package info from file " << spdFileName);

    std::auto_ptr<ossie::SoftpkgInfo> softpkg(new SoftpkgInfo(spdFileName));

    if (!softpkg->parseProfile(fileSys)) {
        return 0;
    } else {
        return softpkg.release();
    }
}

bool SoftpkgInfo::parseProfile(CF::FileSystem_ptr fileSys)
{
    try {
        File_stream spd_file(fileSys, _spdFileName.c_str());
        spd.load(spd_file, _spdFileName.c_str());
        spd_file.close();
    } catch (const ossie::parser_error& e) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
        LOG_ERROR(SoftpkgInfo, "Building component info problem; error parsing SPD: " << _spdFileName << ". " << parser_error_line << " The XML parser returned the following error: " << e.what());
        return false;
    } catch (...) {
        LOG_ERROR(SoftpkgInfo, "Building component info problem; unknown error parsing SPD: " << _spdFileName );
        return false;
    }

    // Set name from the SPD
    _name = spd.getSoftPkgName();

    // Extract implementation data from SPD file
    const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();

    // Assume only one implementation, use first available result [0]
    for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
        const SPD::Implementation& spdImpl = spd_i[implCount];
        LOG_TRACE(SoftpkgInfo, "Adding implementation " << spdImpl.getID());
        ImplementationInfo* newImpl = ImplementationInfo::buildImplementationInfo(fileSys, spdImpl);
        addImplementation(newImpl);
    }

    // Create local copies for all of the usesdevice entries for this implementation.
    const std::vector<UsesDevice>& spdUsesDevices = spd.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                                                        spdUsesDev.getDependencies());
        addUsesDevice(usesDevice);
    }

    return true;
}

void SoftpkgInfo::addImplementation(ImplementationInfo* impl)
{
    _implementations.push_back(impl);
}

const ImplementationInfo::List& SoftpkgInfo::getImplementations() const
{
    return _implementations;
}

const UsesDeviceInfo* SoftpkgInfo::getUsesDeviceById(const std::string& id) const
{
    return UsesDeviceContext::getUsesDeviceById(id);
}

////////////////////////////////////////////////////
/*
 * ComponentInfo member function definitions
 */
PREPARE_CF_LOGGING(ComponentInfo);

ComponentInfo* ComponentInfo::buildComponentInfoFromSPDFile(CF::FileSystem_ptr fileSys,
                                                            const std::string& spdFileName,
                                                            const ComponentInstantiation* instantiation)
{
    LOG_TRACE(ComponentInfo, "Building component info from file " << spdFileName);

    ossie::ComponentInfo* newComponent = new ossie::ComponentInfo(spdFileName, instantiation);

    if (!newComponent->parseProfile(fileSys)) {
        delete newComponent;
        return 0;
    }
    
    if (newComponent->spd.getSCDFile() != 0) {
        try {
            File_stream _scd(fileSys, newComponent->spd.getSCDFile());
            newComponent->scd.load(_scd);
            _scd.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            LOG_ERROR(ComponentInfo, "Building component info problem; error parsing SCD: " << newComponent->spd.getSCDFile() << ". " << parser_error_line << " The XML parser returned the following error: " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "Building component info problem; unknown error parsing SCD: " << newComponent->spd.getSCDFile() );
            delete newComponent;
            return 0;
        }
    }

    if (newComponent->spd.getPRFFile() != 0) {
        LOG_DEBUG(ComponentInfo, "Loading component properties from " << newComponent->spd.getPRFFile());
        try {
            File_stream _prf(fileSys, newComponent->spd.getPRFFile());
            LOG_TRACE(ComponentInfo, "Parsing component properties");
            newComponent->prf.load(_prf);
            LOG_TRACE(ComponentInfo, "Closing PRF file")
            _prf.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            LOG_ERROR(ComponentInfo, "Building component info problem; error parsing PRF: " << newComponent->spd.getPRFFile() << ". " << parser_error_line << " The XML parser returned the following error: " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "Building component info problem; unknown error parsing PRF: " << newComponent->spd.getPRFFile());
            delete newComponent;
            return 0;
        }
    }

    if (newComponent->spd.isScaNonCompliant()) {
        newComponent->setIsScaCompliant(false);
    } else {
        newComponent->setIsScaCompliant(true);
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

        const std::vector<const Property*>& cprop = newComponent->prf.getConstructProperties();
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
    
    LOG_TRACE(ComponentInfo, "Done building component info from file " << spdFileName);
    return newComponent;
}

ComponentInfo::ComponentInfo(const std::string& spdFileName, const ComponentInstantiation* instantiation) :
    SoftpkgInfo(spdFileName),
    _isAssemblyController(false),
    _isScaCompliant(true),
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

const ComponentInstantiation* ComponentInfo::getInstantiation() const
{
    return instantiation;
}

void ComponentInfo::setIdentifier(const std::string& _identifier)
{
    instantiationId = _identifier;
}

void ComponentInfo::setNamingService(const bool _isNamingService)
{
    this->_isNamingService = _isNamingService;
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

void ComponentInfo::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = prf.getProperty(id);
    if (prop != NULL) {
        if (prop->isReadOnly()) {
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
    return instantiationId;
}

bool ComponentInfo::isNamingService() const
{
    return _isNamingService;
}

const char* ComponentInfo::getUsageName() const
{
    return usageName.c_str();
}

const char* ComponentInfo::getNamingServiceName() const
{
    return namingServiceName.c_str();
}

bool ComponentInfo::isResource() const
{
    return scd.isResource();
}

bool ComponentInfo::isConfigurable() const
{
    return scd.isConfigurable();
}


bool ComponentInfo::isAssemblyController() const
{
    return _isAssemblyController;
}

bool ComponentInfo::isScaCompliant() const
{
    return _isScaCompliant;
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
        const ossie::Property* prop = this->prf.getProperty(ossie::corba::returnString(cP->id));
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
