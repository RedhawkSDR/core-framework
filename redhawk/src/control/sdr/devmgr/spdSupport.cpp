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
#include "spdSupport.h"

using namespace ossie::SpdSupport;

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

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation property dependencies")
    const std::vector<ossie::SPD::PropertyRef>& dependencies = spdImpl.getDependencies();
    std::vector<ossie::SPD::PropertyRef>::const_iterator ii;
    for (ii = dependencies.begin(); ii != dependencies.end(); ++ii) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation property dependency '" << *ii);
        addDependencyProperty(*ii);
    }
}

ImplementationInfo::ImplementationInfo( const ImplementationInfo &src ) {
  id = src.id;
  codeType = src.codeType;
  localFileName = src.localFileName;
  entryPoint = src.entryPoint;
  stackSize = src.stackSize;
  priority = src.priority;
  _hasStackSize = src._hasStackSize;
  _hasPriority = src._hasPriority;
  processorDeps.resize(src.processorDeps.size());
  std::copy( src.processorDeps.begin(), src.processorDeps.end(), processorDeps.begin());
  osDeps.resize(src.osDeps.size());
  std::copy( src.osDeps.begin(), src.osDeps.end(), osDeps.begin());
  dependencyProperties.resize(src.dependencyProperties.size());
  std::copy( src.dependencyProperties.begin(), src.dependencyProperties.end(), dependencyProperties.begin());
  softPkgDependencies.resize(src.softPkgDependencies.size());
  std::copy( src.softPkgDependencies.begin(), src.softPkgDependencies.end(), softPkgDependencies.begin());
}


ImplementationInfo::~ImplementationInfo()
{
}

void ImplementationInfo::BuildImplementationInfo(CF::FileSystem_ptr fileSys, 
					     const SPD::Implementation& spdImpl,
					     ImplementationInfo &rimpl )
{
    ImplementationInfo impl(spdImpl);

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependencies")
    const std::vector<ossie::SPD::SoftPkgRef>& softpkgDependencies = spdImpl.getSoftPkgDependencies();
    std::vector<ossie::SPD::SoftPkgRef>::const_iterator jj;
    for (jj = softpkgDependencies.begin(); jj != softpkgDependencies.end(); ++jj) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependency '" << *jj);
        try{
          SoftpkgInfo softpkg (jj->localfile.c_str());
	  SoftpkgInfo::BuildSoftpkgInfo(fileSys, jj->localfile.c_str(), softpkg );
	  impl.addSoftPkgDependency(softpkg);
        }catch(...){
        }
    }

    rimpl = impl;
}

bool ImplementationInfo::operator==( const ImplementationInfo &other ) const {
  
  bool retval =false;
  if ( id != other.id ) return retval;
  if ( localFileName != other.localFileName ) return retval;
  if ( entryPoint != other.entryPoint ) return retval;
  if ( codeType != other.codeType ) return retval;
  return retval;
}


const std::string& ImplementationInfo::getId() const
{
    return id;
}

const std::vector<std::string>& ImplementationInfo::getProcessorDeps() const
{
    return processorDeps;
}

const std::vector<SoftpkgInfo>& ImplementationInfo::getSoftPkgDependency() const
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

const std::vector<ossie::SPD::PropertyRef>& ImplementationInfo::getDependencyProperties() const
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

void ImplementationInfo::addSoftPkgDependency(SoftpkgInfo & softpkg)
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

void ImplementationInfo::clearSelectedDependencyImplementations()
{
  std::vector<ossie::SpdSupport::SoftpkgInfo>::iterator iter;
    for (iter = softPkgDependencies.begin(); iter != softPkgDependencies.end(); ++iter) {
        iter->clearSelectedImplementation();
    }
}


PREPARE_LOGGING(SoftpkgInfo);

SoftpkgInfo::SoftpkgInfo(const std::string& spdFileName):
  _spdFileName(spdFileName),
    _selectedImplementation(_implementations.end())
{
}

SoftpkgInfo::SoftpkgInfo():
    _selectedImplementation(_implementations.end())
{
}

SoftpkgInfo::~SoftpkgInfo()
{
}


SoftpkgInfo::SoftpkgInfo ( const SoftpkgInfo &src ) {
   _name = src._name;
   _identifier = src._identifier;
   _implementations = src._implementations;
}

