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


#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <algorithm>
#include <functional>
#include <set>
#include <list>
#include <unistd.h>

#include <boost/filesystem/path.hpp>

#include <ossie/CF/WellKnownProperties.h>
#include <ossie/FileStream.h>
#include <ossie/prop_helpers.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "RH_NamingContext.h"

namespace fs = boost::filesystem;
using namespace ossie;
using namespace std;

ScopedAllocations::ScopedAllocations(AllocationManager_impl& allocator):
    _allocator(allocator)
{
}

ScopedAllocations::~ScopedAllocations()
{
    try {
        deallocate();
    } catch (...) {
        // Destructors must not throw
    }
}

void ScopedAllocations::push_back(const std::string& allocationID)
{
    _allocations.push_back(allocationID);
}

template <class T>
void ScopedAllocations::transfer(T& dest)
{
    std::copy(_allocations.begin(), _allocations.end(), std::back_inserter(dest));
    _allocations.clear();
}

void ScopedAllocations::transfer(ScopedAllocations& dest)
{
    transfer(dest._allocations);
}

void ScopedAllocations::deallocate()
{
    if (!_allocations.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "Deallocating " << _allocations.size() << " allocations");
        _allocator.deallocate(_allocations.begin(), _allocations.end());
    }
}



/* Rotates a device list to put the device with the given identifier first
 */
static void rotateDeviceList(DeviceList& devices, const std::string& identifier)
{
    const DeviceList::iterator begin = devices.begin();
    for (DeviceList::iterator node = begin; node != devices.end(); ++node) {
        if ((*node)->identifier == identifier) {
            if (node != begin) {
                std::rotate(devices.begin(), node, devices.end());
            }
            return;
        }
    }
}

static std::vector<std::string> mergeProcessorDeps(const ossie::ImplementationInfo::List& implementations)
{
    // this function merges the overlap in processors between the different components that have been selected
    std::vector<std::string> processorDeps;
    for (ossie::ImplementationInfo::List::const_iterator impl = implementations.begin(); impl != implementations.end(); ++impl) {
        const std::vector<std::string>& implDeps = (*impl)->getProcessorDeps();
        if (!implDeps.empty()) {
            if (processorDeps.empty()) {
                // No prior processor dependencies, so overwrite
                processorDeps = implDeps;
            } else {
                std::vector<std::string> toremove;
                toremove.resize(0);
                for (std::vector<std::string>::iterator proc = processorDeps.begin(); proc != processorDeps.end(); ++proc) {
                    if (std::find(implDeps.begin(), implDeps.end(), *proc) == implDeps.end()) {
                        toremove.push_back(*proc);
                    }
                }
                for (std::vector<std::string>::iterator _rem = toremove.begin(); _rem != toremove.end(); ++_rem) {
                    std::vector<std::string>::iterator proc = std::find(processorDeps.begin(), processorDeps.end(), *_rem);
                    if (proc != processorDeps.end()) {
                        processorDeps.erase(proc);
                    }
                }
            }
        }
    }
    return processorDeps;
}

static std::vector<ossie::SPD::NameVersionPair> mergeOsDeps(const ossie::ImplementationInfo::List& implementations)
{
    // this function merges the overlap in operating systems between the different components that have been selected
    std::vector<ossie::SPD::NameVersionPair> osDeps;
    for (ossie::ImplementationInfo::List::const_iterator impl = implementations.begin(); impl != implementations.end(); ++impl) {
        const std::vector<ossie::SPD::NameVersionPair>& implDeps = (*impl)->getOsDeps();
        if (!implDeps.empty()) {
            if (osDeps.empty()) {
                // No prior OS dependencies, so overwrite
                osDeps = implDeps;
            } else {
                std::vector<ossie::SPD::NameVersionPair> toremove;
                toremove.resize(0);
                for (std::vector<ossie::SPD::NameVersionPair>::iterator pair = osDeps.begin(); pair != osDeps.end(); ++pair) {
                    if (std::find(implDeps.begin(), implDeps.end(), *pair) == implDeps.end()) {
                        toremove.push_back(*pair);
                    }
                }
                for (std::vector<ossie::SPD::NameVersionPair>::iterator _rem = toremove.begin(); _rem != toremove.end(); ++_rem) {
                    std::vector<ossie::SPD::NameVersionPair>::iterator pair = std::find(osDeps.begin(), osDeps.end(), *_rem);
                    if (pair != osDeps.end()) {
                        osDeps.erase(pair);
                    }
                }
            }
        }
    }
    return osDeps;
}

PREPARE_LOGGING(ApplicationFactory_impl);

void
ApplicationFactory_impl::ValidateFileLocation( CF::FileManager_ptr fileMgr, const std::string &profile_file)
{
    TRACE_ENTER(ApplicationFactory_impl)

    if (profile_file == "") {
        TRACE_EXIT(ApplicationFactory_impl)
        return;
    }

    // Verify file within the provided FileMgr
    LOG_TRACE(ApplicationFactory_impl, "Validating that profile " << profile_file << " exists");
    if (!fileMgr->exists (profile_file.c_str())) {
        string msg = "File ";
        msg += profile_file;
        msg += " does not exist.";
        throw CF::FileException (CF::CF_ENOENT, msg.c_str());
    }
}


void ApplicationFactory_impl::ValidateSoftPkgDep (CF::FileManager_ptr fileMgr, const std::string& sfw_profile )  {
  SoftPkg pkg;
  ValidateSPD(fileMgr, pkg, sfw_profile, false, false );
}

void ApplicationFactory_impl::ValidateSPD(CF::FileManager_ptr fileMgr, 
                                          const std::string& sfw_profile, 
                                          const bool require_prf, 
                                          const bool require_scd) {
  SoftPkg pkg;
  ValidateSPD(fileMgr, pkg, false, false );
}

