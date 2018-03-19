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

#ifndef APPLICATIONFACTORY_H
#define APPLICATIONFACTORY_H

#include <string>

#include <ossie/CF/cf.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/debug.h>

class DomainManager_impl;

class ApplicationFactory_impl: public virtual POA_CF::ApplicationFactory
{
    ENABLE_LOGGING

private:
    
    ApplicationFactory_impl();  // Node default constructor
    ApplicationFactory_impl(ApplicationFactory_impl&);  // No copying

    std::string _name;
    std::string _identifier;
    std::string _softwareProfile;

    CosNaming::NamingContext_var _domainContext;

    ossie::SoftwareAssembly _sadParser;
    CF::FileManager_var     _fileMgr;

    boost::mutex _pendingCreateLock;

    std::string         _domainName;
    DomainManager_impl* _domainManager;

    unsigned short _lastWaveformUniqueId;
    // Support for creating a new waveform naming context
    std::string getWaveformContextName(std::string name);
    std::string getBaseWaveformContext(std::string waveform_context);
    rh_logger::LoggerPtr _appFactoryLog;

public:
    ApplicationFactory_impl (const std::string& softwareProfile, 
                             const std::string& domainName, 
                             DomainManager_impl* domainManager);
    ~ApplicationFactory_impl ();

    CF::Application_ptr create (const char* name,
                                const CF::Properties& initConfiguration,
                                const CF::DeviceAssignmentSequence& deviceAssignments)
    throw (CF::ApplicationFactory::InvalidInitConfiguration,
           CF::ApplicationFactory::CreateApplicationRequestError,
           CF::ApplicationFactory::CreateApplicationInsufficientCapacityError,
           CF::ApplicationFactory::CreateApplicationError,
           CORBA::SystemException);

    rh_logger::LoggerPtr returnLogger() const {
        return _appFactoryLog;
    }

    void setLogger(rh_logger::LoggerPtr logptr)
    {
        _appFactoryLog = logptr;
    };

    // getters for attributes
    char* name () throw (CORBA::SystemException) {
        return CORBA::string_dup(_name.c_str());
    }

    char* identifier () throw (CORBA::SystemException) {
        return CORBA::string_dup(_identifier.c_str());
    }

    char* softwareProfile () throw (CORBA::SystemException) {
        return CORBA::string_dup(_softwareProfile.c_str());
    }

    const std::string& getIdentifier () const;
    const std::string& getName () const;
    const std::string& getSoftwareProfile() const;

    // allow createHelper to have access to ApplicationFactory_impl
    friend class createHelper;
    friend class ScopedAllocations;
};
#endif