SoftpkgInfo &SoftpkgInfo::operator=(const SoftpkgInfo &src ) {
   _name = src._name;
   _identifier = src._identifier;
   _implementations = src._implementations;
  return *this;
}



const char* SoftpkgInfo::getSpdFileName()
{
    return _spdFileName.c_str();
}

const char* SoftpkgInfo::getName()
{
    return _name.c_str();
}

const char* SoftpkgInfo::getID()
{
    return _identifier.c_str();
}

void SoftpkgInfo::BuildSoftpkgInfo(CF::FileSystem_ptr fileSys, const char* spdFileName,
				   SoftpkgInfo &spd )
{
    LOG_TRACE(SoftpkgInfo, "Building soft package info from file " << spdFileName);

    SoftpkgInfo softpkg(spdFileName);

    if (softpkg.parseProfile(fileSys) == false ) {
      throw -1;
    }
    
    spd =  softpkg;
    return;
}

bool SoftpkgInfo::parseProfile(CF::FileSystem_ptr fileSys)
{
    try {
        File_stream spd_file(fileSys, _spdFileName.c_str());
        spd.load(spd_file, _spdFileName.c_str());
        spd_file.close();
    } catch (const ossie::parser_error& e) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
        LOG_ERROR(SoftpkgInfo, "building component info problem; error parsing spd; " << parser_error_line << "The XML parser returned the following error: " << e.what());
        return false;
    } catch (...) {
        LOG_ERROR(SoftpkgInfo, "building component info problem; unknown error parsing spd;");
        return false;
    }

    // Set name from the SPD
    _name = spd.getSoftPkgName();
    _identifier = spd.getSoftPkgID();
    LOG_DEBUG(SoftpkgInfo, "name/id " << _name << "/" << _identifier);

    // Extract implementation data from SPD file
    const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();

    // Assume only one implementation, use first available result [0]
    for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
        const SPD::Implementation& spdImpl = spd_i[implCount];
        LOG_TRACE(SoftpkgInfo, "Adding implementation " << spdImpl.getID());
        ImplementationInfo newImpl;
	ImplementationInfo::BuildImplementationInfo(fileSys, spdImpl,newImpl);
        addImplementation(newImpl);
    }

    return true;
}

void SoftpkgInfo::addImplementation(ImplementationInfo &impl)
{
    _implementations.push_back(impl);
}

void SoftpkgInfo::getImplementations(ImplementationInfo::List& res)
{
    std::copy(_implementations.begin(), _implementations.end(), std::back_inserter(res));
}

void SoftpkgInfo::setSelectedImplementation(ImplementationInfo &implementation)
{
  _selectedImplementation = std::find(_implementations.begin(), _implementations.end(), implementation);
  if (_selectedImplementation == _implementations.end()) {
    throw std::logic_error("invalid implementation selected");
  }
}

void SoftpkgInfo::clearSelectedImplementation()
{
  _selectedImplementation = _implementations.end();
}

const ImplementationInfo &SoftpkgInfo::getSelectedImplementation() const
{
  if ( _selectedImplementation == _implementations.end() ) 
    throw 0;

  return *_selectedImplementation;
}


////////////////////////////////////////////////////
/*
 * ResourceInfo member function definitions
 */
PREPARE_LOGGING(ResourceInfo);