void ApplicationFactory_impl::ValidateSPD(CF::FileManager_ptr fileMgr, 
                                          SoftPkg &spdParser, 
                                          const std::string& sfw_profile, 
                                          const bool require_prf, 
                                          const bool require_scd) {
    TRACE_ENTER(ApplicationFactory_impl)

    if ( sfw_profile == "" ) {
      LOG_WARN( ApplicationFactory_impl, "No Software Profile Provided.");
      throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "No software profile provided");
      TRACE_EXIT(ApplicationFactory_impl);
    }

    try {
        LOG_TRACE(ApplicationFactory_impl, "Validating SPD " << sfw_profile);
        ValidateFileLocation(fileMgr, sfw_profile);

        // check the filename ends with the extension given in the spec
        if ((strstr (sfw_profile.c_str(), ".spd.xml")) == NULL)
            { LOG_ERROR(ApplicationFactory_impl, "File " << sfw_profile << " should end with .spd.xml"); }
        LOG_TRACE(ApplicationFactory_impl, "validating " << sfw_profile);

        try {
          File_stream _spd(fileMgr, sfw_profile.c_str());
            spdParser.load( _spd,  sfw_profile.c_str() );
            _spd.close();
        } catch (ossie::parser_error& ex) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
            LOG_ERROR(ApplicationFactory_impl, "SPD file failed validation; parser error on file " << sfw_profile << parser_error_line << "The XML parser returned the following error: " << ex.what());
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
        } catch (CF::InvalidFileName ex) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to validate SPD due to invalid file name " << ex.msg);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
        } catch (CF::FileException ex) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to validate SPD due to file exception" << ex.msg);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
        } catch ( ... ) {
            LOG_ERROR(ApplicationFactory_impl, "Unexpected error validating PRF " << sfw_profile);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
        }

        //
        // validate each implementation
        //
        const ossie::SPD::Implementations& impls = spdParser.getImplementations();
        ossie::SPD::Implementations::const_iterator impl = impls.begin();
        for( ; impl != impls.end(); impl++ ) {

          
          // validate code file exists
          try {
            boost::filesystem::path implPath = boost::filesystem::path( spdParser.getSPDPath()) /  impl->getCodeFile();
            LOG_TRACE(ApplicationFactory_impl, "Validating Implmentation existance: " << implPath.string() );
            ValidateFileLocation( fileMgr, implPath.string().c_str() );
          } catch (CF::InvalidFileName ex) {
            LOG_ERROR(ApplicationFactory_impl, "Invalid Code File,  PROFILE:" << sfw_profile << "  CODE:" << impl->getCodeFile());
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
          } catch (CF::FileException ex) {
            LOG_ERROR(ApplicationFactory_impl, "Invalid Code File,  PROFILE:" << sfw_profile << "  CODE:" << impl->getCodeFile());
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
          } catch ( ... ) {
            LOG_ERROR(ApplicationFactory_impl, "Unexpected error validating PRF " << spdParser.getPRFFile());
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
          }

          // validate all the soft package dependencies....
          const ossie::SPD::SoftPkgDependencies& deps = impl->getSoftPkgDependencies();
          ossie::SPD::SoftPkgDependencies::const_iterator dep = deps.begin();
          for(; dep != deps.end(); dep++ ) {
            try {
              LOG_TRACE(ApplicationFactory_impl, "Validating Dependency: " << dep->localfile);
              ValidateSoftPkgDep( fileMgr, dep->localfile.c_str() );
            } catch (CF::InvalidFileName ex) {
              LOG_ERROR(ApplicationFactory_impl, "Invalid Code File,  PROFILE:" << sfw_profile << "  CODE:" << impl->getCodeFile());
              throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            } catch (CF::FileException ex) {
              LOG_ERROR(ApplicationFactory_impl, "Invalid Code File,  PROFILE:" << sfw_profile << "  CODE:" << impl->getCodeFile());
              throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            }
            
          }

        }
        
        // query SPD for PRF
        if (spdParser.getPRFFile() != 0) {
            LOG_TRACE(ApplicationFactory_impl, "validating " << spdParser.getPRFFile());
            try {
              ValidateFileLocation ( fileMgr, spdParser.getPRFFile ());

                // check the file name ends with the extension given in the spec
                if (spdParser.getPRFFile() && (strstr (spdParser.getPRFFile (), ".prf.xml")) == NULL) {
                    LOG_ERROR(ApplicationFactory_impl, "File " << spdParser.getPRFFile() << " should end in .prf.xml.");
                }

                LOG_TRACE(ApplicationFactory_impl, "Creating file stream")
                File_stream prfStream(fileMgr, spdParser.getPRFFile());
                LOG_TRACE(ApplicationFactory_impl, "Loading parser")
                Properties prfParser(prfStream);
                LOG_TRACE(ApplicationFactory_impl, "Closing stream")
                prfStream.close();
            } catch (ossie::parser_error& ex ) {
                std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
                LOG_ERROR(ApplicationFactory_impl, "Error validating PRF " << spdParser.getPRFFile() << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
            } catch (CF::InvalidFileName ex) {
              if ( require_prf ) {
                LOG_ERROR(ApplicationFactory_impl, "Failed to validate PRF due to invalid file name " << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
              }
            } catch (CF::FileException ex) {
              if ( require_prf ) {
                LOG_ERROR(ApplicationFactory_impl, "Failed to validate PRF due to file exception" << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
              }
            } catch ( ... ) {
                LOG_ERROR(ApplicationFactory_impl, "Unexpected error validating PRF " << spdParser.getPRFFile());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
            }
        } else {
            LOG_TRACE(ApplicationFactory_impl, "No PRF file to validate")
        }

        if (spdParser.getSCDFile() != 0) {
            try {
              // query SPD for SCD
              LOG_TRACE(ApplicationFactory_impl, "validating " << spdParser.getSCDFile());
              ValidateFileLocation ( fileMgr, spdParser.getSCDFile ());

              // Check the filename ends with  the extension given in the spec
              if ((strstr (spdParser.getSCDFile (), ".scd.xml")) == NULL)
                { LOG_ERROR(ApplicationFactory_impl, "File " << spdParser.getSCDFile() << " should end with .scd.xml."); }

                File_stream _scd(fileMgr, spdParser.getSCDFile());
                ComponentDescriptor scdParser (_scd);
                _scd.close();
            } catch (ossie::parser_error& ex) {
                std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
                LOG_ERROR(ApplicationFactory_impl, "SCD file failed validation; parser error on file " << spdParser.getSCDFile() << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
            } catch (CF::InvalidFileName ex) {
              if ( require_scd ){
                LOG_ERROR(ApplicationFactory_impl, "Failed to validate SCD due to invalid file name " << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
              }
            } catch (CF::FileException ex) {
              if ( require_scd ) {
                LOG_ERROR(ApplicationFactory_impl, "Failed to validate SCD due to file exception" << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
              }
            } catch ( ... ) {
                LOG_ERROR(ApplicationFactory_impl, "Unexpected error validating PRF " << spdParser.getSCDFile());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
            }
        } else if (spdParser.isScaCompliant() and require_scd ) {
            LOG_ERROR(ApplicationFactory_impl, "SCA compliant component is missing SCD file reference");
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "SCA compliant components require SCD file");
        } else {
            LOG_TRACE(ApplicationFactory_impl, "No SCD file to validate")
        }

    } catch (CF::InvalidFileName& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Failed to validate SPD due to " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch (CF::FileException& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Failed to validate SPD due to " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch (CF::DomainManager::ApplicationInstallationError& ex) {
        throw;
    } catch ( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "Unexpected error validating SPD " << sfw_profile);
        throw CF::DomainManager::ApplicationInstallationError ();
    }


}


ApplicationFactory_impl::ApplicationFactory_impl (const std::string& softwareProfile,
                                                  const std::string& domainName, 
                                                  DomainManager_impl* domainManager) :
    _softwareProfile(softwareProfile),
    _domainName(domainName),
    _domainManager(domainManager),
    _lastWaveformUniqueId(0)
{
    // Get the naming context from the domain
    _domainContext = RH_NamingContext::GetNamingContext( _domainName, !_domainManager->bindToDomain() );
    if (CORBA::is_nil(_domainContext)) {
        LOG_ERROR(ApplicationFactory_impl, "CosNaming::NamingContext::_narrow threw Unknown Exception");
        throw;
    }

    _dmnMgr = domainManager->_this();

    try {
        _fileMgr = _dmnMgr->fileMgr();
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while retrieving the File Manager";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while retrieving the File Manager";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "_dmnMgr->_fileMgr failed with Unknown Exception");
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "Could not get File Manager from Domain Manager");
    }

    try {

      LOG_INFO(ApplicationFactory_impl, "Installing application " << _softwareProfile.c_str());
      ValidateFileLocation ( _fileMgr, _softwareProfile );

      File_stream _sad(_fileMgr, _softwareProfile.c_str());
      _sadParser.load(_sad);
      _sad.close();
    } catch (const ossie::parser_error& ex) {
        ostringstream eout;
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        eout << "Failed to parse SAD file " << _softwareProfile << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what();
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( CF::InvalidFileName& ex ) {
        ostringstream eout;
        eout << "The following InvalidFileName exception occurred, profile: " << _softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, eout.str().c_str());
    } catch ( const CF::FileException& ex ) {
        ostringstream eout;
        eout << "The following FileException occurred: "<<ex.msg<<" While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        ostringstream eout;
        eout << "Parsing SAD failed with unknown exception;";
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    }

    // Makes sure all external port names are unique
    const std::vector<SoftwareAssembly::Port>& ports = _sadParser.getExternalPorts();
    std::vector<std::string> extPorts;
    for (std::vector<SoftwareAssembly::Port>::const_iterator port = ports.begin(); port != ports.end(); ++port) {
        // Gets name to use
        std::string extName;
        if (port->externalname != "") {
            extName = port->externalname;
        } else {
            extName = port->identifier;
        }
        // Check for duplicate
        if (std::find(extPorts.begin(), extPorts.end(), extName) == extPorts.end()) {
            extPorts.push_back(extName);
        } else {
            ostringstream eout;
            eout << "Duplicate External Port name: " << extName;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    // Gets the assembly controller software profile by looping through each
    // component instantiation to find a matching ID to the AC's
    std::string assemblyControllerId = _sadParser.getAssemblyControllerRefId();
    SoftPkg ac_spd;
    CORBA::String_var ac_profile = "";
    bool ac_found = false;
    std::vector<ComponentPlacement> components = _sadParser.getAllComponents();
    for (std::vector<ComponentPlacement>::const_iterator comp = components.begin();
            comp != components.end(); ++comp) {
      SoftPkg comp_pkg;
      std::string p_name;
      try {
        if ( _sadParser.getSPDById(comp->getFileRefId())) {
            p_name = _sadParser.getSPDById(comp->getFileRefId());
            LOG_DEBUG(ApplicationFactory_impl, "Validating...  COMP profile: " << p_name);
            ValidateSPD( _fileMgr, comp_pkg, p_name.c_str() ) ;
        }
        else {
          LOG_ERROR(ApplicationFactory_impl, "installApplication: invalid  componentfileref: " << comp->getFileRefId() );
          throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "installApplication: invalid  componentfileref"); 
        }
      } catch (CF::FileException& ex) {
        LOG_ERROR(ApplicationFactory_impl, "installApplication: While validating the SAD profile: " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
      } catch( CF::InvalidFileName& ex ) {
        std::ostringstream eout;
        eout << "Invalid file name: " << p_name;
        LOG_ERROR(ApplicationFactory_impl, "installApplication: Invalid file name: " << p_name);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, eout.str().c_str());
      } catch (CF::DomainManager::ApplicationInstallationError& e) {
        LOG_TRACE(ApplicationFactory_impl, "rethrowing ApplicationInstallationError" << e.msg);
        throw;
      } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while restoring the application factories";
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, eout.str().c_str());
      } catch (...) {
        LOG_ERROR(ApplicationFactory_impl, "unexpected exception occurred while installing application");
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, "unknown exception");
      }

      if ( !ac_found ) {
        std::vector<ComponentInstantiation> compInstantiations = comp->instantiations;
        for (std::vector<ComponentInstantiation>::const_iterator compInst = compInstantiations.begin();
             compInst != compInstantiations.end(); ++compInst){
          if (assemblyControllerId == compInst->instantiationId) {
            ac_spd = comp_pkg;
            ac_profile = _sadParser.getSPDById(comp->getFileRefId());
            ac_found = true;
            break;
          }
        }
      }
    }

    // Gets the assembly controllers properties
    Properties prf;
    if (ac_found) {
        if ( ac_spd.getPRFFile() ) {
          std::string prf_file(ac_spd.getPRFFile());
            try {
              File_stream _prf(_fileMgr, prf_file.c_str());
                prf.load(_prf);
                _prf.close();
            } catch(ossie::parser_error& ex ) {
              std::ostringstream os;
              std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
              os << "Invalid PRF file: " << prf_file << ". " << parser_error_line << "The XML parser returned the following error: " << ex.what();
              LOG_ERROR(ApplicationFactory_impl, os.str() );
              throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, os.str().c_str());
            } catch( ... ) {
                // Errors are reported at create time
            }
        }
    }

    // Makes sure all external property names are unique
    const std::vector<SoftwareAssembly::Property>& properties = _sadParser.getExternalProperties();
    std::vector<std::string> extProps;
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = properties.begin(); prop != properties.end(); ++prop) {
        // Gets name to use
        std::string extName;
        if (prop->externalpropid != "") {
            extName = prop->externalpropid;
        } else {
            extName = prop->propid;
        }
        // Check for duplicate
        if (std::find(extProps.begin(), extProps.end(), extName) == extProps.end()) {
            extProps.push_back(extName);
        } else {
            ostringstream eout;
            eout << "Duplicate External Property name: " << extName;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    // Make sure AC prop ID's aren't in conflict with external ones
    const std::vector<const Property*>& acProps = prf.getProperties();
    for (unsigned int i = 0; i < acProps.size(); ++i) {
        // Check for duplicate
        if (std::find(extProps.begin(), extProps.end(), acProps[i]->getID()) == extProps.end()) {
            extProps.push_back(acProps[i]->getID());
        } else {
            ostringstream eout;
            eout << "Assembly controller property in use as External Property: " << acProps[i]->getID();
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    _name = _sadParser.getName();
    _identifier = _sadParser.getID();
}

ApplicationFactory_impl::~ApplicationFactory_impl ()
{
  try {
    //
    // remove the naming context assocated with the factory that generates new
    // naming contexts for each application.
    //
    if ( _domainManager && _domainManager->bindToDomain() ) _domainContext->destroy();
  }
  catch(...)
    {};

}

/*
 * Check to make sure assemblyController was initialized if it was SCA compliant
 */
void createHelper::_checkAssemblyController(
    CF::Resource_ptr      assemblyController,
    ossie::ComponentInfo* assemblyControllerComponent) const
{
    if (CORBA::is_nil(assemblyController)) {
        if ((assemblyControllerComponent==NULL) || 
            (assemblyControllerComponent->isScaCompliant())
           ) {
        LOG_DEBUG(ApplicationFactory_impl, "assembly controller is not Sca Compliant or has not been assigned");
        throw (CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET, 
                    "assembly controller is not Sca Compliant or has not been assigned"));
        }
    }
}

void createHelper::_connectComponents(std::vector<ConnectionNode>& connections){
    try{
        connectComponents(connections, _baseNamingContext);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl,
        "Connecting components failed (unclear where this occurred)",
        CF::ApplicationFactory::CreateApplicationError(
            CF::CF_EINVAL, 
            "Connecting components failed (unclear where this occurred)"));
}

void createHelper::_configureComponents()
{
    try{
        configureComponents();
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl, 
        "Configure on component failed (unclear where in the process this occurred)",
        CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Configure of component failed (unclear where in the process this occurred)"))
}

void createHelper::assignRemainingComponentsToDevices(const std::string &appIdentifier)
{
    PlacementList::iterator componentIter;
    for (componentIter  = _requiredComponents.begin(); 
         componentIter != _requiredComponents.end(); 
         componentIter++)
    {
        if (!(*componentIter)->isAssignedToDevice()) {
            allocateComponent(*componentIter, std::string(), _appUsedDevs, appIdentifier);
        }
    }
}

void createHelper::_assignComponentsUsingDAS(const DeviceAssignmentMap& deviceAssignments, const std::string &appIdentifier)
{
    LOG_TRACE(ApplicationFactory_impl, "Assigning " << deviceAssignments.size() 
              << " component(s) based on DeviceAssignmentSequence");

    for (DeviceAssignmentMap::const_iterator ii = deviceAssignments.begin(); ii != deviceAssignments.end(); ++ii) {
        const std::string& componentId = ii->first;
        const std::string& assignedDeviceId = ii->second;
        LOG_TRACE(ApplicationFactory_impl, "Component " << componentId << " is assigned to device " << assignedDeviceId);
        ossie::ComponentInfo* component = findComponentByInstantiationId(componentId);

        if (!component) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to create application; "
                      << "unknown component " << componentId 
                      << " in user assignment (DAS)");
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = componentId.c_str();
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }
        allocateComponent(component, assignedDeviceId, _appUsedDevs, appIdentifier);
    }
}

void createHelper::_resolveImplementations(PlacementList::iterator comp, PlacementList& compList, std::vector<ossie::ImplementationInfo::List> &res_vec)
{
    if (comp == compList.end()) {
        return;
    }
    ossie::ImplementationInfo::List comp_imps;
    std::vector<ossie::ImplementationInfo::List> tmp_res_vec = res_vec;
    (*comp)->getImplementations(comp_imps);
    unsigned int old_res_vec_size = res_vec.size();
    if (old_res_vec_size == 0) {
        res_vec.resize(comp_imps.size());
        for (unsigned int ii=0; ii<comp_imps.size(); ii++) {
            res_vec[ii].resize(1);
            res_vec[ii][0] = comp_imps[ii];
        }
    } else {
        res_vec.resize(old_res_vec_size * comp_imps.size());
        for (unsigned int i=0; i<old_res_vec_size; i++) {
            for (unsigned int ii=0; ii<comp_imps.size(); ii++) {
                unsigned int res_vec_idx = i*comp_imps.size()+ii;
                res_vec[res_vec_idx] = tmp_res_vec[i];
                res_vec[res_vec_idx].insert(res_vec[res_vec_idx].begin(),comp_imps[ii]);
            }
        }
    }
    this->_resolveImplementations(++comp, compList, res_vec);
    return;
}

void createHelper::_removeUnmatchedImplementations(std::vector<ossie::ImplementationInfo::List> &res_vec)
{
    std::vector<ossie::ImplementationInfo::List>::iterator impl_list = res_vec.begin();
    while (impl_list != res_vec.end()) {
        std::vector<ossie::ImplementationInfo::List>::iterator old_impl_list = impl_list;
        ossie::ImplementationInfo::List::iterator impl = (*impl_list).begin();
        std::vector<ossie::SPD::NameVersionPair> reference_pair = (*impl)->getOsDeps();
        std::vector<std::string> reference_procs = (*impl)->getProcessorDeps();
        bool os_init_to_zero = (reference_pair.size()==0);
        bool proc_init_to_zero = (reference_procs.size()==0);
        impl++;
        bool match = true;
        while (impl != (*impl_list).end()) {
            std::vector<ossie::SPD::NameVersionPair> pair = (*impl)->getOsDeps();
            std::vector<std::string> procs = (*impl)->getProcessorDeps();
            bool os_must_match = false;
            bool proc_must_match = false;
            if (os_init_to_zero)
                os_must_match = false;
            if (proc_init_to_zero)
                proc_must_match = false;
            if ((reference_pair.size() != 0) and (pair.size() != 0)) {os_must_match = true;}
            if ((reference_procs.size() != 0) and (procs.size() != 0)) {proc_must_match = true;}
            // if os must match (because both lists are non-zero length), check that at least one of the sets matches
            if (os_must_match) {
                bool at_least_one_match = false;
                for (std::vector<ossie::SPD::NameVersionPair>::iterator ref=reference_pair.begin(); ref<reference_pair.end(); ref++) {
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            at_least_one_match = true;
                            break;
                        }
                    }
                }
                if (!at_least_one_match) {match = false;break;}
            }
            // if proc must match (because both lists are non-zero length), check that at least one of the sets matches
            if (proc_must_match) {
                bool at_least_one_match = false;
                for (std::vector<std::string>::iterator ref=reference_procs.begin(); ref<reference_procs.end(); ref++) {
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            at_least_one_match = true;
                        }
                    }
                }
                if (!at_least_one_match) {match = false;break;}
            }
            // reduce the number of os that can be used as a reference to the overlapping set
            if (reference_pair.size()>pair.size()) {
                for (std::vector<ossie::SPD::NameVersionPair>::iterator ref=reference_pair.begin(); ref<reference_pair.end(); ref++) {
                    bool found_match = false;
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            found_match = true;
                        }
                    }
                    if (not found_match) {
                        reference_pair.erase(ref);
                    }
                }
            }
            // reduce the number of procs that can be used as a reference to the overlapping set
            if (reference_procs.size()>procs.size()) {
                for (std::vector<std::string>::iterator ref=reference_procs.begin(); ref<reference_procs.end(); ref++) {
                    bool found_match = false;
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            found_match = true;
                        }
                    }
                    if (not found_match) {
                        reference_procs.erase(ref);
                    }
                }
            }
            // if the initial entity did not have an os, add it if the current one holds an os requirement
            if (os_init_to_zero) {
                if (pair.size() != 0) {
                    os_init_to_zero = false;
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        reference_pair.push_back((*cur));
                    }
                }
            }
            // if the initial entity did not have a proc, add it if the current one holds a proc requirement
            if (proc_init_to_zero) {
                if (procs.size() != 0) {
                    proc_init_to_zero = false;
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        reference_procs.push_back((*cur));
                    }
                }
            }
            impl++;
        }
        if (not match) {
            (*impl_list).erase(impl);
        }
        impl_list++;
    }
    return;
}

void createHelper::_consolidateAllocations(const ossie::ImplementationInfo::List& impls, CF::Properties& allocs)
{
    allocs.length(0);
    for (ossie::ImplementationInfo::List::const_iterator impl= impls.begin(); impl != impls.end(); ++impl) {
        const std::vector<SPD::PropertyRef>& deps = (*impl)->getDependencyProperties();
        for (std::vector<SPD::PropertyRef>::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
          ossie::ComponentProperty *prop = dep->property.get();
          if (dynamic_cast<const SimplePropertyRef*>( prop ) != NULL) {
                const SimplePropertyRef* dependency = dynamic_cast<const SimplePropertyRef*>(prop);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const SimpleSequencePropertyRef*>(prop) != NULL) {
                const SimpleSequencePropertyRef* dependency = dynamic_cast<const SimpleSequencePropertyRef*>(prop);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const ossie::StructPropertyRef*>(prop) != NULL) {
                const ossie::StructPropertyRef* dependency = dynamic_cast<const ossie::StructPropertyRef*>(prop);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const ossie::StructSequencePropertyRef*>(prop) != NULL) {
                const ossie::StructSequencePropertyRef* dependency = dynamic_cast<const ossie::StructSequencePropertyRef*>(prop);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            }
        }
    }
}

void createHelper::_handleHostCollocation(const std::string &appIdentifier)
{
    const std::vector<SoftwareAssembly::HostCollocation>& hostCollocations =
        _appFact._sadParser.getHostCollocations();
    LOG_TRACE(ApplicationFactory_impl,
              "Assigning " << hostCollocations.size()
                    << " collocated groups of components");

    for (unsigned int ii = 0; ii < hostCollocations.size(); ++ii) {
        _placeHostCollocation(hostCollocations[ii], appIdentifier);
    }
}

