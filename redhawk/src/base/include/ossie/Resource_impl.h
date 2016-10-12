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
#include "ossiecf.h"
#include "Port_impl.h"
#include "LifeCycle_impl.h"
#include "PortSupplier_impl.h"
#include "PropertySet_impl.h"
#include "TestableObject_impl.h"
#include "ossie/ossieSupport.h"
#include "ossie/prop_helpers.h"

class OSSIECF_API Resource_impl: public virtual POA_CF::Resource, public PropertySet_impl, public PortSupplier_impl, public LifeCycle_impl, public TestableObject_impl

{
    ENABLE_LOGGING

public:
    template<class T>
    static void start_component(T*& component, int argc, char* argv[]) {
        std::string naming_context_ior;
        std::string component_identifier;
        std::string name_binding;
        const char* logging_config_uri = 0;
        int debug_level = 3; // Default level is INFO.
        bool standAlone = false;
        std::map<std::string, char*> execparams;

        // Parse execparams.
        for (int i=0; i < argc; i++) {
            if (strcmp("NAMING_CONTEXT_IOR", argv[i]) == 0) {
                naming_context_ior = argv[++i];
            } else if (strcmp("COMPONENT_IDENTIFIER", argv[i]) == 0) {
                component_identifier = argv[++i];
            } else if (strcmp("NAME_BINDING", argv[i]) == 0) {
                name_binding = argv[++i];
            } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
                logging_config_uri = argv[++i];
            } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
                debug_level = atoi(argv[++i]);
            } else if (strcmp("-i", argv[i]) == 0) {
                standAlone = true;
            } else if (i > 0) {  // any other argument besides the first one is part of the execparams
                std::string paramName = argv[i];
                execparams[paramName] = argv[++i];
            }
        }

        if (standAlone) {
            if (component_identifier.empty()) {
                component_identifier = ossie::generateUUID();
            }
            if (name_binding.empty()) {
                name_binding = "";
            }
        } else {
            if (naming_context_ior.empty()) {
                std::cout<<std::endl<<"usage: "<<argv[0]<<" [options] [execparams]"<<std::endl<<std::endl;
                std::cout<<"The set of execparams is defined in the .prf for the component"<<std::endl;
                std::cout<<"They are provided as arguments pairs ID VALUE, for example:"<<std::endl;
                std::cout<<"     "<<argv[0]<<" INT_PARAM 5 STR_PARAM ABCDED"<<std::endl<<std::endl;
                std::cout<<"Options:"<<std::endl;
                std::cout<<"     -i,--interactive           Run the component in interactive test mode"<<std::endl<<std::endl;
                exit(-1);
            }
        }

        // The ORB must be initialized before configuring logging, which may use
        // CORBA to get its configuration file.
        CORBA::ORB_ptr orb = ossie::corba::CorbaInit(argc, argv);

        // Configure logging.
        ossie::configureLogging(logging_config_uri, debug_level);

        // Create the servant.
        LOG_TRACE(Resource_impl, "Creating component with identifier '" << component_identifier << "'");
        component = new T(component_identifier.c_str(), name_binding.c_str());

        // setting all the execparams passed as argument, this method resides in the Resource_impl class
        component->setExecparamProperties(execparams);

        // Activate the component servant.
        LOG_TRACE(Resource_impl, "Activating component object");
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(component);
        CF::Resource_var resource = component->_this();

        // Get the application naming context and bind the component into it.
        if (!naming_context_ior.empty()) {
            LOG_TRACE(Resource_impl, "Binding component to application context with name '" << name_binding << "'");
            CORBA::Object_var applicationObject = orb->string_to_object(naming_context_ior.c_str());
            CosNaming::NamingContext_ptr applicationContext = CosNaming::NamingContext::_narrow(applicationObject);
            ossie::corba::bindObjectToContext(resource, applicationContext, name_binding);
        } else {
            if (standAlone) {
                std::cout<<orb->object_to_string(resource)<<std::endl;
            }
        }

        LOG_TRACE(Resource_impl, "Entering component run loop");
        component->run();
        LOG_TRACE(Resource_impl, "Component run loop terminated");

        LOG_TRACE(Resource_impl, "Deleting component");
        component->_remove_ref();
        LOG_TRACE(Resource_impl, "Shutting down ORB");
        ossie::corba::OrbShutdown(true);
    }

protected:
    typedef std::map<std::string, Port_Uses_base_impl *>       RH_UsesPortMap;
    typedef std::map<std::string, Port_Provides_base_impl *>   RH_ProvidesPortMap;

    RH_UsesPortMap  outPorts;
    std::map<std::string, CF::Port_var> outPorts_var;
    RH_ProvidesPortMap inPorts;
    CORBA::Boolean _started;

    void registerInPort(Port_Provides_base_impl *port);
    void registerOutPort(Port_Uses_base_impl *port, CF::Port_ptr ref);

    void releaseInPorts();
    void releaseOutPorts();
    void deactivateOutPorts();
    void deactivateInPorts();



    omni_mutex component_running_mutex;
    omni_condition component_running;

public:
    Resource_impl (const char* _uuid);
    Resource_impl (const char* _uuid, const char *label);


    void start () throw (CF::Resource::StartError, CORBA::SystemException);
    void stop () throw (CF::Resource::StopError, CORBA::SystemException);
    void releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError);
    char* identifier () throw (CORBA::SystemException);
    CORBA::Boolean started() throw (CORBA::SystemException);

    virtual void run ();
    virtual void halt ();
    
    std::string _identifier;
    std::string naming_service_name;

private:
    Resource_impl(); // No default constructor
    Resource_impl(Resource_impl&);  // No copying
};
#endif
