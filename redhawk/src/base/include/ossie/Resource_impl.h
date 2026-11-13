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


#ifndef RESOURCE_IMPL_H
#define RESOURCE_IMPL_H

#include <string>
#include <map>
#include <boost/scoped_ptr.hpp>
#include "Logging_impl.h"
#include "Port_impl.h"
#include "LifeCycle_impl.h"
#include "PortSet_impl.h"
#include "PropertySet_impl.h"
#include "TestableObject_impl.h"
#include "ossie/logging/loghelpers.h"
#include "ossie/ossieSupport.h"
#include "ossie/prop_helpers.h"
#include "ossie/Containers.h"
#include "ossie/PropertyMap.h"
#include "ossie/Autocomplete.h"

class Resource_impl: 
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    public virtual POA_CF::Resource, 
#endif
    public PropertySet_impl, public PortSet_impl, public LifeCycle_impl, public TestableObject_impl, public Logging_impl
{
    ENABLE_LOGGING

public:
    template<class T>
    static void start_component(T*& component, int argc, char* argv[]) {
        // Initialize the component pointer to quiet warnings--it's not obvious
        // to the compiler that it eventually gets set before being read
        component = 0;

        // Bind the constructor adapter make_component() with the template type
        // and the component argument (explicitly by reference, so that the
        // caller gets the updated value) and pass along to the real
        // implementation
        start_component(boost::bind(&Resource_impl::make_component<T>,boost::ref(component),_1,_2), argc, argv);
    }

    virtual ~Resource_impl ();
    Resource_impl (const char* _uuid);
    Resource_impl (const char* _uuid, const char *label);

    /*
     * REDHAWK constructor. All properties are initialized before this constructor is called
     */
    virtual void constructor ();

    void setParentId( const std::string &parentid ) { _parent_id = parentid; };


    void start ();
    void stop ();
    void initialize ();
    void releaseObject();
    char* identifier ();
    CORBA::Boolean started();
    char* softwareProfile ();
    CF::StringSequence* getNamedLoggers();
    
    virtual void run ();
    virtual void halt ();
    
    virtual void setCurrentWorkingDirectory(std::string& cwd);
    virtual std::string& getCurrentWorkingDirectory();
    
    virtual void setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior, std::string &nic);
    /*
     * Return a pointer to the Domain Manager that the Resource is deployed on
     */
    redhawk::DomainManagerContainer* getDomainManager();

    /*
     * Register a function for notification when this Resource is released
     */
    template <class Func>
    void addReleaseListener(Func func)
    {
        _resourceReleased.add(func);
    }

    /*
     * Register a member function for notification when this Resource is released
     */
    template <class Target, class Func>
    void addReleaseListener(Target target, Func func)
    {
        _resourceReleased.add(target, func);
    }

    const std::string& getIdentifier() const;

    /*
     * Globally unique identifier for this Resource
     */
    std::string _identifier;
    /*
     * Name used to bind this Resource to the Naming Service
     */
    std::string naming_service_name;
    std::string _parent_id;

    void setLogger(rh_logger::LoggerPtr logptr);

protected:
    virtual void setCommandLineProperty(const std::string& id, const redhawk::Value& value);

    void setDomainManager(CF::DomainManager_ptr domainManager);

    const std::string& getDeploymentRoot() const;

    /*
     * Boolean describing whether or not this Resource is started
     */
    bool _started;
    /*
     * Filename for the Resource's SPD file
     */
    std::string _softwareProfile;
    
    omni_mutex component_running_mutex;
    omni_condition component_running;

private:
    Resource_impl(); // No default constructor
    Resource_impl(Resource_impl&);  // No copying

    // Adapter template function for component constructors. This is the only
    // part of component creation that requires type-specific knowledge.
    template <class T>
    static Resource_impl* make_component(T*& component, const std::string& identifier, const std::string& name)
    {
        component = new T(identifier.c_str(), name.c_str());
        return component;
    }

    rh_logger::LoggerPtr _resourceLog;

    // Generic implementation of start_component, taking a function pointer to
    // a component constructor (via make_component).
    typedef boost::function<Resource_impl* (const std::string&, const std::string&)> ctor_type;
    static void start_component(ctor_type ctor, int argc, char* argv[]);
    std::string currentWorkingDirectory;
    boost::scoped_ptr<redhawk::DomainManagerContainer> _domMgr;
    bool _initialized;
    std::string _deploymentRoot;

    ossie::notification<void (Resource_impl*)> _resourceReleased;

public:
    static Resource_impl* create_component(ctor_type, const CF::Properties& parameters);
};
#endif