void createHelper::_placeHostCollocation(const SoftwareAssembly::HostCollocation& collocation, const std::string &appIdentifier)
{
    LOG_TRACE(ApplicationFactory_impl,
              "-- Begin placment for Collocation " <<
              collocation.getName() << " " <<
              collocation.getID());

    PlacementList placingComponents;
    std::vector<ossie::ImplementationInfo::List> res_vec;

    // Some components may have been placed by a user DAS; keep a
    // list of those that still need to be assigned to a device.
    //PlacementList placingComponents;

    // Keep track of devices to which some of the components have
    // been assigned.
    DeviceIDList assignedDevices;

    const std::vector<ComponentPlacement>& collocatedComponents =
        collocation.getComponents();

    _getComponentsToPlace(collocatedComponents,
                          assignedDevices,
                          placingComponents);

    // create every combination of implementations for the components in the set
    // for each combination:
    //  consolidate allocations
    //  attempt allocation
    //  if the allocation succeeds, break the loop
    this->_resolveImplementations(placingComponents.begin(), placingComponents, res_vec);
    this->_removeUnmatchedImplementations(res_vec);

    // Get the executable devices for the domain; if there were any devices
    // assigned, filter out all other devices
    ossie::DeviceList deploymentDevices = _executableDevices;
    if (!assignedDevices.empty()) {
        for (ossie::DeviceList::iterator node = deploymentDevices.begin(); node != deploymentDevices.end(); ++node) {
            if (std::find(assignedDevices.begin(), assignedDevices.end(), (*node)->identifier) == assignedDevices.end()) {
                node = deploymentDevices.erase(node);
            }
        }
    }
    

    for (size_t index = 0; index < res_vec.size(); ++index) {
        // Merge processor and OS dependencies from all implementations
        std::vector<std::string> processorDeps = mergeProcessorDeps(res_vec[index]);
        std::vector<ossie::SPD::NameVersionPair> osDeps = mergeOsDeps(res_vec[index]);

        // Consolidate the allocation properties into a single list
        CF::Properties allocationProperties;
        this->_consolidateAllocations(res_vec[index], allocationProperties);

        const std::string requestid = ossie::generateUUID();
        ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid, allocationProperties, deploymentDevices, appIdentifier, processorDeps, osDeps);
        if (!response.first.empty()) {
            // Ensure that all capacities get cleaned up
            this->_allocations.push_back(response.first);

            // Convert from response back into a device node
            boost::shared_ptr<ossie::DeviceNode>& node = response.second;
            const std::string& deviceId = node->identifier;

            PlacementList::iterator comp = placingComponents.begin();
            ossie::ImplementationInfo::List::iterator impl = res_vec[index].end()-1;
            DeviceAssignmentList      collocAssignedDevs;
            collocAssignedDevs.resize(placingComponents.size());
            for (unsigned int i=0; i<collocAssignedDevs.size(); i++,comp++,impl--) {
                collocAssignedDevs[i].device = CF::Device::_duplicate(node->device);
                collocAssignedDevs[i].deviceAssignment.assignedDeviceId = CORBA::string_dup(deviceId.c_str());
                (*comp)->setSelectedImplementation(*impl);
                if (!resolveSoftpkgDependencies(*impl, *node)) {
                    LOG_TRACE(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                              << (*comp)->getIdentifier() << " implementation " << (*impl)->getId());
                    continue;
                }
                (*comp)->setAssignedDevice(node);
                collocAssignedDevs[i].deviceAssignment.componentId = CORBA::string_dup((*comp)->getIdentifier());
            }
            
            // Move the device to the front of the list
            rotateDeviceList(_executableDevices, deviceId);

            _appUsedDevs.insert(_appUsedDevs.end(),
                                collocAssignedDevs.begin(),
                                collocAssignedDevs.end());
            LOG_TRACE(ApplicationFactory_impl, "-- Completed placement for Collocation ID:" << collocation.id << " Components Placed: " << collocatedComponents.size());
            return;
        }
    }

    std::ostringstream eout;
    eout << "Could not collocate components for collocation NAME: " << collocation.getName() << "  ID:" << collocation.id;
    LOG_ERROR(ApplicationFactory_impl, eout.str());
    throw CF::ApplicationFactory::CreateApplicationRequestError();
}

void createHelper::_getComponentsToPlace(
    const std::vector<ComponentPlacement>& collocatedComponents,
    DeviceIDList&                          assignedDevices,
    PlacementList&                         placingComponents)
{
    std::vector<ComponentPlacement>::const_iterator placement =
        collocatedComponents.begin();

    for (; placement != collocatedComponents.end(); ++placement) {
        ComponentInstantiation instantiation =
            (placement->getInstantiations()).at(0);
        ossie::ComponentInfo* component =
            findComponentByInstantiationId(instantiation.getID());

        if (!component) {
            ostringstream eout;
            eout << "failed to create application; unable to recover component Id (error parsing the SAD file "<<_appFact._softwareProfile<<")";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(
                CF::CF_EAGAIN,
                eout.str().c_str());
        }
        LOG_TRACE(ApplicationFactory_impl,
                  "Collocated component " <<
                        component->getInstantiationIdentifier());

        if (component->isAssignedToDevice()) {
            // This component is already assigned to a device; for collocating
            // other components, the pre-assigned devices are used in the order
            // they are encountered.
            LOG_TRACE(ApplicationFactory_impl,
                      "Already assigned to device " <<
                      component->getAssignedDeviceId());
            assignedDevices.push_back( component->getAssignedDeviceId() );
        } else {
            // This component needs to be assigned to a device.
            placingComponents.push_back(component);
        }
    }
}

void createHelper::_handleUsesDevices(const std::string& appName)
{
    // Gets all uses device info from the SAD file
    const UsesDeviceInfo::List& usesDevices = _appInfo.getUsesDevices();
    LOG_TRACE(ApplicationFactory_impl, "Application has " << usesDevices.size() << " usesdevice dependencies");
    const CF::Properties& appProperties = _appInfo.getACProperties();
    // The device assignments for SAD-level usesdevices are never stored
    DeviceAssignmentList assignedDevices;
    if (!allocateUsesDevices(appName, usesDevices, appProperties, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the application
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (UsesDeviceInfo::List::const_iterator uses = usesDevices.begin(); uses != usesDevices.end(); ++uses) {
            if ((*uses)->getAssignedDeviceId().empty()) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << (*uses)->getId();
            }
        }
        eout << "for application '" << appName << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }
    for (DeviceAssignmentList::iterator dev=assignedDevices.begin(); dev!=assignedDevices.end(); dev++) {
        dev->deviceAssignment.componentId = getAssemblyController()->getIdentifier();
    }
    _appUsedDevs.insert(_appUsedDevs.end(), assignedDevices.begin(), assignedDevices.end());
}

void createHelper::setUpExternalPorts(Application_impl* application)
{
    const std::vector<SoftwareAssembly::Port>& ports =
        _appInfo.getExternalPorts();
    LOG_TRACE(ApplicationFactory_impl,
              "Mapping " << ports.size() << " external port(s)");
    std::vector<SoftwareAssembly::Port>::const_iterator port;

    for (port = ports.begin(); port != ports.end(); ++port) {
        LOG_TRACE(ApplicationFactory_impl,
                  "Port component: " << port->componentrefid
                        << " Port identifier: " << port->identifier);

        // Get the component from the instantiation identifier.
        CORBA::Object_var obj =
            lookupComponentByInstantiationId(port->componentrefid);
        if (CORBA::is_nil(obj)) {
            LOG_ERROR(ApplicationFactory_impl,
                      "Invalid componentinstantiationref ("
                            <<port->componentrefid
                            <<") given for an external port ");
            throw(CF::ApplicationFactory::CreateApplicationError(
                CF::CF_NOTSET,
                "Invalid componentinstantiationref given for external port"));
        }

        if (port->type == SoftwareAssembly::Port::SUPPORTEDIDENTIFIER) {
            if (!obj->_is_a(port->identifier.c_str())) {
                LOG_ERROR(
                    ApplicationFactory_impl,
                    "Component does not support requested interface: "
                        << port->identifier);
                throw(CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Component does not support requested interface"));
            }
        } else {
            // Must be either "usesidentifier" or "providesidentifier",
            // which are equivalent unless you want to be extra
            // pedantic and check how the port is described in the
            // component's SCD.

            CF::PortSupplier_var portSupplier =
                ossie::corba::_narrowSafe<CF::PortSupplier> (obj);

            // Try to look up the port.
            try {
                obj = portSupplier->getPort(port->identifier.c_str());
            } CATCH_THROW_LOG_ERROR(
                ApplicationFactory_impl,
                "Invalid port id",
                CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Invalid port identifier"))
        }

        // Add it to the list of external ports on the application object.
        if (port->externalname == ""){
            application->addExternalPort(port->identifier, obj);
        } else {
            application->addExternalPort(port->externalname, obj);
        }
    }
}

void createHelper::setUpExternalProperties(Application_impl* application)
{
    const std::vector<SoftwareAssembly::Property>& props = _appInfo.getExternalProperties();
    LOG_TRACE(ApplicationFactory_impl, "Mapping " << props.size() << " external property(ies)");
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
        LOG_TRACE(ApplicationFactory_impl, "Property component: " << prop->comprefid << " Property identifier: " << prop->propid);

        // Verify internal property
        ComponentInfo *tmp = findComponentByInstantiationId(prop->comprefid);
        if (tmp == 0) {
            LOG_ERROR(ApplicationFactory_impl, "Unable to find component for comprefid " << prop->comprefid);
            throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unable to find component for given comprefid"));
        }
        const std::vector<const Property*>& props = tmp->prf.getProperties();
        bool foundProp = false;
        for (unsigned int i = 0; i < props.size(); ++i) {
            if (props[i]->getID() == prop->propid){
                foundProp = true;
            }
        }
        if (!foundProp){
            LOG_ERROR(ApplicationFactory_impl, "Attempting to promote property: '" <<
                    prop->propid << "' that does not exist in component: '" << prop->comprefid << "'");
            throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET,
                    "Attempting to promote property that does not exist in component"));
        }

        // Get the component from the compref identifier.
        CF::Resource_var comp = lookupComponentByInstantiationId(prop->comprefid);
        if (CORBA::is_nil(comp)) {
            LOG_ERROR(ApplicationFactory_impl, "Invalid comprefid (" << prop->comprefid << ") given for an external property");
            throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Invalid comprefid given for external property"));
        }

        if (prop->externalpropid == "") {
            application->addExternalProperty(prop->propid,
                                             prop->propid,
                                             comp);
        } else {
            application->addExternalProperty(prop->propid,
                                             prop->externalpropid,
                                             comp);
        }
    }
}

/* Creates and instance of the application.
 *  - Assigns components to devices
 *      - First based on user-provided DAS if one is passed in
 *        (deviceAssignments)
 *      - Then based on property matching and allocation matching
 *  - Attempts to honor host collocation
 *  @param name user-friendly name of the application to be instantiated
 *  @param initConfiguration properties that can override those from the SAD
 *  @param deviceAssignments optional user-provided component-to-device
 *         assignments
 */
CF::Application_ptr ApplicationFactory_impl::create (
    const char* name,
    const CF::Properties& initConfiguration,
    const CF::DeviceAssignmentSequence& deviceAssignments)
throw (CORBA::SystemException, CF::ApplicationFactory::CreateApplicationError,
        CF::ApplicationFactory::CreateApplicationRequestError,
        CF::ApplicationFactory::CreateApplicationInsufficientCapacityError,
        CF::ApplicationFactory::InvalidInitConfiguration)
{
    TRACE_ENTER(ApplicationFactory_impl);
    LOG_TRACE(ApplicationFactory_impl, "Creating application " << name);

    // must declare these here, so we can pass to the createHelper instance
    string _waveform_context_name;
    string base_naming_context;
    CosNaming::NamingContext_var _waveformContext;

    ///////////////////////////////////////////////////
    // Establish new naming context for waveform
    LOG_TRACE(ApplicationFactory_impl, "Establishing waveform naming context");
    try {
        // VERY IMPORTANT: we must first lock the operations in this try block
        //    in order to prevent a naming context collision due to multiple create calls
        boost::mutex::scoped_lock lock(_pendingCreateLock);

        // get new naming context name
        _waveform_context_name = getWaveformContextName(name);
        base_naming_context = getBaseWaveformContext(_waveform_context_name);

        _waveformContext = CosNaming::NamingContext::_nil();

        // create the new naming context
        CosNaming::Name WaveformContextName;
        WaveformContextName.length(1);
        WaveformContextName[0].id = _waveform_context_name.c_str();

        LOG_TRACE(ApplicationFactory_impl, "Binding new context " << _waveform_context_name.c_str());
        try {
            _waveformContext = _domainContext->bind_new_context(WaveformContextName);
        } catch( ... ) {
            // just in case it bound, unbind and error
            // roughly the same code as _cleanupNewContext
            try {
                _domainContext->unbind(WaveformContextName);
            } catch ( ... ) {
            }
            LOG_ERROR(ApplicationFactory_impl, "bind_new_context threw Unknown Exception");
            throw;
        }

    } catch(...){
    }

    // Convert the device assignments into a map for easier lookup
    std::map<std::string,std::string> deviceAssignmentMap;
    for (size_t index = 0; index < deviceAssignments.length(); ++index) {
        const std::string componentId(deviceAssignments[index].componentId);
        const std::string assignedDeviceId(deviceAssignments[index].assignedDeviceId);
        deviceAssignmentMap.insert(std::make_pair(componentId, assignedDeviceId));
    }

    // now use the createHelper class to actually run 'create'
    // - createHelper is needed to allow concurrent calls to 'create' without
    //   each instance stomping on the others
    LOG_TRACE(ApplicationFactory_impl, "Creating new createHelper class.");
    createHelper new_createhelper(*this, _waveform_context_name, base_naming_context, _waveformContext, _domainContext);

    // now actually perform the create operation
    LOG_TRACE(ApplicationFactory_impl, "Performing 'create' function.");
    CF::Application_ptr new_app = new_createhelper.create(name, initConfiguration, deviceAssignmentMap);
    // return the new Application
    TRACE_EXIT(ApplicationFactory_impl);
    return new_app;
}

CF::Application_ptr createHelper::create (
    const char*                         name,
    const CF::Properties&               initConfiguration,
    const DeviceAssignmentMap& deviceAssignments)