void ResourceInfo::LoadResource(CF::FileSystem_ptr fileSys, 
					       const char* spdFileName,
					       ResourceInfo &rsc)
{
    LOG_TRACE(ResourceInfo, "Building component info from file " << spdFileName);

    ResourceInfo newComponent(spdFileName);

    if (!newComponent.parseProfile(fileSys)) {
      throw 0;
    }
    
    if (newComponent.spd.getSCDFile() != 0) {
        try {
            File_stream _scd(fileSys, newComponent.spd.getSCDFile());
            newComponent.scd.load(_scd);
            _scd.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            LOG_ERROR(ResourceInfo, "building component info problem; error parsing scd; " << parser_error_line << "The XML parser returned the following error: " << e.what());
            throw 0;
        } catch( ... ) {
            LOG_ERROR(ResourceInfo, "building component info problem; unknown error parsing scd;");
            throw 0;
        }
    }

    if (newComponent.spd.getPRFFile() != 0) {
        LOG_DEBUG(ResourceInfo, "Loading component properties from " << newComponent.spd.getPRFFile());
        try {
            File_stream _prf(fileSys, newComponent.spd.getPRFFile());
            LOG_DEBUG(ResourceInfo, "Parsing component properties");
            newComponent.prf.load(_prf);
            LOG_TRACE(ResourceInfo, "Closing PRF file")
            _prf.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            LOG_ERROR(ResourceInfo, "building component info problem; error parsing prf; " << parser_error_line << "The XML parser returned the following error: " << e.what());
            throw  0;
        } catch( ... ) {
            LOG_ERROR(ResourceInfo, "building component info problem; unknown error parsing prf;");
            throw 0;
        }
    }

    if (newComponent.spd.isScaNonCompliant()) {
        newComponent.setIsScaCompliant(false);
    } else {
        newComponent.setIsScaCompliant(true);
    }

    // Extract Properties from the implementation-agnostic PRF file
    // once we match the component to a device we can grab the implementation
    // specific PRF file
    if (newComponent.spd.getPRFFile() != 0) {
        // Handle component properties
        LOG_TRACE(ResourceInfo, "Adding factory params")
        const std::vector<const Property*>& fprop = newComponent.prf.getFactoryParamProperties();
        for (unsigned int i = 0; i < fprop.size(); i++) {
            newComponent.addFactoryParameter(convertPropertyToDataType(fprop[i]));
        }

        LOG_TRACE(ResourceInfo, "Adding exec params")
        const std::vector<const Property*>& eprop = newComponent.prf.getExecParamProperties();
        for (unsigned int i = 0; i < eprop.size(); i++) {
            if (std::string(eprop[i]->getMode()) != "readonly") {
                LOG_TRACE(ResourceInfo, "Adding exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
                newComponent.addExecParameter(convertPropertyToDataType(eprop[i]));
            } else {
                LOG_TRACE(ResourceInfo, "Ignoring readonly exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
            }
        }

        // According to SCA 2.2.2 Appendix D 4.1.1.6 the only properties
        // that are to be sent to allocateCapacity are device
        // properties that are external.  Furthermore, the SPD list's its
        // needs, not in the PRF file, but in the SPD file <dependencies>
        // element
        // prop = prf->getMatchingProperties();
        //for (unsigned int i=0; i < prop->size(); i++) {
        //    newComponent.addAllocationCapacity((*prop)[i]->getDataType());
        //}

        const std::vector<const Property*>& prop = newComponent.prf.getConfigureProperties();
        for (unsigned int i = 0; i < prop.size(); i++) {
            if (!prop[i]->isReadOnly()) {
                LOG_TRACE(ResourceInfo, "Adding configure prop " << prop[i]->getID() << " " << prop[i]->getName() << " " << prop[i]->isReadOnly())
                newComponent.addConfigureProperty(convertPropertyToDataType(prop[i]));
            }
        }

        const std::vector<const Property*>& cprop = newComponent.prf.getConstructProperties();
        for (unsigned int i = 0; i < cprop.size(); i++) {
          LOG_TRACE(ResourceInfo, "Adding construct prop " << cprop[i]->getID() << " " << cprop[i]->getName() << " " << cprop[i]->isReadOnly());
          if (cprop[i]->isCommandLine()) {
            newComponent.addExecParameter(convertPropertyToDataType(cprop[i]));
          } else {
            newComponent.addConstructProperty(convertPropertyToDataType(cprop[i]));
          }
        }

    }
        
    LOG_TRACE(ResourceInfo, "Done building component info from file " << spdFileName);
    rsc = newComponent;
    return;
}

ResourceInfo::ResourceInfo(const std::string& spdFileName) :
    SoftpkgInfo(spdFileName),
    _isAssemblyController(false),
    _isScaCompliant(true)
{
    resolved_softpkg_dependencies.resize(0);

    // add internal affinity properties
    try {
      std::stringstream os(redhawk::affinity::get_property_definitions());
      _affinity_prf.load(os);
    }
    catch(...){
    }

}


ResourceInfo & ResourceInfo::operator=( const ResourceInfo& src) {
  SoftpkgInfo::operator=(src);
  _copy(src);
  return *this;
}

ResourceInfo::ResourceInfo( const ResourceInfo& src) : SoftpkgInfo(src)
{
  _copy(src);
}

void ResourceInfo::_copy( const ResourceInfo& src) 
{
  prf = src.prf;
  scd = src.scd;
  _isAssemblyController = src._isAssemblyController;
  _isConfigurable = src._isConfigurable;
  _isScaCompliant = src._isScaCompliant;
  isNamingService = src.isNamingService;
  usageName = src.usageName;
  identifier = src.identifier;
  instantiationId = src.instantiationId;
  namingServiceName = src.namingServiceName;
  loggingConfig = src.loggingConfig;
  configureProperties = src.configureProperties;
  ctorProperties = src.ctorProperties;
  options = src.options;
  factoryParameters = src.factoryParameters;
  execParameters = src.execParameters;
  affinityOptions = src.affinityOptions;
  resolved_softpkg_dependencies.resize(src.resolved_softpkg_dependencies.size());
  std::copy( src.resolved_softpkg_dependencies.begin(),
	     src.resolved_softpkg_dependencies.end(), resolved_softpkg_dependencies.begin() );
}

ResourceInfo::~ResourceInfo ()
{
}

void ResourceInfo::addResolvedSoftPkgDependency(const std::string &dep) {
    this->resolved_softpkg_dependencies.push_back(dep);
}

std::vector<std::string> ResourceInfo::getResolvedSoftPkgDependencies() {
    return this->resolved_softpkg_dependencies;
}

void ResourceInfo::setIdentifier(const char* _identifier, std::string instance_id)
{
    identifier = _identifier;
    // Per the SCA spec, the identifier is the instantiation ID:waveform_name
    instantiationId = instance_id;
}


void ResourceInfo::setNamingService(const bool _isNamingService)
{
    isNamingService = _isNamingService;
}

void ResourceInfo::setNamingServiceName(const char* _namingServiceName)
{
    namingServiceName = _namingServiceName;
}

void ResourceInfo::setUsageName(const char* _usageName)
{
    if (_usageName != 0) {
        usageName = _usageName;
    }
}

void ResourceInfo::setIsAssemblyController(bool _isAssemblyController)
{
    this->_isAssemblyController = _isAssemblyController;
}

void ResourceInfo::setIsScaCompliant(bool _isScaCompliant)
{
    this->_isScaCompliant = _isScaCompliant;
}


void ResourceInfo::setAffinity( const AffinityProperties &affinity_props )
{
  for (unsigned int i = 0; i < affinity_props.size(); ++i) {
    const ossie::ComponentProperty* propref = &affinity_props[i];
    std::string propId = propref->getID();
    RH_NL_DEBUG("ResourceInfo", "Affinity property id = " << propId);
    const Property* prop = _affinity_prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
      RH_NL_WARN("ResourceInfo", "Ignoring attempt to override property " << propId << ", Reason: Property ID does not exist in component");
        continue;
    }

    // add property
    CF::DataType dt = overridePropertyValue(prop, propref);
    addProperty( dt, affinityOptions );
  }

}


void ResourceInfo::setLoggingConfig( const LoggingConfig  &logcfg )
{
  loggingConfig = logcfg;
}


void ResourceInfo::addFactoryParameter(CF::DataType dt)
{
    addProperty(dt, factoryParameters);
}

void ResourceInfo::addExecParameter(CF::DataType dt)
{
    addProperty(dt, execParameters);
}

void ResourceInfo::addConfigureProperty(CF::DataType dt)
{
    addProperty(dt, configureProperties);
}

void ResourceInfo::addConstructProperty(CF::DataType dt)
{
    addProperty(dt, ctorProperties);
}

void ResourceInfo::overrideProperty(const ossie::ComponentProperty& propref) {
  overrideProperty(&propref);
}

void ResourceInfo::overrideProperty(const ossie::ComponentProperty* propref) {
    std::string propId = propref->getID();
    LOG_TRACE(ResourceInfo, "Instantiation property id = " << propId)
    const Property* prop = prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ResourceInfo, "Ignoring attempt to override property " << propId << " Reason: Property ID not exist in component")
        return;
    }

    CF::DataType dt = overridePropertyValue(prop, propref);
    overrideProperty(dt.id, dt.value);
}


