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
PREPARE_CF_LOGGING(ImplementationInfo);

ImplementationInfo::ImplementationInfo(const SPD::Implementation& spdImpl) :
    id(spdImpl.getID()),
    codeType(),
    _codeType(),
    propertyFile(),
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
    setPropertyFile(spdImpl.getPRFFile());

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation property dependencies")
    const std::vector<ossie::SPD::PropertyRef>& dependencies = spdImpl.getDependencies();
    std::vector<ossie::SPD::PropertyRef>::const_iterator ii;
    for (ii = dependencies.begin(); ii != dependencies.end(); ++ii) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation property dependency '" << *ii);
        addDependencyProperty(*ii);
    }
}


ImplementationInfo::~ImplementationInfo()
{
    for ( SoftpkgInfoList::iterator ii = softPkgDependencies.begin(); ii != softPkgDependencies.end(); ++ii) {
        delete (*ii);
    }
}

ImplementationInfo *ImplementationInfo::BuildImplementationInfo(CF::FileSystem_ptr fileSys, 
                                                                const SPD::Implementation& spdImpl,
								CF::FileSystem_ptr  depFileSys )
{
    std::auto_ptr<ImplementationInfo> impl(new ImplementationInfo(spdImpl));

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependencies")
    const std::vector<ossie::SPD::SoftPkgRef>& softpkgDependencies = spdImpl.getSoftPkgDependencies();
    std::vector<ossie::SPD::SoftPkgRef>::const_iterator jj;
    for (jj = softpkgDependencies.begin(); jj != softpkgDependencies.end(); ++jj) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependency '" << *jj);
        std::auto_ptr<SoftpkgInfo> softpkg(SoftpkgInfo::BuildSoftpkgInfo(depFileSys, jj->localfile.c_str(),depFileSys));
        impl->addSoftPkgDependency(softpkg.release());
    }

    return impl.release();
}


bool ImplementationInfo::operator==( const ImplementationInfo &other ) const {
  
  bool retval =false;
  if ( id != other.id ) return retval;
  if ( localFileName != other.localFileName ) return retval;
  if ( entryPoint != other.entryPoint ) return retval;
  if ( codeType != other.codeType ) return retval;
  return true;
}


const std::string& ImplementationInfo::getId() const
{
    return id;
}

const std::vector<std::string>& ImplementationInfo::getProcessorDeps() const
{
    return processorDeps;
}

const SoftpkgInfoList & ImplementationInfo::getSoftPkgDependencies() const
{
    return softPkgDependencies;
}

const std::vector<ossie::SPD::NameVersionPair>& ImplementationInfo::getOsDeps() const
{
    return osDeps;
}

const std::string& ImplementationInfo::getLocalFileName() const
{
    return localFileName;
}

CF::LoadableDevice::LoadType ImplementationInfo::getCodeType() const
{
    return codeType;
}