throw (CORBA::SystemException,
       CF::ApplicationFactory::CreateApplicationError,
       CF::ApplicationFactory::CreateApplicationRequestError,
       CF::ApplicationFactory::InvalidInitConfiguration)
{
    TRACE_ENTER(ApplicationFactory_impl);
    
    bool aware_application = true;
    
    CF::Properties modifiedInitConfiguration;

    try {
        ///////////////////////////////////////////////////////////////////
        // Check to see if this is an aware application and 
        //  check to see if a different GPP reservation setting is defined
        const std::string aware_app_property_id(ExtendedCF::WKP::AWARE_APPLICATION);
        for (unsigned int initCount = 0; initCount < initConfiguration.length(); initCount++) {
            if (std::string(initConfiguration[initCount].id) == aware_app_property_id) {
                initConfiguration[initCount].value >>= aware_application;
                modifiedInitConfiguration.length(initConfiguration.length()-1);
                for (unsigned int rem_idx=0; rem_idx<initConfiguration.length()-1; rem_idx++) {
                    unsigned int idx_mod = 0;
                    if (rem_idx == initCount)
                        idx_mod = 1;
                    modifiedInitConfiguration[rem_idx] = initConfiguration[rem_idx+idx_mod];
                    //modifiedInitConfiguration[rem_idx].id = initConfiguration[rem_idx+idx_mod].id;
                    //modifiedInitConfiguration[rem_idx].value = initConfiguration[rem_idx+idx_mod].value;
                }
            }
        }
        
        if (modifiedInitConfiguration.length() == 0) {
            modifiedInitConfiguration = initConfiguration;
        }

        const std::string specialized_reservation("SPECIALIZED_CPU_RESERVATION");
        for (unsigned int initCount = 0; initCount < modifiedInitConfiguration.length(); initCount++) {
            if (std::string(modifiedInitConfiguration[initCount].id) == specialized_reservation) {
                CF::Properties *reservations;
                if (modifiedInitConfiguration[initCount].value >>= reservations) {
                    for (unsigned int rem_idx=0; rem_idx<reservations->length(); rem_idx++) {
                        double value = 0;
                        std::string component_id((*reservations)[rem_idx].id);
                        if ((*reservations)[rem_idx].value >>= value) {
                            specialized_reservations[component_id] = value;
                        }
                    }
                } else {
                    // the value of the any is of the wrong type
                }
                for (unsigned int rem_idx=initCount; rem_idx<modifiedInitConfiguration.length()-1; rem_idx++) {
                    modifiedInitConfiguration[rem_idx] = modifiedInitConfiguration[rem_idx+1];
                }
                modifiedInitConfiguration.length(modifiedInitConfiguration.length()-1);
            }
        }

        // Get a list of all device currently in the domain
        _registeredDevices = _appFact._domainManager->getRegisteredDevices();
        _executableDevices.clear();
        for (DeviceList::iterator iter = _registeredDevices.begin(); iter != _registeredDevices.end(); ++iter) {
            if ((*iter)->isExecutable) {
                _executableDevices.push_back(*iter);
            }
        }

        // Fail immediately if there are no available devices to execute components
        if (_executableDevices.empty()) {
            const char* message = "Domain has no executable devices (GPPs) to run components";
            LOG_WARN(ApplicationFactory_impl, message);
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENODEV, message);
        }

        const std::string lastExecutableDevice = _appFact._domainManager->getLastDeviceUsedForDeployment();
        if (!lastExecutableDevice.empty()) {
            LOG_TRACE(ApplicationFactory_impl, "Placing device " << lastExecutableDevice
                      << " first in deployment list");
            rotateDeviceList(_executableDevices, lastExecutableDevice);
        }

        //////////////////////////////////////////////////
        // Load the components to instantiate from the SAD
        getRequiredComponents();

        ossie::ComponentInfo* assemblyControllerComponent = getAssemblyController();
        if (assemblyControllerComponent) {
            overrideProperties(modifiedInitConfiguration, assemblyControllerComponent);
        }

        //////////////////////////////////////////////////
        // Store information about this application
        _appInfo.populateApplicationInfo(_appFact._sadParser);
        for (unsigned int i = 0; i < _requiredComponents.size(); ++i) {
            ComponentInfo *comp = _requiredComponents[i];
            if (comp->isAssemblyController()) {
                _appInfo.setACProperties(comp->getConfigureProperties());
            }
            _appInfo.addComponent(comp);
        }

        overrideExternalProperties(modifiedInitConfiguration);

        ////////////////////////////////////////////////
        // Assign components to devices
        ////////////////////////////////////////////////

        /*
         * _appUsedDevs and appCapacityTable represent all the allocations
         * and assigned made during applicaiton deployment. It provides the
         * "context" for the deployment.  This context pattern will be
         * applied again when collocation requests are fullfilled.  There 2
         * container are used to deploy the waveform, and also to "cleanup"
         *  if deployment fails
         */

        // reset list of devices that were used during component
        // allocation/placement process for an application
        _appUsedDevs.resize(0);

        // Start with a empty set of allocation properties, used to keep
        // track of device capacity allocations. If this is not cleared
        // each time, deallocation may start occuring multiple times,
        // resulting in incorrect capacities.
        //_appCapacityTable.clear();

        // Allocate any usesdevice capacities specified in the SAD file
        _handleUsesDevices(name);

        // Give the application a unique identifier of the form 
        // "softwareassemblyid:ApplicationName", where the application 
        // name includes the serial number generated for the naming context
        // (e.g. "Application_1").
        std::string appIdentifier = 
            _appFact._identifier + ":" + _waveformContextName;

        // First, assign components to devices based on the caller supplied
        // DAS.
        _assignComponentsUsingDAS(deviceAssignments, appIdentifier);

        // Second, attempt to honor host collocation.
        _handleHostCollocation(appIdentifier);

        assignRemainingComponentsToDevices(appIdentifier);

        ////////////////////////////////////////////////
        // Create the Application servant

        // Manage the Application servant with an auto_ptr in case 
        // something throws an exception.
        _application = new Application_impl(appIdentifier,
                                            name, 
                                            _appFact._softwareProfile, 
                                            _appFact._domainManager, 
                                            _waveformContextName, 
                                            _waveformContext,
                                            aware_application,
                                            _domainContext);

        // Activate the new Application servant
        PortableServer::ObjectId_var oid = Application_impl::Activate(_application);

        std::vector<ConnectionNode> connections;
        std::vector<std::string> allocationIDs;

        CF::ApplicationRegistrar_var app_reg = _application->appReg();
        loadAndExecuteComponents(app_reg);
        waitForComponentRegistration();
        initializeComponents();

        // Check that the assembly controller is valid
        CF::Resource_var assemblyController;
        if (assemblyControllerComponent) {
            assemblyController = assemblyControllerComponent->getResourcePtr();
        }
        _checkAssemblyController(assemblyController, assemblyControllerComponent);

        _connectComponents(connections);
        _configureComponents();

        setUpExternalPorts(_application);
        setUpExternalProperties(_application);

        ////////////////////////////////////////////////
        // Create the application
        //
        // We are assuming that all components and their resources are 
        // collocated. This means that we assume the SAD <partitioning> 
        // element contains the <hostcollocation> element. NB: Ownership 
        // of the ConnectionManager is passed to the application.
        _allocations.transfer(allocationIDs);

        _application->populateApplication(
            assemblyController,
            _appUsedDevs, 
            _startSeq, 
            connections, 
            allocationIDs);

        // Add a reference to the new application to the 
        // ApplicationSequence in DomainManager
        CF::Application_var appObj = _application->_this();
        try {
            _appFact._domainManager->addApplication(_application);
        } catch (CF::DomainManager::ApplicationInstallationError& ex) {
            // something bad happened - clean up
            LOG_ERROR(ApplicationFactory_impl, ex.msg);
            throw CF::ApplicationFactory::CreateApplicationError(ex.errorNumber, ex.msg);
        }

        // After all components have been deployed, we know that the first
        // executable device in the list was used for the last deployment,
        // so update the domain manager
        _appFact._domainManager->setLastDeviceUsedForDeployment(_executableDevices.front()->identifier);

        if ( _appFact._domainManager ) {
          _appFact._domainManager->sendAddEvent( _appFact._identifier.c_str(), 
                                                 appIdentifier.c_str(), 
                                                 name,
                                                 appObj,
                                                 StandardEvent::APPLICATION);
        }

        LOG_INFO(ApplicationFactory_impl, "Done creating application " << appIdentifier << " " << name);
        _isComplete = true;
        return appObj._retn();
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation; " << ex.msg);
        throw;
    } catch (CF::ApplicationFactory::CreateApplicationRequestError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation")
        throw;
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str()));
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, eout.str().c_str()));
    } catch ( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "Unexpected error in application creation - see log")
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unexpected error in application creation - see log."));
    }

}

ossie::ComponentInfo* createHelper::getAssemblyController()
{
    for (PlacementList::iterator ii = _requiredComponents.begin(); ii != _requiredComponents.end(); ++ii) {
        if ((*ii)->isAssemblyController()) {
            return *ii;
        }
    }
    return 0;
}

void createHelper::overrideExternalProperties(const CF::Properties& initConfiguration)
{
    const std::vector<SoftwareAssembly::Property>& props = _appInfo.getExternalProperties();

    for (unsigned int i = 0; i < initConfiguration.length(); ++i) {
        for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
            std::string id;
            if (prop->externalpropid == "") {
                id = prop->propid;
            } else {
                id = prop->externalpropid;
            }

            if (id == static_cast<const char*>(initConfiguration[i].id)) {
                ComponentInfo *comp = findComponentByInstantiationId(prop->comprefid);
                // Only configure on non AC components
                if (comp != 0 && !comp->isAssemblyController()) {
                    comp->overrideProperty(prop->propid.c_str(), initConfiguration[i].value);
                }
            }
        }
    }
}

void createHelper::overrideProperties(const CF::Properties& initConfiguration,
                                      ossie::ComponentInfo* component) {
    // Override properties
    for (unsigned int initCount = 0; initCount < initConfiguration.length(); initCount++) {
        const std::string init_id(initConfiguration[initCount].id);
        if (init_id == "LOGGING_CONFIG_URI"){
            // See if the LOGGING_CONFIG_URI has already been set
            // via <componentproperties> or initParams
            bool alreadyHasLoggingConfigURI = false;
            CF::Properties execParameters = component->getExecParameters();
            for (unsigned int i = 0; i < execParameters.length(); ++i) {
                const std::string propid(execParameters[i].id);
                if (propid == "LOGGING_CONFIG_URI") {
                    alreadyHasLoggingConfigURI = true;
                    break;
                }
            }
            // If LOGGING_CONFIG_URI isn't already an exec param, add it
            // Otherwise, don't override component exec param value 
            if (!alreadyHasLoggingConfigURI) {
                // Add LOGGING_CONFIG_URI as an exec param now so that it can be set to the overridden value
                CF::DataType lcuri = initConfiguration[initCount];
                component->addExecParameter(lcuri);
                LOG_TRACE(ApplicationFactory_impl, "Adding LOGGING_CONFIG_URI as exec param with value "
                      << ossie::any_to_string(lcuri.value));
            }
        } else {
            LOG_TRACE(ApplicationFactory_impl, "Overriding property " << init_id
                      << " with " << ossie::any_to_string(initConfiguration[initCount].value));
            component->overrideProperty(init_id.c_str(), initConfiguration[initCount].value);
        }
    }
}

CF::AllocationManager::AllocationResponseSequence* createHelper::allocateUsesDeviceProperties(const UsesDeviceInfo::List& usesDevices, const CF::Properties& configureProperties)
{
    CF::AllocationManager::AllocationRequestSequence request;
    request.length(usesDevices.size());
    
    for (unsigned int usesdev_idx=0; usesdev_idx< usesDevices.size(); usesdev_idx++) {
        const std::string requestid = usesDevices[usesdev_idx]->getId();
        request[usesdev_idx].requestID = requestid.c_str();

        // Get the usesdevice dependency properties, first from the SPD...
        CF::Properties& allocationProperties = request[usesdev_idx].allocationProperties;
        const std::vector<SPD::PropertyRef>&prop_refs = usesDevices[usesdev_idx]->getProperties();
        this->_castRequestProperties(allocationProperties, prop_refs);
        
        // ...then from the SAD; in practice, these are mutually exclusive, but
        // there is no harm in doing both, as one set will always be empty
        const std::vector<SoftwareAssembly::PropertyRef>& sad_refs = usesDevices[usesdev_idx]->getSadDeps();
        this->_castRequestProperties(allocationProperties, sad_refs, allocationProperties.length());
        
        this->_evaluateMATHinRequest(allocationProperties, configureProperties);
    }
    
    return this->_allocationMgr->allocate(request);
}
                                                          
/* Check all allocation dependencies for a particular component and assign it to a device.
 *  - Check component's overall usesdevice dependencies
 *  - Allocate capacity on usesdevice(s)
 *  - Find and implementation that has it's implementation-specific usesdevice dependencies satisfied
 *  - Allocate the component to a particular device

 Current implementation takes advantage of single failure then clean up everything..... To support collocation
 allocation failover for mulitple devices, then we need to clean up only the allocations that we made during a failed
 collocation request.  This requires that we know and cleanup only those allocations that we made..
 appCapacityTable holds all the applications that were made during the entire application deployment process.

 I think for each try of a collocation request... we need to swap out the current appCapacityTable for a
 temporary table, to assist with the allocation and clean up

 */
