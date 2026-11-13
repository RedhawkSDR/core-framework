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
#include <cctype>
#include <algorithm>

#include <ossie/CF/cf.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/debug.h>

class DomainManager_impl;

class ContainerRegistrar : public virtual POA_CF::ApplicationRegistrar {

public:
    ContainerRegistrar(boost::condition_variable *_regCondition, boost::condition_variable *_regCondition_post, CF::Application_ptr *app, CF::DomainManager_ptr *domMgr) {
        _rC = _regCondition;
        _rC_post = _regCondition_post;
        _app = app;
        _domMgr = domMgr;
    };
    void registerComponent(const char * Name, CF::Resource_ptr obj) {
        CF::Application_ptr tmp_app = CF::Application::_narrow(obj);
        *_app = tmp_app;
        if (_rC) {
            _rC->notify_all();
        } else {
            if (_rC_post) {
                _rC_post->notify_all();
            }
        }
    };
    void resetCondition() {
        _rC = NULL;
    }
    void resetCondition_post() {
        _rC_post = NULL;
    }

    CF::Application_ptr app() {return CF::Application::_duplicate(*_app);};
    CF::DomainManager_ptr domMgr() {return CF::DomainManager::_duplicate(*_domMgr);};

    void bind(const CosNaming::Name &Name, CORBA::Object_ptr obj) {};
    void unbind(const CosNaming::Name &Name) {};
    void rebind(const CosNaming::Name &Name, CORBA::Object_ptr obj) {};

    void bind_context(const CosNaming::Name &Name, CosNaming::NamingContext_ptr obj) {};
    void rebind_context(const CosNaming::Name &Name, CosNaming::NamingContext_ptr obj) {};
    CORBA::Object_ptr resolve(const CosNaming::Name &Name) {return CORBA::Object::_nil();};
    CosNaming::NamingContext_ptr new_context() {return CosNaming::NamingContext::_nil();};
    CosNaming::NamingContext_ptr bind_new_context(const CosNaming::Name &Name) {return CosNaming::NamingContext::_nil();};
    void destroy() {};
    void list(CORBA::ULong length, CosNaming::BindingList_out out, CosNaming::BindingIterator_out iterator) {};

protected:
    boost::condition_variable *_rC;
    boost::condition_variable *_rC_post;
    CF::Application_ptr *_app;
    CF::DomainManager_ptr *_domMgr;

};

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
                                const CF::DeviceAssignmentSequence& deviceAssignments);

    rh_logger::LoggerPtr returnLogger() const {
        return _appFactoryLog;
    }

    void setLogger(rh_logger::LoggerPtr logptr)
    {
        _appFactoryLog = logptr;
    };

    // getters for attributes
    char* name () {
        return CORBA::string_dup(_name.c_str());
    }

    char* identifier () {
        return CORBA::string_dup(_identifier.c_str());
    }

    char* softwareProfile () {
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
