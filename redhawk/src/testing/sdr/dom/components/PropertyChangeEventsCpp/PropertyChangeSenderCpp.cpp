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

#include "PropertyChangeSenderCpp.h"


PREPARE_LOGGING(PropertyChangeSenderCpp);

PropertyChangeSenderCpp::PropertyChangeSenderCpp (const char *uuid, const char *label) :
    Resource_impl(uuid, label)
{
    
    addProperty(somevalue,
               "somevalue",
               "somevalue",
               "readwrite",
               "null",
               "external",
               "configure");
    
    addProperty(myprop,
               "myprop",
               "myprop",
               "readwrite",
               "null",
               "external",
               "configure,event");
    
    addProperty(anotherprop,
               "anotherprop",
               "anotherprop",
               "readwrite",
               "null",
               "external",
               "configure,event");
               
    addProperty(seqprop,
               "seqprop",
               "",
               "readwrite",
               "null",
               "external",
               "configure,event");

    addProperty(some_struct,
                some_struct_struct(), 
               "some_struct",
               "",
               "readwrite",
               "",
               "external",
               "configure,event");
            
    structseq_prop.resize(0);
    addProperty(structseq_prop,
               "structseq_prop",
               "",
               "readwrite",
               "",
               "external",
               "configure,event");
    
    propEvent = new PropertyEventSupplier("propEvent");
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(propEvent);
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("somevalue"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("myprop"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("anotherprop"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("seqprop"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("some_struct"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("structseq_prop"));
    this->registerPropertyChangePort(propEvent);
    propEvent->_remove_ref();

}

PropertyChangeSenderCpp::~PropertyChangeSenderCpp (void)
{

}

CORBA::Object_ptr PropertyChangeSenderCpp::getPort (const char* name) throw (CF::PortSupplier::UnknownPort, CORBA::SystemException)
{
    if (strcmp(name, "propEvent") != 0) {
        throw CF::PortSupplier::UnknownPort();
    }

    return propEvent->_this();
}

void PropertyChangeSenderCpp::releaseObject (void) throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(propEvent);
    root_poa->deactivate_object(oid);

    Resource_impl::releaseObject();
}

void PropertyChangeSenderCpp::start (void) throw (CF::Resource::StartError)
{
    some_struct_struct tmp = (*&structseq_prop)[0];
    double a_number = (*&some_struct).some_number;
}