const std::string& ImplementationInfo::getPropertyFile() const
{
    return propertyFile;
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
    _codeType = type;
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

void ImplementationInfo::setPropertyFile(const char* fileName)
{
    if (fileName) {
        propertyFile = fileName;
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

void ImplementationInfo::addSoftPkgDependency(SoftpkgInfo * softpkg)
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
    SoftpkgInfoList::iterator iter;
    for (iter = softPkgDependencies.begin(); iter != softPkgDependencies.end(); ++iter) {
        (*iter)->clearSelectedImplementation();
    }
}


PREPARE_CF_LOGGING(SoftpkgInfo);

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
    for (ImplementationInfo::List::iterator ii = _implementations.begin(); ii != _implementations.end(); ++ii) {
        delete *ii;
    }

}


const char* SoftpkgInfo::getSpdFileName() const
{
    return _spdFileName.c_str();
}

const char* SoftpkgInfo::getName() const
{
    return _name.c_str();
}

const char* SoftpkgInfo::getID() const
{
    return _identifier.c_str();
}

SoftpkgInfo *SoftpkgInfo::BuildSoftpkgInfo(CF::FileSystem_ptr fileSys, const char* spdFileName, 
					   CF::FileSystem_ptr depFileSys )

{
    LOG_TRACE(SoftpkgInfo, "Building soft package info from file " << spdFileName);

    std::auto_ptr<SoftpkgInfo> softpkg(new SoftpkgInfo(spdFileName));

    if (!softpkg->parseProfile(fileSys,depFileSys)) {
        return 0;
    } else {
        return softpkg.release();
    }
}

bool SoftpkgInfo::parseProfile(CF::FileSystem_ptr fileSys, CF::FileSystem_ptr depFileSys )
{
    try {
        LOG_TRACE(SoftpkgInfo, "Parsing SPD file:  " << _spdFileName );
        File_stream spd_file(fileSys, _spdFileName.c_str());
        spd.load(spd_file, _spdFileName.c_str());
        spd_file.close();
    } catch (const ossie::parser_error& e) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
        std::ostringstream eout;
        eout << "Building component info problem; error parsing SPD: " << _spdFileName << ". " << parser_error_line << " The XML parser returned the following error: " << e.what();
        throw std::runtime_error(eout.str().c_str());        
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while parsing the SPD: " << _spdFileName;
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while parsing the SPD: " << _spdFileName;
        throw std::runtime_error(eout.str().c_str());
    } catch ( ... ) {
        std::ostringstream eout;
        eout << "Unknown error, parsing SPD: " << _spdFileName;
        throw std::runtime_error(eout.str().c_str());
    }

    // Set name from the SPD
    _name = spd.getSoftPkgName();
    _identifier = spd.getSoftPkgID();
    LOG_DEBUG(SoftpkgInfo, "name/id " << _name << "/" << _identifier);

    // Extract implementation data from SPD file
    const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();

    for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
        const SPD::Implementation& spdImpl = spd_i[implCount];
        LOG_TRACE(SoftpkgInfo, "Adding implementation " << spdImpl.getID());
        ImplementationInfo* newImpl = ImplementationInfo::BuildImplementationInfo(fileSys, spdImpl, depFileSys);
        addImplementation(newImpl);
    }

    return true;
}

void SoftpkgInfo::addImplementation(ImplementationInfo *impl)
{
    _implementations.push_back(impl);
}

void SoftpkgInfo::getImplementations(ImplementationInfo::List& res) const
{
    std::copy(_implementations.begin(), _implementations.end(), std::back_inserter(res));
}

void SoftpkgInfo::setSelectedImplementation(const ImplementationInfo *implementation)
{
  _selectedImplementation = std::find(_implementations.begin(), _implementations.end(), implementation);
  if (_selectedImplementation == _implementations.end()) {
    throw std::logic_error("invalid implementation selected");
  }
}

void SoftpkgInfo::clearSelectedImplementation()
{
    if (*_selectedImplementation) {
        (*_selectedImplementation)->clearSelectedDependencyImplementations();
    }
    _selectedImplementation = _implementations.end();
}

const ImplementationInfo *SoftpkgInfo::getSelectedImplementation() const
{
    if ( _selectedImplementation == _implementations.end() ) 
        return 0;
    return *_selectedImplementation;
}

ImplementationInfo *SoftpkgInfo::selectedImplementation() const
{
  if ( _selectedImplementation == _implementations.end() ) 
    return 0;

  return *_selectedImplementation;
}


PREPARE_CF_LOGGING(ProgramProfile);


std::auto_ptr<ProgramProfile> ProgramProfile::LoadProgramProfile(CF::FileSystem_ptr fileSys, 
								 const char* spdFileName,
								 CF::FileSystem_ptr depFileSys ) {
  std::auto_ptr<ProgramProfile> ret( LoadProfile( fileSys, spdFileName, depFileSys ) );
    return ret;
}

