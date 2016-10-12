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
#include "Logging_impl.h"
#include "Port_impl.h"
#include "LifeCycle_impl.h"
#include "PortSupplier_impl.h"
#include "PropertySet_impl.h"
#include "TestableObject_impl.h"
#include "ossie/logging/loghelpers.h"
#include "ossie/ossieSupport.h"
#include "ossie/prop_helpers.h"

class Resource_impl: public virtual POA_CF::Resource, public PropertySet_impl, public PortSupplier_impl, public LifeCycle_impl, public TestableObject_impl, public Logging_impl
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

    Resource_impl (const char* _uuid);
    Resource_impl (const char* _uuid, const char *label);


    void setParentId( const std::string &parentid ) { _parent_id = parentid; };


    void start () throw (CF::Resource::StartError, CORBA::SystemException);
    void stop () throw (CF::Resource::StopError, CORBA::SystemException);
    void releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError);
    char* identifier () throw (CORBA::SystemException);
    CORBA::Boolean started() throw (CORBA::SystemException);
    char* softwareProfile () throw (CORBA::SystemException);
    
    virtual void run ();
    virtual void halt ();
    
    virtual void setCurrentWorkingDirectory(std::string& cwd);
    virtual std::string& getCurrentWorkingDirectory();
    
    void setAdditionalParameters(std::string softwareProfile);

    std::string _identifier;
    std::string naming_service_name;
    std::string _parent_id;

protected:
    bool _started;
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

    // Generic implementation of start_component, taking a function pointer to
    // a component constructor (via make_component).
    typedef boost::function<Resource_impl* (const std::string&, const std::string&)> ctor_type;
    static void start_component(ctor_type ctor, int argc, char* argv[]);

    std::string currentWorkingDirectory;
};
#endif