void ResourceInfo::overrideSimpleProperty(const char* id, const std::string value)
{
    const Property* prop = prf.getProperty(id);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ResourceInfo, "Ignoring attempt to override property " << id << " Reason: Property ID does not exist in component")
        return;
    }

    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(prop);
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(simple->getType()));
        CORBA::Any val = ossie::string_to_any(value, type);
        overrideProperty(id, val);
    } else {
        LOG_WARN(ResourceInfo, "attempt to override non-simple property with string value");
    }
}

void ResourceInfo::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = prf.getProperty(id);
    if (prop != NULL) {
        if (prop->isReadOnly()) {
          LOG_WARN(ResourceInfo, "Ignoring attempt to override readonly property " << id);
            return;
        }
    }
    process_overrides(&ctorProperties, id, value);
    process_overrides(&configureProperties, id, value);
    process_overrides(&options, id, value);
    process_overrides(&factoryParameters, id, value);
    process_overrides(&execParameters, id, value);
}



void ResourceInfo::process_overrides(CF::Properties* props, const char* id, CORBA::Any value)
{
    LOG_DEBUG(ResourceInfo, "Attempting to override property " << id);
    for (unsigned int i = 0; i < (*props).length(); ++i ) {
        if (strcmp(id, (*props)[i].id) == 0) {
            LOG_DEBUG(ResourceInfo, "Overriding property " << id << " with value " << ossie::any_to_string(value));
            (*props)[i].value = value;
        }
    }
    return;
}