ProgramProfile *ProgramProfile::LoadProfile(CF::FileSystem_ptr fileSys, 
					    const char* spdFileName,
					    CF::FileSystem_ptr depFileSys ) {
  
    LOG_TRACE(ProgramProfile, "Building component info from file " << spdFileName);

    std::auto_ptr<ProgramProfile> newComponent(new ProgramProfile(spdFileName));

    if ( !newComponent->parseProfile(fileSys, depFileSys) ) {
        return 0;
    }
    
    if (newComponent->spd.getSCDFile() != 0) {
        try {
            File_stream _scd(fileSys, newComponent->spd.getSCDFile());
            newComponent->scd.load(_scd);
            _scd.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            std::ostringstream eout;
            eout << "Building component info problem; error parsing SCD: " << newComponent->spd.getSCDFile() << ". " << parser_error_line << " The XML parser returned the following error: " << e.what();
            LOG_TRACE(ProgramProfile, eout.str());
            throw std::runtime_error(eout.str().c_str());
        } catch( ... ) {
            std::ostringstream eout;
            eout << "Building component info problem; unknown error parsing SCD: " << newComponent->spd.getSCDFile();
            LOG_TRACE(ProgramProfile, eout.str());
            throw std::runtime_error(eout.str().c_str());
        }
    }

    if (newComponent->spd.getPRFFile() != 0) {
        LOG_DEBUG(ProgramProfile, "Loading component properties from " << newComponent->spd.getPRFFile());
        try {
            File_stream _prf(fileSys, newComponent->spd.getPRFFile());
            LOG_DEBUG(ProgramProfile, "Parsing component properties");
            newComponent->prf.load(_prf);
            LOG_TRACE(ProgramProfile, "Closing PRF file")
            _prf.close();
        } catch (ossie::parser_error& e) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            std::ostringstream eout;
            eout << "Building component info problem; error parsing PRF: " << newComponent->spd.getPRFFile() << ". " << parser_error_line << " The XML parser returned the following error: " << e.what();
            LOG_TRACE(ProgramProfile, eout.str());
            throw std::runtime_error(eout.str().c_str());
        } catch( ... ) {
            std::ostringstream eout;
            eout << "Building component info problem; unknown error parsing PRF: " << newComponent->spd.getPRFFile();
            LOG_TRACE(ProgramProfile, eout.str());
            throw std::runtime_error(eout.str().c_str());
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
        LOG_TRACE(ProgramProfile, "Adding factory params")
        const std::vector<const Property*>& fprop = newComponent->prf.getFactoryParamProperties();
        for (unsigned int i = 0; i < fprop.size(); i++) {
            newComponent->addFactoryParameter(convertPropertyToDataType(fprop[i]));
        }

        LOG_TRACE(ProgramProfile, "Adding exec params")
        const std::vector<const Property*>& eprop = newComponent->prf.getExecParamProperties();
        for (unsigned int i = 0; i < eprop.size(); i++) {
            if (std::string(eprop[i]->getMode()) != "readonly") {
                LOG_TRACE(ProgramProfile, "Adding exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
                newComponent->addExecParameter(convertPropertyToDataType(eprop[i]));
            } else {
                if ( eprop[i]->isProperty() )  {
                    LOG_TRACE(ProgramProfile, "Adding exec param (readonly property) " << eprop[i]->getID() << " " << eprop[i]->getName());
                    newComponent->addExecParameter(convertPropertyToDataType(eprop[i]));
                }
                else {
                    LOG_TRACE(ProgramProfile, "Ignoring readonly exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
                }
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
                LOG_TRACE(ProgramProfile, "Adding configure prop " << prop[i]->getID() << " " << prop[i]->getName() << " " << prop[i]->isReadOnly())
                newComponent->addConfigureProperty(convertPropertyToDataType(prop[i]));
            }
        }

        const std::vector<const Property*>& cprop = newComponent->prf.getConstructProperties();
        for (unsigned int i = 0; i < cprop.size(); i++) {
          LOG_TRACE(ProgramProfile, "Adding construct prop " << cprop[i]->getID() << " " << cprop[i]->getName() << " " << cprop[i]->isReadOnly());
          if (cprop[i]->isCommandLine()) {
              LOG_TRACE(ProgramProfile, "Adding (cmdline) construct prop " << cprop[i]->getID() << " " << cprop[i]->getName() << " " << cprop[i]->isReadOnly());
            newComponent->addExecParameter(convertPropertyToDataType(cprop[i]));
          } else {
            newComponent->addConstructProperty(convertPropertyToDataType(cprop[i]));
          }
        }

    }

    newComponent->fillAllSeqForStructProperty();
    LOG_TRACE(ProgramProfile, "Done building component info from file " << spdFileName);
    return newComponent.release();
}

ProgramProfile::ProgramProfile(const std::string& spdFileName) :
    SoftpkgInfo(spdFileName),
    _isAssemblyController(false),
    _isScaCompliant(true)
{
    nicAssignment="";
    resolved_softpkg_dependencies.resize(0);

    // add internal affinity properties
    try {
      std::stringstream os(redhawk::affinity::get_property_definitions());
      _affinity_prf.load(os);
    }
    catch(...){
    }

}


ProgramProfile::~ProgramProfile ()
{
}

void ProgramProfile::addResolvedSoftPkgDependency(const std::string &dep) {
    this->resolved_softpkg_dependencies.push_back(dep);
}

std::vector<std::string> ProgramProfile::getResolvedSoftPkgDependencies() {
    return this->resolved_softpkg_dependencies;
}

void ProgramProfile::setIdentifier(const char* _identifier, std::string instance_id)
{
    identifier = _identifier;
    // Per the SCA spec, the identifier is the instantiation ID:waveform_name
    instantiationId = instance_id;
}


void ProgramProfile::setNamingService(const bool _isNamingService)
{
    isNamingService = _isNamingService;
}

void ProgramProfile::setNamingServiceName(const char* _namingServiceName)
{
    if ( _namingServiceName != 0 ) {
        namingServiceName = _namingServiceName;
    }
}

void ProgramProfile::setUsageName(const char* _usageName)
{
    if (_usageName != 0) {
        usageName = _usageName;
    }
}

void ProgramProfile::setIsAssemblyController(bool _isAssemblyController)
{
    this->_isAssemblyController = _isAssemblyController;
}

void ProgramProfile::setIsScaCompliant(bool _isScaCompliant)
{
    this->_isScaCompliant = _isScaCompliant;
}

void ProgramProfile::setNicAssignment(std::string nic) {
    nicAssignment = nic;
};


void ProgramProfile::setAffinity( const AffinityProperties &affinity_props )
{
  for (unsigned int i = 0; i < affinity_props.size(); ++i) {
    const ossie::ComponentProperty* propref = &affinity_props[i];
    std::string propId = propref->getID();
    RH_NL_DEBUG("ProgramProfile", "Affinity property id = " << propId);
    const Property* prop = _affinity_prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
      RH_NL_WARN("ProgramProfile", "Ignoring attempt to override property " << propId << ", Reason: Property ID does not exist in component");
        continue;
    }

    // add property
    CF::DataType dt = overridePropertyValue(prop, propref);
    addProperty( dt, affinityOptions );
  }

}


void ProgramProfile::setLoggingConfig( const LoggingConfig  &logcfg )
{
  loggingConfig = logcfg;
}


void ProgramProfile::addFactoryParameter(CF::DataType dt)
{
    addProperty(dt, factoryParameters);
}

void ProgramProfile::addExecParameter(CF::DataType dt)
{
    addProperty(dt, execParameters);
}

void ProgramProfile::addConfigureProperty(CF::DataType dt)
{
    addProperty(dt, configureProperties);
}

void ProgramProfile::addConstructProperty(CF::DataType dt)
{
    addProperty(dt, ctorProperties);
}

void ProgramProfile::overrideProperty(const ossie::ComponentProperty& propref) {
  overrideProperty(&propref);
}

void ProgramProfile::overrideProperty(const ossie::ComponentProperty* propref) {
    std::string propId = propref->getID();
    LOG_TRACE(ProgramProfile, "Instantiation property id = " << propId)
    const Property* prop = prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        if ( propId != "LOGGING_CONFIG_URI" and propId != "LOG_LEVEL" ) {
            LOG_WARN(ProgramProfile, "Ignoring attempt to override property " << propId << " Reason: Property ID not exist in component")
                return;
        }

    }

    // allow intrinstic properties to be command line
    if ( propId == "LOGGING_CONFIG_URI" or propId == "LOG_LEVEL" ) {
        LOG_DEBUG(ProgramProfile, "Allowing LOGGING_CONFIG_URI and LOG_LEVEL to be passed to override");
        //if (propId == "LOG_LEVEL") propId = "DEBUG_LEVEL";
        CF::DataType prop;
        prop.id = propId.c_str();
        prop.value <<= dynamic_cast<const SimplePropertyRef*>(propref)->getValue();
        addExecParameter(prop);

    }
    else{
        CF::DataType dt = overridePropertyValue(prop, propref);
        overrideProperty(dt.id, dt.value);
    }
}