void createHelper::allocateComponent(ossie::ComponentInfo*  component,
                                     const std::string& assignedDeviceId,
                                     DeviceAssignmentList &appAssignedDevs,
                                     const std::string& appIdentifier)
{
    // get the implementations from the component
    ossie::ImplementationInfo::List  implementations;
    component->getImplementations(implementations);

    CF::Properties configureProperties = component->getConfigureProperties();
    const CF::Properties &construct_props = component->getConstructProperties();
    unsigned int configlen = configureProperties.length();
    configureProperties.length(configureProperties.length()+construct_props.length());
    for (unsigned int i=0; i<construct_props.length(); i++) {
      configureProperties[i+configlen] = construct_props[i];
    }
    
    // Find the devices that allocate the SPD's minimum required usesdevices properties
    const UsesDeviceInfo::List &usesDevVec = component->getUsesDevices();
    if (!allocateUsesDevices(component->getIdentifier(), usesDevVec, configureProperties, appAssignedDevs, this->_allocations)) {
        // There were unsatisfied usesdevices for the component
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (UsesDeviceInfo::List::const_iterator uses = usesDevVec.begin(); uses != usesDevVec.end(); ++uses) {
            if ((*uses)->getAssignedDeviceId().empty()) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << (*uses)->getId();
            }
        }
        eout << "for component '" << component->getIdentifier() << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }
    
    // now attempt to find an implementation that can have it's allocation requirements met
    for (size_t implCount = 0; implCount < implementations.size(); implCount++) {
        ossie::ImplementationInfo* impl = implementations[implCount];

        // Handle 'usesdevice' dependencies for the particular implementation
        DeviceAssignmentList implAllocatedDevices;
        ScopedAllocations implAllocations(*this->_allocationMgr);
        const UsesDeviceInfo::List &implUsesDevVec = impl->getUsesDevices();
        
        if (!allocateUsesDevices(component->getIdentifier(), implUsesDevVec, configureProperties, implAllocatedDevices, implAllocations)) {
            LOG_DEBUG(ApplicationFactory_impl, "Unable to satisfy 'usesdevice' dependencies for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }
        
        // Found an implementation which has its 'usesdevice' dependencies
        // satisfied, now perform assignment/allocation of component to device
        LOG_DEBUG(ApplicationFactory_impl, "Trying to find the device");
        ossie::AllocationResult response = allocateComponentToDevice(component, impl, assignedDeviceId, appIdentifier);
        
        if (response.first.empty()) {
            LOG_DEBUG(ApplicationFactory_impl, "Unable to allocate device for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }
        
        // Track successful deployment allocation
        implAllocations.push_back(response.first);
        
        // Convert from response back into a device node
        DeviceNode& node = *(response.second);
        const std::string& deviceId = node.identifier;
        
        if (!resolveSoftpkgDependencies(impl, node)) {
            component->clearSelectedImplementation();
            LOG_DEBUG(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }
        
        // Allocation to a device succeeded
        LOG_DEBUG(ApplicationFactory_impl, "Assigned component " << component->getInstantiationIdentifier()
                  << " implementation " << impl->getId() << " to device " << deviceId);
        component->setAssignedDevice(response.second);

        // Move the device to the front of the list
        rotateDeviceList(_executableDevices, deviceId);
        
        ossie::DeviceAssignmentInfo dai;
        dai.deviceAssignment.componentId = CORBA::string_dup(component->getIdentifier());
        dai.deviceAssignment.assignedDeviceId = deviceId.c_str();
        dai.device = CF::Device::_duplicate(node.device);
        appAssignedDevs.push_back(dai);
        
        // Store the implementation-specific usesdevice allocations and
        // device assignments
        implAllocations.transfer(this->_allocations);
        std::copy(implAllocatedDevices.begin(), implAllocatedDevices.end(), std::back_inserter(appAssignedDevs));
        
        component->setSelectedImplementation(impl);
        return;
    }

    ossie::DeviceList::iterator device;
    ossie::DeviceList devices = _registeredDevices;
    bool allBusy = true;
    unsigned int num_exec_devices = 0;
    for (device = devices.begin(); device != devices.end(); ++device) {
        if ((*device)->isExecutable) {
            num_exec_devices++;
            if ((*device)->device->usageState() != CF::Device::BUSY) {
                allBusy = false;
            }
        }
    }
    if (num_exec_devices == 0) {
        // Report failure
        std::ostringstream eout;
        eout << "Unable to launch component '"<<component->getName()<<"'. No executable devices (i.e.: GPP) are available in the Domain";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }
    if (allBusy) {
        // Report failure
        std::ostringstream eout;
        eout << "Unable to launch component '"<<component->getName()<<"'. All executable devices (i.e.: GPP) in the Domain are busy";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }

    // Report failure
    std::ostringstream eout;
    eout << "Failed to satisfy device dependencies for component: '";
    eout << component->getName() << "' with component id: '" << component->getIdentifier() << "'";
    LOG_DEBUG(ApplicationFactory_impl, eout.str());
    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
}

bool createHelper::allocateUsesDevices(const std::string& componentIdentifier,
                                       const ossie::UsesDeviceInfo::List& usesDevices,
                                       const CF::Properties& configureProperties,
                                       DeviceAssignmentList& deviceAssignments,
                                       ScopedAllocations& allocations)
{
    // Create a temporary lookup table for reconciling allocation requests with
    // usesdevice identifiers
    typedef std::map<std::string,UsesDeviceInfo*> UsesDeviceMap;
    UsesDeviceMap usesDeviceMap;
    for (UsesDeviceInfo::List::const_iterator iter = usesDevices.begin(); iter != usesDevices.end(); ++iter) {
        // Ensure that no devices are assigned to start; the caller can check
        // for unassigned devices to report which usesdevices failed
        (*iter)->clearAssignedDeviceId();
        usesDeviceMap[(*iter)->getId()] = *iter;
    }
    
    // Track allocations made internally, either to clean up on failure or to
    // pass to the caller
    ScopedAllocations localAllocations(*_allocationMgr);
    
    CF::AllocationManager::AllocationResponseSequence_var response = allocateUsesDeviceProperties(usesDevices, configureProperties);
    for (unsigned int resp = 0; resp < response->length(); resp++) {
        // Ensure that this allocation is recorded so that it can be cleaned up
        const std::string allocationId(response[resp].allocationID);
        LOG_TRACE(ApplicationFactory_impl, "Allocated " << allocationId);
        localAllocations.push_back(allocationId);
        
        // Find the usesdevice that matches the request and update it, removing
        // the key from the map
        const std::string requestID(response[resp].requestID);
        UsesDeviceMap::iterator uses = usesDeviceMap.find(requestID);
        if (uses == usesDeviceMap.end()) {
            // This condition should never occur
            LOG_WARN(ApplicationFactory_impl, "Allocation request " << requestID
                     << " does not match any usesdevice");
            continue;
        }
        const std::string deviceId = ossie::corba::returnString(response[resp].allocatedDevice->identifier());
        uses->second->setAssignedDeviceId(deviceId);
        usesDeviceMap.erase(uses);

        DeviceAssignmentInfo assignment;
        assignment.deviceAssignment.componentId = componentIdentifier.c_str();
        assignment.deviceAssignment.assignedDeviceId = deviceId.c_str();
        assignment.device = CF::Device::_duplicate(response[resp].allocatedDevice);
        deviceAssignments.push_back(assignment);
    }
    
    if (usesDeviceMap.empty()) {
        // All usesdevices were satisfied; give the caller ownership of all the
        // allocations
        localAllocations.transfer(allocations);
        return true;
    } else {
        // Some usesdevices were not satisfied--these will have no assigned
        // device id; successful allocations will be deallocated when the
        // ScopedAllocations goes out of scope
        return false;
    }
}

void createHelper::_evaluateMATHinRequest(CF::Properties &request, const CF::Properties &configureProperties)
{
    for (unsigned int math_prop=0; math_prop<request.length(); math_prop++) {
        CF::Properties *tmp_prop;
        if (request[math_prop].value >>= tmp_prop) {
            this->_evaluateMATHinRequest(*tmp_prop, configureProperties);
            request[math_prop].value <<= *tmp_prop;
            continue;
        }
        std::string value = ossie::any_to_string(request[math_prop].value);
        if (value.find("__MATH__") != string::npos) {
            // Turn propvalue into a string for easy parsing
            std::string mathStatement = value.substr(8);
            if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                std::vector<std::string> args;
                while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                    LOG_TRACE(ApplicationFactory_impl, "__MATH__ ARG: " << mathStatement.substr(0, mathStatement.find(',')) );
                    args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                    mathStatement.erase(0, mathStatement.find(',') + 1);
                }
                args.push_back(mathStatement);

                if (args.size() != 3) {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                    throw ossie::PropertyMatchingError(eout.str());
                }

                double operand = strtod(args[0].c_str(), NULL);
                if (args[0].size() == 0) {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ argument (argument empty);";
                    throw ossie::PropertyMatchingError(eout.str());
                }
                if (not std::isdigit(args[0][0])) { // if the first character is not numeric, then cannot apply __MATH__
                    std::ostringstream eout;
                    eout << " invalid __MATH__ argument; '" << args[0] << "'";
                    if (args[0][0] != '.') {
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                    if (args[0].size() == 1) { // the string is only '.'
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                    if (not std::isdigit(args[0][1])) { // the string starts with '.' but is not followed by a number
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                }

                // See if there is a property in the component
                const CF::DataType* matchingCompProp = 0;
                for (unsigned int j = 0; j < configureProperties.length(); j++) {
                    if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                        matchingCompProp = &configureProperties[j];
                    }
                }

                CF::Properties *tmp_prop;
                if (matchingCompProp == 0) {
                    // see if it's in a struct
                    for (unsigned int j = 0; j < configureProperties.length(); j++) {
                        if (configureProperties[j].value >>= tmp_prop) {
                            for (unsigned int jj = 0; jj < (*tmp_prop).length(); jj++) {
                                if (strcmp((*tmp_prop)[jj].id, args[1].c_str()) == 0) {
                                    matchingCompProp = &(*tmp_prop)[jj];
                                    break;
                                }
                            }
                        }
                        if (matchingCompProp != 0)
                            break;
                    }
                }

                if (matchingCompProp == 0) {
                    std::ostringstream eout;
                    eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                    throw ossie::PropertyMatchingError(eout.str());
                }

                std::string math = args[2];
                CORBA::Any compValue = matchingCompProp->value;
                CORBA::TypeCode_var matchingCompPropType = matchingCompProp->value.type();
                request[math_prop].value = ossie::calculateDynamicProp(operand, compValue, math, matchingCompPropType->kind());
                std::string retval = ossie::any_to_string(request[math_prop].value);
                LOG_DEBUG(ApplicationFactory_impl, "__MATH__ RESULT: " << retval << " op1: " << operand << " op2:" << ossie::any_to_string(compValue) );
            } else {
                std::ostringstream eout;
                eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                throw ossie::PropertyMatchingError(eout.str());
            }
        }
    }
}

/* Perform allocation/assignment of a particular component to the device.
 *  - First do allocation/assignment based on user provided DAS
 *  - If not specified in DAS, then iterate through devices looking for a device that satisfies
 *    the allocation properties
 */
ossie::AllocationResult createHelper::allocateComponentToDevice( ossie::ComponentInfo* component,
                                              ossie::ImplementationInfo* implementation,
                                              const std::string& assignedDeviceId,
                                              const std::string& appIdentifier)
{
    ossie::DeviceList devices = _registeredDevices;

    // First check to see if the component was assigned in the user provided DAS
    // See if a device was assigned in the DAS
    if (!assignedDeviceId.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "User-provided DAS: Component: '" << component->getName() <<
                  "'  Assigned device: '" << assignedDeviceId << "'");
        ossie::DeviceList::iterator device;
        for (device = devices.begin(); device != devices.end(); ++device) {
            if (assignedDeviceId == (*device)->identifier) {
                break;
            }
        }

        if (device == devices.end()) {
            LOG_DEBUG(ApplicationFactory_impl, "DAS specified unknown device " << assignedDeviceId <<
                      " for component " << component->getIdentifier());
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = CORBA::string_dup(component->getIdentifier());
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }

        // Remove all non-requested devices
        devices.erase(devices.begin(), device++);
        devices.erase(device, devices.end());
    }

    const std::string requestid = ossie::generateUUID();
    std::vector<SPD::PropertyRef> prop_refs = implementation->getDependencyProperties();
    redhawk::PropertyMap allocationProperties;
    this->_castRequestProperties(allocationProperties, prop_refs);

    CF::Properties configure_props = component->getConfigureProperties();
    CF::Properties construct_props = component->getConstructProperties();
    unsigned int initial_length = configure_props.length();
    configure_props.length(configure_props.length()+construct_props.length());
    for (unsigned int i=0; i<construct_props.length(); i++) {
        configure_props[i+initial_length] = construct_props[i];
    }
    this->_evaluateMATHinRequest(allocationProperties, configure_props);
    
    LOG_TRACE(ApplicationFactory_impl, "alloc prop size " << allocationProperties.size() );
    redhawk::PropertyMap::iterator iter=allocationProperties.begin();
    for( ; iter != allocationProperties.end(); iter++){
      LOG_TRACE(ApplicationFactory_impl, "alloc prop: " << iter->id  <<" value:" <<  ossie::any_to_string(iter->value) );
    }
    
    redhawk::PropertyMap::iterator nic_alloc = allocationProperties.find("nic_allocation");
    std::string alloc_id;
    if (nic_alloc != allocationProperties.end()) {
        redhawk::PropertyMap& substr = nic_alloc->getValue().asProperties();
        alloc_id = substr["nic_allocation::identifier"].toString();
        if (alloc_id.empty()) {
          alloc_id = ossie::generateUUID();
          substr["nic_allocation::identifier"] = alloc_id;
        }
    }
    
    ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid, allocationProperties, devices, appIdentifier, implementation->getProcessorDeps(), implementation->getOsDeps());
    if (allocationProperties.contains("nic_allocation")) {
        if (!response.first.empty()) {
            redhawk::PropertyMap query_props;
            query_props["nic_allocation_status"] = redhawk::Value();
            response.second->device->query(query_props);
            redhawk::ValueSequence& retstruct = query_props["nic_allocation_status"].asSequence();
            for (redhawk::ValueSequence::iterator it = retstruct.begin(); it!=retstruct.end(); it++) {
                redhawk::PropertyMap& struct_prop = it->asProperties();
                std::string identifier = struct_prop["nic_allocation_status::identifier"].toString();
                if (identifier == alloc_id) {
                    const std::string interface = struct_prop["nic_allocation_status::interface"].toString();
                    LOG_DEBUG(ApplicationFactory_impl, "Allocation NIC assignment: " << interface );
                    component->setNicAssignment(interface);
                    redhawk::PropertyType nic_execparam;
                    nic_execparam.id = "NIC";
                    nic_execparam.setValue(interface);
                    component->addExecParameter(nic_execparam);

                    // RESOLVE - need SAD file directive to control this behavior.. i.e if promote_nic_to_affinity==true...
                    // for now add nic assignment as application affinity to all components deployed by this device
                    _app_affinity = component->getAffinityOptionsWithAssignment();
                }
            }
        }
    }
    TRACE_EXIT(ApplicationFactory_impl);
    return response;
}

void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SPD::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
      allocationProperties[offset+i] = castProperty(prop_refs[i].property.get());
    }
}

void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SoftwareAssembly::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
      allocationProperties[offset+i] = castProperty(prop_refs[i].property.get());
    }
}

CF::DataType createHelper::castProperty(const ossie::ComponentProperty* property)
{
    if (dynamic_cast<const SimplePropertyRef*>(property) != NULL) {
        const SimplePropertyRef* dependency = dynamic_cast<const SimplePropertyRef*>(property);
        return convertPropertyToDataType(&(*dependency));
    } else if (dynamic_cast<const SimpleSequencePropertyRef*>(property) != NULL) {
        const SimpleSequencePropertyRef* dependency = dynamic_cast<const SimpleSequencePropertyRef*>(property);
        return convertPropertyToDataType(dependency);
    } else if (dynamic_cast<const ossie::StructPropertyRef*>(property) != NULL) {
        const ossie::StructPropertyRef* dependency = dynamic_cast<const ossie::StructPropertyRef*>(property);
        return convertPropertyToDataType(dependency);
    } else if (dynamic_cast<const ossie::StructSequencePropertyRef*>(property) != NULL) {
        const ossie::StructSequencePropertyRef* dependency = dynamic_cast<const ossie::StructSequencePropertyRef*>(property);
        return convertPropertyToDataType(dependency);
    }
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(property->_id.c_str());
    return dataType;
}