const char* ResourceInfo::getInstantiationIdentifier()
{
    return instantiationId.c_str();
}

const char* ResourceInfo::getIdentifier()
{
    return identifier.c_str();
}


const bool  ResourceInfo::getNamingService()
{
    return isNamingService;
}

const char* ResourceInfo::getUsageName()
{
    return usageName.c_str();
}

const char* ResourceInfo::getNamingServiceName()
{
    return namingServiceName.c_str();
}


const bool  ResourceInfo::isResource()
{
    return scd.isResource();
}

const bool  ResourceInfo::isConfigurable()
{
    return scd.isConfigurable();
}


const bool  ResourceInfo::isAssemblyController()
{
    return _isAssemblyController;
}

const bool  ResourceInfo::isScaCompliant()
{
    return _isScaCompliant;
}

CF::Properties ResourceInfo::getNonNilConfigureProperties()
{
    return ossie::getNonNilConfigureProperties(configureProperties);
}

CF::Properties ResourceInfo::getNonNilConstructProperties()
{
    return ossie::getNonNilProperties(ctorProperties);
}

CF::Properties ResourceInfo::getAffinityOptions()
{
    return affinityOptions;
}


void ResourceInfo::mergeAffinityOptions( const CF::Properties &new_affinity )
{
  // for each new affinity setting apply settings to component's affinity options
  const redhawk::PropertyMap &newmap = redhawk::PropertyMap::cast(new_affinity);
  redhawk::PropertyMap &currentAffinity = redhawk::PropertyMap::cast(affinityOptions);
  redhawk::PropertyMap::const_iterator iter = newmap.begin();
  for ( ; iter != newmap.end(); iter++ ) {
    std::string id = iter->getId();
    currentAffinity[id]=newmap[id];
  }
}

CF::Properties ResourceInfo::getConfigureProperties()
{
    return configureProperties;
}


CF::Properties ResourceInfo::getConstructProperties()
{
    return ctorProperties;
}


CF::Properties ResourceInfo::getOptions()
{
    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    //  unfortunately this can't happen until an implementation has been chosen
  if (_selectedImplementation != _implementations.end()) {
        if (_selectedImplementation->hasStackSize()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("STACK_SIZE");  // 3.1.3.3.3.3.6
            options[options.length()-1].value <<= _selectedImplementation->getStackSize();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
        if (_selectedImplementation->hasPriority()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("PRIORITY");  // 3.1.3.3.3.3.7
            options[options.length()-1].value <<= _selectedImplementation->getPriority();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
    }

    // Pass all afinity settings under single options list
    CF::Properties affinity_options;
    for ( uint32_t i=0; i < affinityOptions.length(); i++ ) {
      affinity_options.length(affinity_options.length()+1);
      affinity_options[affinity_options.length()-1] = affinityOptions[i];
      CF::DataType dt = affinityOptions[i];
      RH_NL_DEBUG("ResourceInfo", "Affinity Property: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
    }

    if ( affinity_options.length() > 0 ) {
      options.length(options.length()+1);
      options[options.length()-1].id = CORBA::string_dup("AFFINITY"); 
      options[options.length()-1].value <<= affinity_options;
      RH_NL_INFO("ResourceInfo", "Affinity Sequence... Extending options set length: " << affinity_options.length());
    }

    return options;
}

CF::Properties ResourceInfo::getExecParameters()
{
    return execParameters;
}