void ProgramProfile::overrideSimpleProperty(const char* id, const std::string &value)
{
    const Property* prop = prf.getProperty(id);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ProgramProfile, "Ignoring attempt to override property " << id << " Reason: Property ID does not exist in component");
        return;
    }

    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(prop);
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(simple->getType()));
        CORBA::Any val = ossie::string_to_any(value, type);
        overrideProperty(id, val);
    } else {
        LOG_WARN(ProgramProfile, "attempt to override non-simple property with string value");
    }
}

void ProgramProfile::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = prf.getProperty(id);

    if (prop != NULL) {
        if (prop->isReadOnly()) {
            if ( !prop->isProperty()) {
                LOG_WARN(ProgramProfile, "Ignoring attempt to override readonly property " << id);
            }
            else {
                // allow read-only exec param properties
                if ( prop->isCommandLine()) {
                    LOG_TRACE(ProgramProfile, "overrideProperty (read-only command line ) id " << id << 
                              " with value " << ossie::any_to_string(value));
                    process_overrides(&execParameters, id, value);
                }
                process_overrides(&ctorProperties, id, value);
            }
            return;
        }
    }

    process_overrides(&ctorProperties, id, value);
    process_overrides(&configureProperties, id, value);
    process_overrides(&options, id, value);
    process_overrides(&factoryParameters, id, value);
    process_overrides(&execParameters, id, value);
}



