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
#include <string.h>

#include "ossie/Service_impl.h"
#include "ossie/CorbaUtils.h"

PREPARE_CF_LOGGING(Service_impl)

void Service_impl::initResources (char* devMgr_ior, char* name)
{
    _name = name;
    _devMgr_ior = devMgr_ior;
    initialConfiguration = true;
    _baseLog = rh_logger::Logger::getNewHierarchy(_name);
    _serviceLog = _baseLog->getChildLogger("Service", "system");
}

Service_impl::Service_impl (char* devMgr_ior, char* _name) :
    component_running_mutex(),
    component_running(&component_running_mutex)
{
    RH_TRACE(_serviceLog, "Constructing Device")
    initResources(devMgr_ior, _name);
    RH_TRACE(_serviceLog, "Done Constructing Device")
}

void  Service_impl::resolveDeviceManager ()
{
    RH_TRACE(_serviceLog, "entering resolveDeviceManager()");
    _deviceManager = CF::DeviceManager::_nil();
    CORBA::Object_var obj = ossie::corba::Orb()->string_to_object(_devMgr_ior.c_str());
    if (CORBA::is_nil(obj)) {
        RH_ERROR(_serviceLog, "Invalid device manager IOR");
        exit(-1);
    }
    _deviceManager = CF::DeviceManager::_narrow(obj);
    if (CORBA::is_nil(_deviceManager)) {
        RH_ERROR(_serviceLog, "Could not narrow device manager IOR");
        exit(-1);
    }
    this->_devMgr = new redhawk::DeviceManagerContainer(_deviceManager);
    if (!CORBA::is_nil(_deviceManager)) {
        this->_domMgr = new redhawk::DomainManagerContainer(_deviceManager->domMgr());
        return;
    }
    RH_TRACE(_serviceLog, "leaving resolveDeviceManager()");
}

void  Service_impl::registerServiceWithDevMgr ()
{
    // code generator fills this function in the implementation
}

void  Service_impl::run ()
{
    RH_TRACE(_serviceLog, "handling CORBA requests");
    component_running.wait();
    RH_TRACE(_serviceLog, "leaving run()");
}

void  Service_impl::halt ()
{
    RH_DEBUG(_serviceLog, "Halting Service")
    component_running.signal();
    RH_TRACE(_serviceLog, "Done sending service running signal");
}

void Service_impl::terminateService ()
{
    // code generator fills this function in the implementation
}

Service_impl::~Service_impl ()
{
    if (this->_devMgr != NULL)
        delete this->_devMgr;
    if (this->_domMgr != NULL)
        delete this->_domMgr;
}

// compareAnys function compares both Any type inputs
// returns FIRST_BIGGER if the first argument is bigger
// returns SECOND_BIGGER is the second argument is bigger
// and BOTH_EQUAL if they are equal
Service_impl::AnyComparisonType Service_impl::compareAnys (CORBA::Any& first,
                                                         CORBA::Any& second)
{
    CORBA::TypeCode_var tc1 = first.type ();
    CORBA::TypeCode_var tc2 = second.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    case CORBA::tk_long: {
        CORBA::Long frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    case CORBA::tk_short: {
        CORBA::Short frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    default:
        return UNKNOWN;
    }

    return UNKNOWN;
}


// compareAnyToZero function compares the any type input to zero
// returns POSITIVE if the first argument is bigger
// returns NEGATIVE is the second argument is bigger
// and ZERO if they are equal
Service_impl::AnyComparisonType Service_impl::compareAnyToZero (CORBA::Any& first)
{
    CORBA::TypeCode_var tc1 = first.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    case CORBA::tk_long: {
        CORBA::Long frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    case CORBA::tk_short: {
        CORBA::Short frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    default:
        return UNKNOWN;
    }

    return UNKNOWN;
}