bool createHelper::resolveSoftpkgDependencies(ossie::ImplementationInfo* implementation, ossie::DeviceNode& device)
{
    const std::vector<ossie::SoftpkgInfo*>& tmpSoftpkg = implementation->getSoftPkgDependency();
    std::vector<ossie::SoftpkgInfo*>::const_iterator iterSoftpkg;

    for (iterSoftpkg = tmpSoftpkg.begin(); iterSoftpkg != tmpSoftpkg.end(); ++iterSoftpkg) {
        // Find an implementation whose dependencies match
        ossie::ImplementationInfo* spdImplInfo = resolveDependencyImplementation(*iterSoftpkg, device);
        if (spdImplInfo) {
            (*iterSoftpkg)->setSelectedImplementation(spdImplInfo);
        } else {
            LOG_DEBUG(ApplicationFactory_impl, "resolveSoftpkgDependencies: implementation match not found between soft package dependency and device");
            implementation->clearSelectedDependencyImplementations();
            return false;
        }
    }

    return true;
}

ossie::ImplementationInfo* createHelper::resolveDependencyImplementation(ossie::SoftpkgInfo* softpkg,
                                                                         ossie::DeviceNode& device)
{
    ossie::ImplementationInfo::List spd_list;
    softpkg->getImplementations(spd_list);

    for (size_t implCount = 0; implCount < spd_list.size(); implCount++) {
        ossie::ImplementationInfo* implementation = spd_list[implCount];
        // Check that this implementation can run on the device
        if (!implementation->checkProcessorAndOs(device.prf)) {
            continue;
        }

        // Recursively check any softpkg dependencies
        if (resolveSoftpkgDependencies(implementation, device)) {
            return implementation;
        }
    }

    return 0;
}

/* Create a vector of all the components for the SAD associated with this App Factory
 *  - Get component information from the SAD and store in _requiredComponents vector
 */
void createHelper::getRequiredComponents()
 throw (CF::ApplicationFactory::CreateApplicationError)
{
    TRACE_ENTER(ApplicationFactory_impl);

    std::vector<ComponentPlacement> componentsFromSAD = _appFact._sadParser.getAllComponents();

    const std::string assemblyControllerRefId = _appFact._sadParser.getAssemblyControllerRefId();

    // Bin the start orders based on the values in the SAD. Using a map of
    // vectors, keyed on the start order value, accounts for duplicate keys and
    // allows assigning the effective order easily by iterating through all
    // the values.
    std::map<int,std::vector<std::string> > startOrders;

    for (unsigned int i = 0; i < componentsFromSAD.size(); i++) {
        const ComponentPlacement& component = componentsFromSAD[i];
        
        // Create a list of pairs of start orders and instantiation IDs
        for (unsigned int ii = 0; ii < component.getInstantiations().size(); ii++) {
            // Only add a pair if a start order was provided, and the component is not the assembly controller
            if (strcmp(component.getInstantiations()[ii].getStartOrder(), "") != 0 &&
                    component.getInstantiations()[ii].getID() != assemblyControllerRefId) {
                // Get the start order of the component
                int startOrder = atoi(component.getInstantiations()[ii].getStartOrder());
                std::string instId = component.getInstantiations()[ii].getID();
                startOrders[startOrder].push_back(instId);
            }
        }
        
        // Extract required data from SPD file
        ossie::ComponentInfo* newComponent = 0;
        LOG_TRACE(ApplicationFactory_impl, "Getting the SPD Filename")
        const char *spdFileName = _appFact._sadParser.getSPDById(component.getFileRefId());
        if (spdFileName == NULL) {
            ostringstream eout;
            eout << "The SPD file reference for componentfile "<<component.getFileRefId()<<" is missing";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }
        LOG_TRACE(ApplicationFactory_impl, "Building Component Info From SPD File")
        newComponent = ossie::ComponentInfo::buildComponentInfoFromSPDFile(_appFact._fileMgr, spdFileName);
        if (newComponent == 0) {
            ostringstream eout;
            eout << "Error loading component information for file ref " << component.getFileRefId();
            LOG_ERROR(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Done building Component Info From SPD File")
        // Even though it is possible for there to be more than one instantiation per component,
        //  the tooling doesn't support that, so supporting this at a framework level would add
        //  substantial complexity without providing any appreciable improvements. It is far
        //  easier to have multiple placements rather than multiple instantiations.
        const vector<ComponentInstantiation>& instantiations = component.getInstantiations();

        const ComponentInstantiation& instance = instantiations[0];

        ostringstream identifier;
        identifier << instance.getID();
        // Violate SR:172, we use the uniquified name rather than the passed in name
        identifier << ":" << _waveformContextName;
        assert(newComponent != 0);
        newComponent->setIdentifier(identifier.str().c_str(), instance.getID());

        if (newComponent->getInstantiationIdentifier() == assemblyControllerRefId) {
            newComponent->setIsAssemblyController(true);
        }

        newComponent->setNamingService(instance.isNamingService());

        if (newComponent->getNamingService()) {
            ostringstream nameBinding;
            nameBinding << instance.getFindByNamingServiceName();
#if UNIQUIFY_NAME_BINDING
// DON'T USE THIS YET AS IT WILL BREAK OTHER PARTS OF REDHAWK
            nameBinding << "_" << i;  // Add a _UniqueIdentifier, per SR:169
#endif
            newComponent->setNamingServiceName(nameBinding.str().c_str());  // SR:169
        } else {
            if (newComponent->isScaCompliant()) {
                LOG_WARN(ApplicationFactory_impl, "component instantiation is sca compliant but does not provide a 'findcomponent' name...this is probably an error")
            }
        }
    
        newComponent->setUsageName(instance.getUsageName());
        newComponent->setAffinity( instance.getAffinity() );
        newComponent->setLoggingConfig( instance.getLoggingConfig() );

        const ossie::ComponentPropertyList & ins_prop = instance.getProperties();

        int docker_image_idx = -1;
        for (unsigned int i = 0; i < ins_prop.size(); ++i) {
            if (ins_prop[i]._id == "__DOCKER_IMAGE__") {
                docker_image_idx = i;
                continue;
            }
            newComponent->overrideProperty(&ins_prop[i]);
        }

        if (docker_image_idx > -1) {
            CF::Properties tmp;
            redhawk::PropertyMap& tmpProp = redhawk::PropertyMap::cast(tmp);
            tmpProp["__DOCKER_IMAGE__"].setValue(dynamic_cast<const SimplePropertyRef &>(ins_prop[docker_image_idx]).getValue());
            newComponent->addExecParameter(tmpProp[0]);
        }

        _requiredComponents.push_back(newComponent);
    }

    // Build the start order instantiation ID vector in the right order
    _startOrderIds.clear();
    for (std::map<int,std::vector<std::string> >::iterator ii = startOrders.begin(); ii != startOrders.end(); ++ii) {
        _startOrderIds.insert(_startOrderIds.end(), ii->second.begin(), ii->second.end());
    }

    TRACE_EXIT(ApplicationFactory_impl);
}

/* Given a device id, returns a CORBA pointer to the device
 *  - Gets a CORBA pointer for a device from a given id
 */
CF::Device_ptr createHelper::find_device_from_id(const char* device_id)
{
    try {
        return CF::Device::_duplicate(find_device_node_from_id(device_id).device);
    } catch ( ... ){
    }

    for (DeviceAssignmentList::iterator iter = _appUsedDevs.begin(); iter != _appUsedDevs.end(); ++iter) {
        if (strcmp(device_id, iter->deviceAssignment.assignedDeviceId) == 0) {
            return CF::Device::_duplicate(iter->device);
        }
    }

    TRACE_EXIT(ApplicationFactory_impl);
    return CF::Device::_nil();
}

const ossie::DeviceNode& createHelper::find_device_node_from_id(const char* device_id) throw(std::exception)
{
    for (DeviceList::iterator dn = _registeredDevices.begin(); dn != _registeredDevices.end(); ++dn) {
        if ((*dn)->identifier == device_id) {
            return **dn;
        }
    }

    TRACE_EXIT(ApplicationFactory_impl);
    throw(std::exception());
}

/* Given a component instantiation id, returns the associated ossie::ComponentInfo object
 *  - Gets the ComponentInfo class instance for a particular component instantiation id
 */
ossie::ComponentInfo* createHelper::findComponentByInstantiationId(const std::string& identifier)
{
    for (size_t ii = 0; ii < _requiredComponents.size(); ++ii) {
        if (identifier == _requiredComponents[ii]->getInstantiationIdentifier()) {
            return _requiredComponents[ii];
        }
    }

    return 0;
}

/* Given a waveform/application name, return a unique waveform naming context
 *  - Returns a unique waveform naming context
 *  THIS FUNCTION IS NOT THREAD SAFE
 */
string ApplicationFactory_impl::getWaveformContextName(string name )
{
    //
    // Find a new unique waveform naming for the naming context
    //


    bool found_empty = false;
    string waveform_context_name;

    // iterate through N for waveformname_N until a unique naming context if found
    CosNaming::NamingContext_ptr inc = ossie::corba::InitialNamingContext();
    do {
        ++_lastWaveformUniqueId;
        // Never use 0
        if (_lastWaveformUniqueId == 0) ++_lastWaveformUniqueId;
        waveform_context_name = "";
        waveform_context_name.append(name);
        std::string mod_waveform_context_name = waveform_context_name;
        for (int i=mod_waveform_context_name.size()-1;i>=0;i--) {
            if (mod_waveform_context_name[i]=='.') {
                mod_waveform_context_name.insert(i, 1, '\\');
            }
        }
        waveform_context_name.append("_");
        mod_waveform_context_name.append("_");
        ostringstream number_str;
        number_str << _lastWaveformUniqueId;
        waveform_context_name.append(number_str.str());
        mod_waveform_context_name.append(number_str.str());
        string temp_waveform_context(_domainName + string("/"));
        temp_waveform_context.append(mod_waveform_context_name);
        CosNaming::Name_var cosName = ossie::corba::stringToName(temp_waveform_context);
        try {
            CORBA::Object_var obj_WaveformContext = inc->resolve(cosName);
        } catch (const CosNaming::NamingContext::NotFound&) {
            found_empty = true;
        }
    } while (!found_empty);

    return waveform_context_name;

}

/* Given a waveform/application-specific context, return the full waveform naming context
 *  - Returns a full context path for the waveform
 */
string ApplicationFactory_impl::getBaseWaveformContext(string waveform_context)
{
    string base_naming_context(_domainName + string("/"));
    base_naming_context.append(waveform_context);

    return base_naming_context;
}

void createHelper::loadDependencies(ossie::ComponentInfo& component,
                                    CF::LoadableDevice_ptr device,
                                    const std::vector<SoftpkgInfo*>& dependencies)
{
    for (std::vector<SoftpkgInfo*>::const_iterator dep = dependencies.begin(); dep != dependencies.end(); ++dep) {
        const ossie::ImplementationInfo* implementation = (*dep)->getSelectedImplementation();
        if (!implementation) {
            LOG_ERROR(ApplicationFactory_impl, "No implementation selected for dependency " << (*dep)->getName());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Missing implementation");
        }

        // Recursively load dependencies
        LOG_TRACE(ApplicationFactory_impl, "Loading dependencies for soft package " << (*dep)->getName());
        loadDependencies(component, device, implementation->getSoftPkgDependency());

        // Determine absolute path of dependency's local file
        CF::LoadableDevice::LoadType codeType = implementation->getCodeType();
        fs::path codeLocalFile = fs::path(implementation->getLocalFileName());
        if (!codeLocalFile.has_root_directory()) {
            // Path is relative to SPD file location
            fs::path base_dir = fs::path((*dep)->getSpdFileName()).parent_path();
            codeLocalFile = base_dir / codeLocalFile;
        }
        codeLocalFile = codeLocalFile.normalize();
        if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
            codeLocalFile = codeLocalFile.branch_path();
        }

        const std::string fileName = codeLocalFile.string();
        LOG_DEBUG(ApplicationFactory_impl, "Loading dependency local file " << fileName);
        try {
             device->load(_appFact._fileMgr, fileName.c_str(), codeType);
        } catch (...) {
            LOG_ERROR(ApplicationFactory_impl, "Failure loading file " << fileName);
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Failed to load file");
        }
        component.addResolvedSoftPkgDependency(fileName);
        _application->addComponentLoadedFile(component.getIdentifier(), fileName);
    }
}

/* Perform 'load' and 'execute' operations to launch component on the assigned device
 *  - Actually loads and executes the component on the given device
 */
void createHelper::loadAndExecuteComponents(CF::ApplicationRegistrar_ptr _appReg)
{
    LOG_TRACE(ApplicationFactory_impl, "Loading and Executing " << _requiredComponents.size() << " components");
    // apply application affinity options to required components
    applyApplicationAffinityOptions();

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        const ossie::ImplementationInfo* implementation = component->getSelectedImplementation();

        boost::shared_ptr<ossie::DeviceNode> device = component->getAssignedDevice();
        if (!device) {
            std::ostringstream message;
            message << "component " << component->getIdentifier() << " was not assigned to a device";
            throw std::logic_error(message.str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Component - " << component->getName()
                  << "   Assigned device - " << device->identifier);

        // Let the application know to expect the given component
        _application->addComponent(component->getIdentifier(), component->getSpdFileName());
        _application->setComponentImplementation(component->getIdentifier(), implementation->getId());
        if (component->getNamingService()) {
            std::string lookupName = _appFact._domainName + "/" + _waveformContextName + "/" + component->getNamingServiceName() ;
            _application->setComponentNamingContext(component->getIdentifier(), lookupName);
        }
        _application->setComponentDevice(component->getIdentifier(), device->device);

        // get the code.localfile
        fs::path codeLocalFile = fs::path(implementation->getLocalFileName());
        LOG_TRACE(ApplicationFactory_impl, "Host is " << device->label << " Local file name is "
                << codeLocalFile);
        if (!codeLocalFile.has_root_directory()) {
            codeLocalFile = fs::path(component->spd.getSPDPath()) / codeLocalFile;
        }
        codeLocalFile = codeLocalFile.normalize();
        if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
            codeLocalFile = codeLocalFile.branch_path();
        }

        // Get file name, load if it is not empty
        if (codeLocalFile.string().size() <=  0) {
            ostringstream eout;
            eout << "code.localfile is empty for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getId() << "'";
            eout << " on device id: '" << device->identifier << "'";
            eout << " in waveform '" << _waveformContextName<<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str());
        }

        // narrow to LoadableDevice interface
        CF::LoadableDevice_var loadabledev = ossie::corba::_narrowSafe<CF::LoadableDevice>(device->device);
        if (CORBA::is_nil(loadabledev)) {
            std::ostringstream message;
            message << "component " << component->getIdentifier() << " was assigned to non-loadable device "
                    << device->identifier;
            throw std::logic_error(message.str());
        }

        loadDependencies(*component, loadabledev, implementation->getSoftPkgDependency());

        // load the file(s)
        ostringstream load_eout; // used for any error messages dealing with load
        try {
            try {
                LOG_TRACE(ApplicationFactory_impl, "loading " << codeLocalFile << " on device " << ossie::corba::returnString(loadabledev->label()));
                loadabledev->load(_appFact._fileMgr, codeLocalFile.string().c_str(), implementation->getCodeType());
            } catch( ... ) {
                load_eout << "'load' failed for component: '";
                load_eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
                load_eout << " with implementation id: '" << implementation->getId() << "';";
                load_eout << " on device id: '" << device->identifier << "'";
                load_eout << " in waveform '" << _waveformContextName<<"'";
                load_eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw;
            }
        } catch( CF::InvalidFileName& _ex ) {
            load_eout << " with error: <" << _ex.msg << ">;";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str());
        } catch( CF::Device::InvalidState& _ex ) {
            load_eout << " with error: <" << _ex.msg << ">;";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str());
        } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl, "", CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str()));

        // Mark the file as loaded
        _application->addComponentLoadedFile(component->getIdentifier(), codeLocalFile.string());
                
        // OSSIE extends section D.2.1.6.3 to support loading a directory
        // and execute a file in that directory using a entrypoint
        // 1. Executable means to use CF LoadableDevice::load and CF ExecutableDevice::execute operations. This is a "main" process.
        //    - A Executable that references a directory instead of a file means to recursively load the contents of the directory
        //      and then execute the program specified via entrypoint
        // 2. Driver and Kernel Module means load only.
        // 3. SharedLibrary means dynamic linking.
        // 4. A (SharedLibrary) Without a code entrypoint element means load only.
        // 5. A (SharedLibrary) With a code entrypoint element means load and CF Device::execute.
        if (((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) ||
                (implementation->getCodeType() == CF::LoadableDevice::SHARED_LIBRARY)) && (implementation->getEntryPoint().size() != 0)) {

            // get executable device reference
            CF::ExecutableDevice_var execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice>(loadabledev);
            if (CORBA::is_nil(execdev)){
                std::ostringstream message;
                message << "component " << component->getIdentifier() << " was assigned to non-executable device "
                        << device->identifier;
                throw std::logic_error(message.str());
            }

            // Add the required parameters specified in SR:163
            // Naming Context IOR, Name Binding, and component identifier
            CF::DataType ci;
            ci.id = "COMPONENT_IDENTIFIER";
            ci.value <<= component->getIdentifier();
            component->addExecParameter(ci);

            CF::DataType nb;
            nb.id = "NAME_BINDING";
            nb.value <<= component->getNamingServiceName();
            component->addExecParameter(nb);

            CF::DataType dp;
            dp.id = "DOM_PATH";
            dp.value <<= _baseNamingContext;
            component->addExecParameter(dp);

            CF::DataType pn;
            pn.id = "PROFILE_NAME";
            pn.value <<= component->getSpdFileName();
            component->addExecParameter(pn);
            
            // See if the LOGGING_CONFIG_URI has already been set
            // via <componentproperties> or initParams
            bool alreadyHasLoggingConfigURI = false;
            std::string logging_uri("");
            CF::DataType* logcfg_prop = NULL;
            CF::Properties execParameters = component->getExecParameters();
            for (unsigned int i = 0; i < execParameters.length(); ++i) {
                std::string propid = static_cast<const char*>(execParameters[i].id);
                if (propid == "LOGGING_CONFIG_URI") {
                  logcfg_prop = &execParameters[i];
                  const char* tmpstr;
                  if ( ossie::any::isNull(logcfg_prop->value) == true ) {
                    LOG_WARN(ApplicationFactory_impl, "Missing value for LOGGING_CONFIG_URI, component: " << _baseNamingContext << "/" << component->getNamingServiceName() );
                  }
                  else {
                    logcfg_prop->value >>= tmpstr;
                    LOG_TRACE(ApplicationFactory_impl, "Resource logging configuration provided, logcfg:" << tmpstr);
                    logging_uri = string(tmpstr);
                    alreadyHasLoggingConfigURI = true;
                  }
                  break;
                }
            }

            ossie::logging::LogConfigUriResolverPtr logcfg_resolver = ossie::logging::GetLogConfigUriResolver();
            std::string logcfg_path = ossie::logging::GetComponentPath( _appFact._domainName, _waveformContextName, component->getNamingServiceName() );
            if ( _appFact._domainManager->getUseLogConfigResolver() && logcfg_resolver ) {
                  std::string t_uri = logcfg_resolver->get_uri( logcfg_path );
                  LOG_DEBUG(ApplicationFactory_impl, "Using LogConfigResolver plugin: path " << logcfg_path << " logcfg:" << t_uri );
                  if ( !t_uri.empty() ) logging_uri = t_uri;
            }
            
            if (!alreadyHasLoggingConfigURI && logging_uri.empty() ) {
                // Query the DomainManager for the logging configuration
                LOG_TRACE(ApplicationFactory_impl, "Checking DomainManager for LOGGING_CONFIG_URI");
                PropertyInterface *log_prop = _appFact._domainManager->getPropertyFromId("LOGGING_CONFIG_URI");
                StringProperty *logProperty = (StringProperty *)log_prop;
                if (!logProperty->isNil()) {
                    logging_uri = logProperty->getValue();
                } else {
                    LOG_TRACE(ApplicationFactory_impl, "DomainManager LOGGING_CONFIG_URI is not set");
                }

                rh_logger::LoggerPtr dom_logger = _appFact._domainManager->getLogger();
                if ( dom_logger ) {
                  rh_logger::LevelPtr dlevel = dom_logger->getLevel();
                  if ( !dlevel ) dlevel = rh_logger::Logger::getRootLogger()->getLevel();
                  CF::DataType prop;
                  prop.id = "DEBUG_LEVEL";
                  prop.value <<= static_cast<CORBA::Long>(ossie::logging::ConvertRHLevelToDebug( dlevel ));
                  component->addExecParameter(prop);
                }
            }

            // if we have a uri but no property, add property to component's exec param list
            if ( logcfg_prop == NULL && !logging_uri.empty() ) {
                CF::DataType prop;
                prop.id = "LOGGING_CONFIG_URI";
                prop.value <<= logging_uri.c_str();
                LOG_DEBUG(ApplicationFactory_impl, "logcfg_prop == NULL " << prop.id << " / " << logging_uri );
                component->addExecParameter(prop);
            }

            if (!logging_uri.empty()) {
                if (logging_uri.substr(0, 4) == "sca:") {
                    string fileSysIOR = ossie::corba::objectToString(_appFact._domainManager->_fileMgr);
                    logging_uri += ("?fs=" + fileSysIOR);
                    LOG_TRACE(ApplicationFactory_impl, "Adding file system IOR " << logging_uri);
                }

                LOG_DEBUG(ApplicationFactory_impl, " LOGGING_CONFIG_URI :" << logging_uri);
                CORBA::Any loguri;
                loguri <<= logging_uri.c_str();
                // this overrides all instances of the property called LOGGING_CONFIG_URI
                LOG_TRACE(ApplicationFactory_impl, "override ....... uri " << logging_uri );
                component->overrideProperty("LOGGING_CONFIG_URI", loguri);
            }
            // Add the Naming Context IOR to make it easier to parse the command line
            CF::DataType ncior;
            ncior.id = "NAMING_CONTEXT_IOR";
            ncior.value <<= ossie::corba::objectToString(_appReg);
            component->addExecParameter(ncior);
            
            std::string sr_key;
            if (this->specialized_reservations.find(std::string(component->getIdentifier())) != this->specialized_reservations.end()) {
                sr_key = std::string(component->getIdentifier());
            } else if (this->specialized_reservations.find(std::string(component->getUsageName())) != this->specialized_reservations.end()) {
                sr_key = std::string(component->getUsageName());
            }
            if (not sr_key.empty()) {
                CF::DataType spec_res;
                spec_res.id = "RH::GPP::MODIFIED_CPU_RESERVATION_VALUE";
                //std::stringstream ss;
                //ss << this->specialized_reservations[sr_key];
                spec_res.value <<= this->specialized_reservations[sr_key];
                component->addExecParameter(spec_res);
            }

            fs::path executeName;
            if ((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) && (implementation->getEntryPoint().size() == 0)) {
                LOG_WARN(ApplicationFactory_impl, "executing using code file as entry point; this is non-SCA compliant behavior; entrypoint must be set")
                executeName = codeLocalFile;
            } else {
                executeName = fs::path(implementation->getEntryPoint());
                LOG_TRACE(ApplicationFactory_impl, "Using provided entry point " << executeName)
                if (!executeName.has_root_directory()) {
                    executeName = fs::path(component->spd.getSPDPath()) / executeName;
                }
                executeName = executeName.normalize();
            }

            attemptComponentExecution(executeName, execdev, component, implementation);
        }
    }
}