void ProgramProfile::process_overrides(CF::Properties* props, const char* id, const CORBA::Any &value)
{
    LOG_DEBUG(ProgramProfile, "Attempting to override property " << id);
    for (unsigned int i = 0; i < (*props).length(); ++i ) {
        if (strcmp(id, (*props)[i].id) == 0) {
            LOG_DEBUG(ProgramProfile, "Overriding property " << id << " with value " << ossie::any_to_string(value));
            (*props)[i].value = value;
        }
    }
    return;
}


const char* ProgramProfile::getInstantiationIdentifier()
{
    return instantiationId.c_str();
}

const char* ProgramProfile::getIdentifier()
{
    return identifier.c_str();
}


const bool  ProgramProfile::getNamingService()
{
    return isNamingService;
}

const char* ProgramProfile::getUsageName()
{
    return usageName.c_str();
}

const char* ProgramProfile::getNamingServiceName()
{
    return namingServiceName.c_str();
}

const std::string ProgramProfile::getNicAssignment() {
    return nicAssignment;
};


const bool  ProgramProfile::isResource()
{
    return scd.isResource();
}

const bool  ProgramProfile::isConfigurable()
{
    return scd.isConfigurable();
}


const bool  ProgramProfile::isAssemblyController()
{
    return _isAssemblyController;
}

