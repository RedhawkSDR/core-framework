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

 
/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include "port_impl.h"
#include "BasicAC_cpp_impl1.h"
 

// ----------------------------------------------------------------------------------------
// CF_Resource_Out_i definition
// ----------------------------------------------------------------------------------------
CF_Resource_Out_i::CF_Resource_Out_i(std::string port_name, BasicAC_cpp_impl1_base *_parent) :
Port_Uses_base_impl(port_name)
{
    parent = static_cast<BasicAC_cpp_impl1_i *> (_parent);
    recConnectionsRefresh = false;
    recConnections.length(0);
}

CF_Resource_Out_i::~CF_Resource_Out_i()
{
}


void CF_Resource_Out_i::initialize()
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->initialize();
            } catch(...) {
                std::cout << "Call to initialize by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

void CF_Resource_Out_i::releaseObject()
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->releaseObject();
            } catch(...) {
                std::cout << "Call to releaseObject by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

void CF_Resource_Out_i::runTest(unsigned long testid, CF::Properties& testValues)
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->runTest(testid, testValues);
            } catch(...) {
                std::cout << "Call to runTest by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

void CF_Resource_Out_i::configure(const CF::Properties& configProperties)
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->configure(configProperties);
            } catch(...) {
                std::cout << "Call to configure by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

void CF_Resource_Out_i::query(CF::Properties& configProperties)
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->query(configProperties);
            } catch(...) {
                std::cout << "Call to query by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

CORBA::Object_ptr CF_Resource_Out_i::getPort(const char* name)
{
    CORBA::Object_ptr retval = 0;
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval =((*i).first)->getPort(name);
            } catch(...) {
                std::cout << "Call to getPort by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return retval;
}

void CF_Resource_Out_i::start()
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->start();
            } catch(...) {
                std::cout << "Call to start by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}

void CF_Resource_Out_i::stop()
{
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->stop();
            } catch(...) {
                std::cout << "Call to stop by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return;
}
std::string CF_Resource_Out_i::identifier()
{
    std::string retval("");
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    
    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->identifier();
            } catch(...) {
                std::cout << "Call to identifier by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return retval;
}

CORBA::Boolean CF_Resource_Out_i::started()
{
    CORBA::Boolean retval = 0;
    std::vector < std::pair < CF::Resource_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    
    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->started();
            } catch(...) {
                std::cout << "Call to started by CF_Resource_Out_i failed" << std::endl;
            }
        }
    }

    return retval;
}


