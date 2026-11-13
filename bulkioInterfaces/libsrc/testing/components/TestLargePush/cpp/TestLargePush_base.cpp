/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "TestLargePush_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

TestLargePush_base::TestLargePush_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    serviceThread(0)
{
    construct();
}

void TestLargePush_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
    dataChar = new bulkio::OutCharPort("dataChar");
    oid = ossie::corba::RootPOA()->activate_object(dataChar);
    dataFile = new bulkio::OutFilePort("dataFile");
    oid = ossie::corba::RootPOA()->activate_object(dataFile);
    dataShort = new bulkio::OutShortPort("dataShort");
    oid = ossie::corba::RootPOA()->activate_object(dataShort);
    dataUlong = new bulkio::OutULongPort("dataUlong");
    oid = ossie::corba::RootPOA()->activate_object(dataUlong);
    dataUlongLong = new bulkio::OutULongLongPort("dataUlongLong");
    oid = ossie::corba::RootPOA()->activate_object(dataUlongLong);
    dataUshort = new bulkio::OutUShortPort("dataUshort");
    oid = ossie::corba::RootPOA()->activate_object(dataUshort);
    dataXML = new bulkio::OutXMLPort("dataXML");
    oid = ossie::corba::RootPOA()->activate_object(dataXML);
    dataLong = new bulkio::OutLongPort("dataLong");
    oid = ossie::corba::RootPOA()->activate_object(dataLong);
    dataLongLong = new bulkio::OutLongLongPort("dataLongLong");
    oid = ossie::corba::RootPOA()->activate_object(dataLongLong);
    dataOctet = new bulkio::OutOctetPort("dataOctet");
    oid = ossie::corba::RootPOA()->activate_object(dataOctet);
    dataFloat = new bulkio::OutFloatPort("dataFloat");
    oid = ossie::corba::RootPOA()->activate_object(dataFloat);
    dataSDDS = new bulkio::OutSDDSPort("dataSDDS");
    oid = ossie::corba::RootPOA()->activate_object(dataSDDS);
    dataDouble = new bulkio::OutDoublePort("dataDouble");
    oid = ossie::corba::RootPOA()->activate_object(dataDouble);

    registerOutPort(dataChar, dataChar->_this());
    registerOutPort(dataFile, dataFile->_this());
    registerOutPort(dataShort, dataShort->_this());
    registerOutPort(dataUlong, dataUlong->_this());
    registerOutPort(dataUlongLong, dataUlongLong->_this());
    registerOutPort(dataUshort, dataUshort->_this());
    registerOutPort(dataXML, dataXML->_this());
    registerOutPort(dataLong, dataLong->_this());
    registerOutPort(dataLongLong, dataLongLong->_this());
    registerOutPort(dataOctet, dataOctet->_this());
    registerOutPort(dataFloat, dataFloat->_this());
    registerOutPort(dataSDDS, dataSDDS->_this());
    registerOutPort(dataDouble, dataDouble->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void TestLargePush_base::initialize() 
{
}

void TestLargePush_base::start() 
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<TestLargePush_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void TestLargePush_base::stop() 
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

CORBA::Object_ptr TestLargePush_base::getPort(const char* _id) 
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {
    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void TestLargePush_base::releaseObject() 
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    delete(dataChar);
    delete(dataFile);
    delete(dataShort);
    delete(dataUlong);
    delete(dataUlongLong);
    delete(dataUshort);
    delete(dataXML);
    delete(dataLong);
    delete(dataLongLong);
    delete(dataOctet);
    delete(dataFloat);
    delete(dataSDDS);
    delete(dataDouble);

    Resource_impl::releaseObject();
}

void TestLargePush_base::loadProperties()
{
    addProperty(numSamples,
                3000000,
                "numSamples",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