const bool  ProgramProfile::isScaCompliant()
{
    return _isScaCompliant;
}


void ProgramProfile::fillAllSeqForStructProperty() {
    this->fillSeqForStructProperty(configureProperties);
    this->fillSeqForStructProperty(ctorProperties);
    this->fillSeqForStructProperty(options);
    this->fillSeqForStructProperty(factoryParameters);
    this->fillSeqForStructProperty(execParameters);
}

void ProgramProfile::fillSeqForStructProperty(CF::Properties &props) {
    redhawk::PropertyMap& tmpProps = redhawk::PropertyMap::cast(props);
    // iterate over all the properties
    for (redhawk::PropertyMap::iterator tmpP = tmpProps.begin(); tmpP != tmpProps.end(); ++tmpP) {
        std::string tmp_id(ossie::corba::returnString(tmpP->id));
        // see if the property is a struct
        for (std::vector<const Property*>::const_iterator prf_iter = prf.getProperties().begin(); prf_iter != prf.getProperties().end(); ++prf_iter) {
            std::string prf_id((*prf_iter)->getID());
            if (prf_id == tmp_id) {
                const ossie::StructProperty* tmp_struct = dynamic_cast<const ossie::StructProperty*>(*prf_iter);
                if (tmp_struct) {
                    // check to see if the property has a combination of nil sequences and non-nil simples
                    CF::Properties* struct_val;
                    if (tmpP->value >>= struct_val) {
                        redhawk::PropertyMap& structProps = redhawk::PropertyMap::cast(*struct_val);
                        bool nilSeq = false;
                        bool nonNilVal = false;
                        for (redhawk::PropertyMap::iterator structIter = structProps.begin(); structIter != structProps.end(); ++structIter) {
                            if (structProps[ossie::corba::returnString(structIter->id)].isNil()) {
                                // is the nil value for a sequence?
                                for (std::vector<Property*>::const_iterator internal_iter = tmp_struct->getValue().begin(); internal_iter != tmp_struct->getValue().end(); ++internal_iter) {
                                    std::string _inner_id((*internal_iter)->getID());
                                    if (_inner_id == ossie::corba::returnString(structIter->id)) {
                                        if (dynamic_cast<const ossie::SimpleSequenceProperty*>(*internal_iter)) {
                                            nilSeq = true;
                                        }
                                    }
                                }
                            } else {
                                // is the non-nil value for a simple?
                                for (std::vector<Property*>::const_iterator internal_iter = tmp_struct->getValue().begin(); internal_iter != tmp_struct->getValue().end(); ++internal_iter) {
                                    std::string _inner_id((*internal_iter)->getID());
                                    if (_inner_id == ossie::corba::returnString(structIter->id)) {
                                        if (dynamic_cast<const ossie::SimpleProperty*>(*internal_iter)) {
                                            nonNilVal = true;
                                        }
                                    }
                                }
                            }
                        }
                        if (nilSeq and nonNilVal) {
                            // there's a nil sequence value and a non-nil simple. The nil sequence needs to be a zero-length sequence
                            for (redhawk::PropertyMap::iterator structIter = structProps.begin(); structIter != structProps.end(); ++structIter) {
                                if (structProps[ossie::corba::returnString(structIter->id)].isNil()) {
                                    // is the nil value for a sequence?
                                    for (std::vector<Property*>::const_iterator internal_iter = tmp_struct->getValue().begin(); internal_iter != tmp_struct->getValue().end(); ++internal_iter) {
                                        std::string _inner_id((*internal_iter)->getID());
                                        if (_inner_id == ossie::corba::returnString(structIter->id)) {
                                            const ossie::SimpleSequenceProperty* _type = dynamic_cast<const ossie::SimpleSequenceProperty*>(*internal_iter);
                                            std::vector<std::string> empty_string_vector;
                                            structProps[ossie::corba::returnString(structIter->id)] = ossie::strings_to_any(empty_string_vector, ossie::getTypeKind(_type->getType()));
                                        }
                                    }
                                }
                            }
                            tmpP->value <<= *struct_val;
                        }
                    }
                }
            }
        }
    }
}

