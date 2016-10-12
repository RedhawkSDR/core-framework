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

#include <string>
#include <cstdlib>
#include "ossie/CorbaUtils.h"

using namespace ossie::corba;

PREPARE_LOGGING(POACreator);

CORBA::Boolean POACreator::unknown_adapter (PortableServer::POA_ptr parent, const char* name)
    throw (CORBA::SystemException)
{
    bool install_adapter_activator = false;

    CORBA::String_var parent_name_obj = parent->the_name();
    const std::string parent_name = static_cast<char*>(parent_name_obj);
    const std::string child_name = name;

    PortableServer::LifespanPolicy_var lifespan;
    PortableServer::IdAssignmentPolicy_var idassignment;
    PortableServer::ThreadPolicy_var thread;

    // Set policies and establish the hierarchy for POAs.
    if (child_name == "DomainManager") {
        if (parent_name != "RootPOA") {
            return 0;
        }
        if (isPersistenceEnabled()) {
            lifespan = parent->create_lifespan_policy(PortableServer::PERSISTENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::USER_ID);
        } else {
            lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        }
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
        install_adapter_activator = true;
    } else if (child_name == "DeviceManager") {
        if (parent_name != "RootPOA") {
            return 0;
        }
        lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
        idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
        install_adapter_activator = true;
    } else if (child_name == "ApplicationFactories") {
        if (parent_name != "DomainManager") {
            return 0;
        }
        if (isPersistenceEnabled()) {
            lifespan = parent->create_lifespan_policy(PortableServer::PERSISTENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::USER_ID);
        } else {
            lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        }
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
    } else if (child_name == "Applications") {
        if (parent_name != "DomainManager") {
            return 0;
        }
        if (isPersistenceEnabled()) {
            lifespan = parent->create_lifespan_policy(PortableServer::PERSISTENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::USER_ID);
        } else {
            lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
            idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        }
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
    } else if (child_name == "Files") {
        if (parent_name != "RootPOA") {
            return 0;
        }
        lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
        idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
#if ENABLE_EVENTS
    } else if (child_name == "EventChannels") {
        if (parent_name != "DomainManager") {
            return 0;
        }
        lifespan = parent->create_lifespan_policy(PortableServer::TRANSIENT);
        idassignment = parent->create_id_assignment_policy(PortableServer::SYSTEM_ID);
        thread = parent->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
#endif
    } else {
        return 0;
    }

    // Build a policy list from the individual policies.
    CORBA::PolicyList policy_list;
    policy_list.length(3);
    policy_list[0] = PortableServer::LifespanPolicy::_duplicate(lifespan);
    policy_list[1] = PortableServer::IdAssignmentPolicy::_duplicate(idassignment);
    policy_list[2] = PortableServer::ThreadPolicy::_duplicate(thread);
    lifespan->destroy();
    idassignment->destroy();
    thread->destroy();

    PortableServer::POAManager_var poa_mgr = parent->the_POAManager();

    try {
        LOG_TRACE(POACreator, "Creating POA " << name);
        PortableServer::POA_var child = parent->create_POA(name, poa_mgr, policy_list);
        if (install_adapter_activator) {
            PortableServer::AdapterActivator_var tmpObj = this->_this();
            child->the_activator(tmpObj);
        }
    } catch (const PortableServer::POA::AdapterAlreadyExists &) {
        return 0;
    } catch (const PortableServer::POA::InvalidPolicy &) {
        abort(); // design error
    }

    return 1;
}