void createHelper::attemptComponentExecution (
        const fs::path&                                           executeName,
        CF::ExecutableDevice_ptr                                  execdev,
        ossie::ComponentInfo*                                     component,
        const ossie::ImplementationInfo*                          implementation) {

    CF::Properties execParameters;
    
    // get entrypoint
    CF::ExecutableDevice::ProcessID_Type tempPid = -1;

    // attempt to execute the component
    try {
        LOG_TRACE(ApplicationFactory_impl, "executing " << executeName << " on device " << ossie::corba::returnString(execdev->label()));
        execParameters = component->getExecParameters();
        for (unsigned int i = 0; i < execParameters.length(); ++i) {
            LOG_TRACE(ApplicationFactory_impl, " exec param " << execParameters[i].id << " " << ossie::any_to_string(execParameters[i].value))
        }
        // call 'execute' on the ExecutableDevice to execute the component
        CF::StringSequence dep_seq;
        std::vector<std::string> resolved_softpkg_deps = component->getResolvedSoftPkgDependencies();
        dep_seq.length(resolved_softpkg_deps.size());
        for (unsigned int p=0;p!=dep_seq.length();p++) {
            dep_seq[p]=CORBA::string_dup(resolved_softpkg_deps[p].c_str());
        }

        // get Options list
        CF::Properties cop = component->getOptions(); 
        for (unsigned int i = 0; i < cop.length(); ++i) {
            LOG_TRACE(ApplicationFactory_impl, " RESOURCE OPTION: " << cop[i].id << " " << ossie::any_to_string(cop[i].value))
        }

        tempPid = execdev->executeLinked(executeName.string().c_str(), cop, component->getPopulatedExecParameters(), dep_seq);
    } catch( CF::InvalidFileName& _ex ) {
        ostringstream eout;
        eout << "InvalidFileName when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::Device::InvalidState& _ex ) {
        ostringstream eout;
        eout << "InvalidState when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidParameters& _ex ) {
        ostringstream eout;
        eout << "InvalidParameters when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid params: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidParms.length(); propIdx++){
            eout << "(" << _ex.invalidParms[propIdx].id << "," << ossie::any_to_string(_ex.invalidParms[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidOptions& _ex ) {
        ostringstream eout;
        eout << "InvalidOptions when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid options: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidOpts.length(); propIdx++){
            eout << "(" << _ex.invalidOpts[propIdx].id << "," << ossie::any_to_string(_ex.invalidOpts[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch (CF::ExecutableDevice::ExecuteFail& ex) {
        ostringstream eout;
        eout << "ExecuteFail when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with message: '" << ex.msg << "'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } CATCH_THROW_LOG_ERROR(
            ApplicationFactory_impl, "Caught an unexpected error when calling 'execute' on device with device id: '"
            << component->getAssignedDeviceId() << "' for component: '" << component->getName()
            << "' with component id: '" << component->getIdentifier() << "' "
            << " with implementation id: '" << implementation->getId() << "'"
            << " in waveform '" << _waveformContextName<<"'"
            << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__,
            CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Caught an unexpected error when calling 'execute' on device"));

    // handle pid output
    if (tempPid < 0) {
        ostringstream eout;
        eout << "Failed to 'execute' component for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EAGAIN, eout.str().c_str());
    } else {
        _application->setComponentPid(component->getIdentifier(), tempPid);
    }
}


void createHelper::applyApplicationAffinityOptions() {

  if ( _app_affinity.length() > 0  ) {
    // log deployments with application affinity 
    for ( uint32_t i=0; i < _app_affinity.length(); i++ ) {
      CF::DataType dt = _app_affinity[i];
      LOG_INFO(ApplicationFactory_impl, " Applying Application Affinity: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
    }
    
    //
    // Promote NIC affinity for all components deployed on the same device
    //
    boost::shared_ptr<ossie::DeviceNode> deploy_on_device;
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
      ossie::ComponentInfo * comp =  _requiredComponents[rc_idx];
      if ( comp->getNicAssignment() != "" ) {
        deploy_on_device = comp->getAssignedDevice();
      }
    }

    if ( deploy_on_device ) {
      for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        boost::shared_ptr<ossie::DeviceNode> dev= component->getAssignedDevice();
        // for matching device deployments then apply nic affinity settings
        if ( dev->identifier == deploy_on_device->identifier ) {
          component->mergeAffinityOptions( _app_affinity );
        }
      }
    }

  }

}
    


void createHelper::waitForComponentRegistration()
{
    // Wait for all components to be registered before continuing
    int componentBindingTimeout = _appFact._domainManager->getComponentBindingTimeout();
    LOG_TRACE(ApplicationFactory_impl, "Waiting " << componentBindingTimeout << "s for all components register");

    // Track only SCA-compliant components; non-compliant components will never
    // register with the application, nor do they need to be initialized
    std::set<std::string> expected_components;
    for (PlacementList::iterator ii = _requiredComponents.begin(); ii != _requiredComponents.end(); ++ii) {
        if ((*ii)->isScaCompliant()) {
            expected_components.insert((*ii)->getIdentifier());
        }
    }

    // Record current time, to measure elapsed time in the event of a failure
    time_t start = time(NULL);

    if (!_application->waitForComponents(expected_components, componentBindingTimeout)) {
        // For reference, determine much time has really elapsed.
        time_t elapsed = time(NULL)-start;
        LOG_ERROR(ApplicationFactory_impl, "Timed out waiting for component to bind to naming context (" << elapsed << "s elapsed)");
        ostringstream eout;
        for (unsigned int req_idx = 0; req_idx < _requiredComponents.size(); req_idx++) {
            if (expected_components.count(_requiredComponents[req_idx]->getIdentifier())) {
                eout << "Timed out waiting for component to register: '" << _requiredComponents[req_idx]->getName() << "' with component id: '" << _requiredComponents[req_idx]->getIdentifier()<< " assigned to device: '"<<_requiredComponents[req_idx]->getAssignedDeviceId()<<"'";
                break;
            }
        }
        eout << " in waveform '" << _waveformContextName<<"';";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    }
}

/* Initializes the components
 *  - Make sure internal lists are up to date
 *  - Ensure components have started and are bound to Naming Service
 *  - Initialize each component
 */
void createHelper::initializeComponents()
{
    // Install the different components in the system
    LOG_TRACE(ApplicationFactory_impl, "initializing " << _requiredComponents.size() << " waveform components")

    // Resize the _startSeq vector to the right size
    _startSeq.resize(_startOrderIds.size());
    
    CF::Components_var app_registeredComponents = _application->registeredComponents();

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];

        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Component is non SCA-compliant, continuing to next component");
            continue;
        }

        if (!component->isResource()) {
            LOG_TRACE(ApplicationFactory_impl, "Component is not a resource, continuing to next component");
            continue;
        }

        // Find the component on the Application
        const std::string componentId = component->getIdentifier();
        CF::Resource_var resource = CF::Resource::_nil();
        for (unsigned int comp_idx=0; comp_idx<app_registeredComponents->length(); comp_idx++) {
            std::string comp_id = std::string(app_registeredComponents[comp_idx].identifier);
            if (comp_id == componentId) {
                resource = ossie::corba::_narrowSafe<CF::Resource>(app_registeredComponents[comp_idx].componentObject);
                break;
            }
        }
        if (CORBA::is_nil(resource)) {
            ostringstream eout;
            eout << "CF::Resource::_narrow failed with Unknown Exception for component: '" << component->getName() << "' with component id: '" << componentId << " assigned to device: '"<<component->getAssignedDeviceId()<<"'";
            eout << " in waveform '" << _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        component->setResourcePtr(resource);

        int initAttempts=3;
        while ( initAttempts > 0 ) {
            initAttempts--;
            if ( ossie::corba::objectExists(resource) == true ) { initAttempts = 0; continue; }
            LOG_DEBUG(ApplicationFactory_impl, "Retrying component ping............ comp:" << component->getIdentifier() << " waveform: " << _waveformContextName);
            usleep(1000);
        }


        //
        // call resource's initializeProperties method to handle any properties required for construction
        //
        LOG_DEBUG(ApplicationFactory_impl, "Initialize properties for component " << componentId);
        if (component->isResource () && component->isConfigurable ()) {
          CF::Properties partialStruct = component->containsPartialStructConstruct();
          if (partialStruct.length() != 0) {
            ostringstream eout;
            eout << "Failed to 'configure' Assembly Controller: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout <<  "This component contains structure"<<partialStruct[0].id<<" with a mix of defined and nil values.";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
          }
          try {
            // Try to set the initial values for the component's properties
            resource->initializeProperties(component->getNonNilNonExecConstructProperties());
          } catch(CF::PropertySet::InvalidConfiguration& e) {
            ostringstream eout;
            eout << "Failed to initialize component properties: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout <<  "InvalidConfiguration with this info: <";
            eout << e.msg << "> for these invalid properties: ";
            for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
              eout << "(" << e.invalidProperties[propIdx].id << ",";
              eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
            }
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
          } catch(CF::PropertySet::PartialConfiguration& e) {
            ostringstream eout;
            eout << "Failed to initialize component properties: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout << "PartialConfiguration for these invalid properties: ";
            for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
              eout << "(" << e.invalidProperties[propIdx].id << ",";
              eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
            }
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
          } catch( ... ) {
            ostringstream eout;
            eout << "Failed to initialize component properties: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout << "'initializeProperties' failed with Unknown Exception";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
          }
        }

        LOG_TRACE(ApplicationFactory_impl, "Initializing component " << componentId);
        try {
            resource->initialize();
        } catch (const CF::LifeCycle::InitializeError& error) {
            // Dump the detailed initialization failure to the log
            ostringstream logmsg;
            logmsg << "Initializing component " << componentId << " failed";
            for (CORBA::ULong index = 0; index < error.errorMessages.length(); ++index) {
                logmsg << std::endl << error.errorMessages[index];
            }
            LOG_ERROR(ApplicationFactory_impl, logmsg.str());

            const std::string errmsg = "Unable to initialize component " + componentId;
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, errmsg.c_str());
        } catch (const CORBA::SystemException& exc) {
            ostringstream eout;
            eout << "CORBA " << exc._name() << " exception initializing component " << componentId;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        if (!component->isAssemblyController()) {
            // Try and find the right location in the vector to add the reference
            unsigned int pos = 0;
            for (unsigned int i = 0; i < _startOrderIds.size(); i++) {
                std::string currID = _startOrderIds[i];
                currID = currID.append(":");
                currID = currID.append(_waveformContextName);

                if (componentId == currID) {
                    break;
                }
                pos++;
            }

            // Add the reference if it belongs in the list
            if (pos < _startOrderIds.size()) {
                _startSeq[pos] = CF::Resource::_duplicate(resource);
            }
        }
    }
}

void createHelper::configureComponents()
{
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        
        if (component->isAssemblyController ()) {
            continue;
        }
        
        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Component is non SCA-compliant, continuing to next component");
            continue;
        }

        if (!component->isResource ()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Component in not resource, continuing to next component");
            continue;
        }

        // Assuming 1 instantiation for each componentplacement
        if (component->getNamingService ()) {

            CF::Resource_var _rsc = component->getResourcePtr();

            if (CORBA::is_nil(_rsc)) {
                LOG_ERROR(ApplicationFactory_impl, "Could not get component reference");
                ostringstream eout;
                eout << "Could not get component reference for component: '" 
                     << component->getName() << "' with component id: '" 
                     << component->getIdentifier() << " assigned to device: '"
                     << component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            if (component->isResource () && component->isConfigurable ()) {
                CF::Properties partialStruct = component->containsPartialStructConfig();
                bool partialWarn = false;
                if (partialStruct.length() != 0) {
                    ostringstream eout;
                    eout <<  "Component " << component->getIdentifier() << " contains structure"<< partialStruct[0].id <<" with a mix of defined and nil values. The behavior for the component is undefined";
                    LOG_WARN(ApplicationFactory_impl, eout.str());
                    partialWarn = true;
                }
                try {
                    // try to configure the component
                    _rsc->configure (component->getNonNilConfigureProperties());
                } catch(CF::PropertySet::InvalidConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to 'configure' component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout <<  "InvalidConfiguration with this info: <";
                    eout << e.msg << "> for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch(CF::PropertySet::PartialConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to instantiate component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "Failed to 'configure' component; PartialConfiguration for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch( ... ) {
                    ostringstream eout;
                    eout << "Failed to instantiate component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "'configure' failed with Unknown Exception";
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
                }
            }
        }
    }
    
    //  configure the assembly controller last
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        
        if (!component->isAssemblyController ()) {
            continue;
        }
        
        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Assembly controller is non SCA-compliant");
            break;
        }
        
        if (!component->isResource ()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Assembly controller is not resource");
            break;
        }
        
        // Assuming 1 instantiation for each componentplacement
        if (component->getNamingService ()) {
            
            CF::Resource_var _rsc = component->getResourcePtr();
            
            if (CORBA::is_nil(_rsc)) {
                LOG_ERROR(ApplicationFactory_impl, "Could not get Assembly Controller reference");
                ostringstream eout;
                eout << "Could not get reference for Assembly Controller: '" 
                << component->getName() << "' with component id: '" 
                << component->getIdentifier() << " assigned to device: '"
                << component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }
            
            if (component->isResource () && component->isConfigurable ()) {
                CF::Properties partialStruct = component->containsPartialStructConfig();
                bool partialWarn = false;
                if (partialStruct.length() != 0) {
                    ostringstream eout;
                    eout <<  "Component " << component->getIdentifier() << " contains structure"<< partialStruct[0].id <<" with a mix of defined and nil values. The behavior for the component is undefined";
                    LOG_WARN(ApplicationFactory_impl, eout.str());
                    partialWarn = true;
                }
                try {
                    // try to configure the component
                    _rsc->configure (component->getNonNilConfigureProperties());
                } catch(CF::PropertySet::InvalidConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to 'configure' Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout <<  "InvalidConfiguration with this info: <";
                    eout << e.msg << "> for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch(CF::PropertySet::PartialConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to instantiate Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "Failed to 'configure' Assembly Controller; PartialConfiguration for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch( ... ) {
                    ostringstream eout;
                    eout << "Failed to instantiate Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "'configure' failed with Unknown Exception";
                    if (partialWarn) {
                        eout << ". Note that this component contains a property with a mix of defined and nil values.";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
                }
            }
        }
        break;
    }
}

/* Connect the components
 *  - Connect the components
 */
void createHelper::connectComponents(std::vector<ConnectionNode>& connections, string base_naming_context)
{
    const std::vector<Connection>& _connection = _appFact._sadParser.getConnections ();

    // Create an AppConnectionManager to resolve and track all connections in the application.
    // NB: Use an auto_ptr instead of a bare pointer so that it will automatically be deleted
    //     in the event of a failure.
    using ossie::AppConnectionManager;
    std::auto_ptr<AppConnectionManager> connectionManager(new AppConnectionManager(_appFact._domainManager, this, this, base_naming_context));

    // Create all resource connections
    LOG_TRACE(ApplicationFactory_impl, "Establishing " << _connection.size() << " waveform connections")
    for (int c_idx = _connection.size () - 1; c_idx >= 0; c_idx--) {
        const Connection& connection = _connection[c_idx];

        LOG_TRACE(ApplicationFactory_impl, "Processing connection " << connection.getID());

        // Attempt to resolve the connection; if any connection fails, application creation fails.
        if (!connectionManager->resolveConnection(connection)) {
            LOG_ERROR(ApplicationFactory_impl, "Unable to make connection " << connection.getID());
            ostringstream eout;
            eout << "Unable to make connection " << connection.getID();
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }
    }

    // Copy all established connections into the connection array
    const std::vector<ConnectionNode>& establishedConnections = connectionManager->getConnections();
    std::copy(establishedConnections.begin(), establishedConnections.end(), std::back_inserter(connections));
}

createHelper::createHelper (
        const ApplicationFactory_impl& appFact,
        string                         waveformContextName,
        string                         baseNamingContext,
        CosNaming::NamingContext_ptr   waveformContext,
        CosNaming::NamingContext_ptr   domainContext ):

    _appFact(appFact),
    _allocationMgr(_appFact._domainManager->_allocationMgr),
    _allocations(*_allocationMgr),
    _isComplete(false),
    _application(0)
{
    this->_waveformContextName = waveformContextName;
    this->_baseNamingContext   = baseNamingContext;
    this->_waveformContext     = CosNaming::NamingContext::_duplicate(waveformContext);
    this->_domainContext     =  domainContext;
}

createHelper::~createHelper()
{
    if (!_isComplete) {
        _cleanupFailedCreate();
    }
    if (_application) {
        _application->_remove_ref();
    }
    for (PlacementList::iterator comp = _requiredComponents.begin(); comp != _requiredComponents.end(); ++comp) {
        delete (*comp);
    }
    _requiredComponents.clear();
}

void createHelper::_cleanupFailedCreate()
{
    if (_application) {
        _application->releaseComponents();
        _application->terminateComponents();
        _application->unloadComponents();
        _application->_cleanupActivations();
    }

    LOG_TRACE(ApplicationFactory_impl, "Removing all bindings from naming context");
    try {
      if ( _appFact._domainManager && !_appFact._domainManager->bindToDomain() ) {
        ossie::corba::unbindAllFromContext(_waveformContext);
      }
    } CATCH_LOG_WARN(ApplicationFactory_impl, "Could not unbind contents of naming context");

    CosNaming::Name DNContextname;
    DNContextname.length(1);
    DNContextname[0].id = _waveformContextName.c_str();
    LOG_TRACE(ApplicationFactory_impl, "Unbinding the naming context")
    try {
        _appFact._domainContext->unbind(DNContextname);
    } catch ( ... ) {
    }

    LOG_TRACE(ApplicationFactory_impl, "Destroying naming context");
    try {
        _waveformContext->destroy();
    } CATCH_LOG_WARN(ApplicationFactory_impl, "Could not destroy naming context");
}

/* Given a component instantiation id, returns the associated CORBA Resource pointer
 *  - Gets the Resource pointer for a particular component instantiation id
 */
CF::Resource_ptr createHelper::lookupComponentByInstantiationId(const std::string& identifier)
{
    ossie::ComponentInfo* component = findComponentByInstantiationId(identifier);
    if (component) {
        return component->getResourcePtr();
    }

    return CF::Resource::_nil();
}

/* Given a component instantiation id, returns the associated CORBA Device pointer
 *  - Gets the Device pointer for a particular component instantiation id
 */
CF::Device_ptr createHelper::lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device that loaded component " << componentId);

    ossie::ComponentInfo* component = findComponentByInstantiationId(componentId);
    if (!component) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    boost::shared_ptr<ossie::DeviceNode> device = component->getAssignedDevice();
    if (!device) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] Component not assigned to device");
        return CF::Device::_nil();
    }
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << device->identifier);
    return CF::Device::_duplicate(device->device);
}


/* Given a component instantiation id and uses id, returns the associated CORBA Device pointer
 *  - Gets the Device pointer for a particular component instantiation id and uses id
 */
CF::Device_ptr createHelper::lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device used by component " << componentId);
    ossie::ComponentInfo* component = findComponentByInstantiationId(componentId.c_str());
    if (!component) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Uses id " << usesId);
    const ossie::UsesDeviceInfo* usesdevice = component->getUsesDeviceById(usesId);
    if (!usesdevice) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    std::string deviceId = usesdevice->getAssignedDeviceId();
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << deviceId);

    return find_device_from_id(deviceId.c_str());
}

CF::Device_ptr createHelper::lookupDeviceUsedByApplication(const std::string& usesRefId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device used by application, Uses Id: " << usesRefId);


    const ossie::UsesDeviceInfo* usesdevice = _appInfo.getUsesDeviceById(usesRefId);
    if (!usesdevice) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    std::string deviceId = usesdevice->getAssignedDeviceId();
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << deviceId);

    return find_device_from_id(deviceId.c_str());
}