bool ProgramProfile::checkStruct(CF::Properties &props)
{
    redhawk::PropertyMap& tmpProps = redhawk::PropertyMap::cast(props);
    int state = 0; // 1 set, -1 nil
    for (redhawk::PropertyMap::iterator tmpP = tmpProps.begin(); tmpP != tmpProps.end(); tmpP++) {
        if (tmpProps[ossie::corba::returnString(tmpP->id)].isNil()) {
            // ignore if the nil value is for a sequence (nil sequences are equivalent to zero-length sequences)
            bool foundSeq = false;
            for (std::vector<const Property*>::const_iterator prf_iter = prf.getProperties().begin(); prf_iter != prf.getProperties().end(); ++prf_iter) {
                const ossie::StructProperty* tmp_struct = dynamic_cast<const ossie::StructProperty*>(*prf_iter);
                if (tmp_struct) {
                    for (std::vector<Property*>::const_iterator internal_iter = tmp_struct->getValue().begin(); internal_iter != tmp_struct->getValue().end(); ++internal_iter) {
                        std::string _id((*internal_iter)->getID());
                        if (_id == ossie::corba::returnString(tmpP->id)) {
                            if (dynamic_cast<const ossie::SimpleSequenceProperty*>(*internal_iter)) {
                                foundSeq = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (foundSeq)
                continue;
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

CF::Properties ProgramProfile::iteratePartialStruct(CF::Properties &props)
{
    CF::Properties retval;
    redhawk::PropertyMap& configProps = redhawk::PropertyMap::cast(props);
    for (redhawk::PropertyMap::iterator cP = configProps.begin(); cP != configProps.end(); cP++) {
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



CF::Properties ProgramProfile::containsPartialStructConfig() {
    return this->iteratePartialStruct(configureProperties);
}

CF::Properties ProgramProfile::containsPartialStructConstruct()
{
    return this->iteratePartialStruct(ctorProperties);
}

CF::Properties ProgramProfile::getNonNilConfigureProperties()
{
    return ossie::getNonNilConfigureProperties(configureProperties);
}

CF::Properties ProgramProfile::getNonNilConstructProperties()
{
    return ossie::getNonNilProperties(ctorProperties);
}

CF::Properties ProgramProfile::getNonNilNonExecConstructProperties()
{
    return ossie::getNonNilProperties(ctorProperties);
}


CF::Properties ProgramProfile::getAffinityOptionsWithAssignment()
{
  // Add affinity setting first...
  CF::Properties affinity_options;
  for ( uint32_t i=0; i < affinityOptions.length(); i++ ) {
      affinity_options.length(affinity_options.length()+1);
      affinity_options[affinity_options.length()-1] = affinityOptions[i];
      CF::DataType dt = affinityOptions[i];
      RH_NL_DEBUG("spdSupport", "ProgramProfile getAffinityOptionsWithAssignment ... Affinity Property: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
  }

  // add nic allocations to affinity list 
  if ( nicAssignment != "" ) {
      affinity_options.length(affinity_options.length()+1);
      affinity_options[affinity_options.length()-1].id = CORBA::string_dup("nic");  
      affinity_options[affinity_options.length()-1].value <<= nicAssignment.c_str(); 
      RH_NL_DEBUG("spdSupport", "ProgramProfile getAffinityOptionsWithAssignment ... NIC AFFINITY: pol/value "  <<  "nic"  << "/" << nicAssignment );
  }      

  return affinity_options;

}

CF::Properties ProgramProfile::getAffinityOptions()
{
    return affinityOptions;
}

void ProgramProfile::mergeAffinityOptions( const CF::Properties &new_affinity )
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

CF::Properties ProgramProfile::getConfigureProperties()
{
    return configureProperties;
}


CF::Properties ProgramProfile::getConstructProperties()
{
    return ctorProperties;
}




CF::Properties ProgramProfile::getOptions()
{
    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    //  unfortunately this can't happen until an implementation has been chosen
    if (*_selectedImplementation) {
        ImplementationInfo *impl = *_selectedImplementation;
        if ( impl->hasStackSize()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("STACK_SIZE");  // 3.1.3.3.3.3.6
            options[options.length()-1].value <<= impl->getStackSize();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
        if (impl->hasPriority()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("PRIORITY");  // 3.1.3.3.3.3.7
            options[options.length()-1].value <<= impl->getPriority();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
    }

    // Add affinity settings under AFFINITY property directory
    CF::Properties affinity_options;
    for ( uint32_t i=0; i < affinityOptions.length(); i++ ) {
      affinity_options.length(affinity_options.length()+1);
      affinity_options[affinity_options.length()-1] = affinityOptions[i];
      CF::DataType dt = affinityOptions[i];
      RH_NL_DEBUG("ProgramProfile", "ProgramProfile - Affinity Property: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
    }

    // add nic allocations to affinity list 
    if ( nicAssignment != "" ) {
      std::string id = "nic";
      const redhawk::PropertyMap &tmap = redhawk::PropertyMap::cast( affinityOptions );
      if ( !tmap.contains(id) ) {
        // missing nic directive... add to map
        affinity_options.length(affinity_options.length()+1);
        affinity_options[affinity_options.length()-1].id = CORBA::string_dup("nic");  
        affinity_options[affinity_options.length()-1].value <<= nicAssignment.c_str(); 
      }
      else {
        std::string nic_iface = tmap[id].toString();
        if ( nic_iface != nicAssignment ) {
          // nic_iface is differnet add this 
          affinity_options.length(affinity_options.length()+1);
          affinity_options[affinity_options.length()-1].id = CORBA::string_dup("nic");  
          affinity_options[affinity_options.length()-1].value <<= nicAssignment.c_str(); 
        }
      }
      RH_NL_DEBUG("ProgramProfile", "ProgramProfile - NIC AFFINITY: pol/value "  <<  "nic"  << "/" << nicAssignment );
    }      

    if ( affinity_options.length() > 0 ) {
      options.length(options.length()+1);
      options[options.length()-1].id = CORBA::string_dup("AFFINITY"); 
      options[options.length()-1].value <<= affinity_options;
      RH_NL_DEBUG("ProgramProfile", "ProgramProfile - Extending options, adding Affinity Properties ...set length: " << affinity_options.length());
    }

    RH_NL_TRACE("ProgramProfile", "ProgramProfile - getOptions.... length: " << options.length());
    for ( uint32_t i=0; i < options.length(); i++ ) {
      RH_NL_TRACE("ProgramProfile", "ProgramProfile - getOptions id:"  <<  options[i].id << "/" <<  ossie::any_to_string( options[i].value )) ;
    }
    return options;
}


CF::Properties ProgramProfile::getExecParameters()
{
    return execParameters;
}

CF::Properties ProgramProfile::getPopulatedExecParameters()
{
    CF::Properties retval;
    unsigned int i;
    for ( i=0; i < execParameters.length(); i++ ) {
        RH_NL_TRACE("ProgramProfile", "getPopulatedExecParams i " << i << " id " << execParameters[i].id);
        // no empty exec params allowed..
        CORBA::TypeCode_var typecode = execParameters[i].value.type();
        RH_NL_TRACE("ProgramProfile", "getPopulatedExecParams i " << i << " typekind  " << typecode->kind() );
        if (typecode->kind() == CORBA::tk_null) continue;
          
        std::string v=ossie::any_to_string( execParameters[i].value );
        RH_NL_TRACE("ProgramProfile", "getPopulatedExecParams i : " << i << " value string " <<  v);
        if ( v.size() == 0 )  continue;
          
        // add to retval
        int l=retval.length();
        retval.length(l+1);
        retval[l] = execParameters[i];
    }

    return retval;
}

#if 0
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

#endif
