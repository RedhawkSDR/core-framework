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

#include "CPP_Ports_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

         Source: CPP_Ports.spd.xml
         Generated on: Mon Dec 10 14:36:40 EST 2012
         Redhawk IDE
         Version:@buildLabel@
         Build id: @buildId@

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/
PREPARE_LOGGING(CPP_Ports_base);

CPP_Ports_base::MySddsCallback::MySddsCallback( CPP_Ports_base &p ) :
parent(p)
{}

char *CPP_Ports_base::MySddsCallback::attach( const BULKIO::SDDSStreamDefinition& stream, const char* userid )
      throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError)
{

  std::string aid = ossie::generateUUID();
  std::cout << "TestRCV::ATTACH USERID: " << userid << " ATTACH ID: " << aid << std::endl;

  //return aid.str();
  return  const_cast<char *>(aid.c_str());
}

void CPP_Ports_base::MySddsCallback::detach( const char* attachId) {

  std::cout << "TestRCV::DETACH  ATTACH ID: " << attachId << std::endl;
}

CPP_Ports_base::CPP_Ports_base(const char *uuid, const char *label) :
  Resource_impl(uuid, label),
  cb(*this),
  serviceThread(0)
{
    construct();

    if ( __logger  ) {
      __logger->setLevel( rh_logger::Level::getInfo()   );
    }
}

void CPP_Ports_base::construct()
{
  Resource_impl::_started = false;
  loadProperties();
  serviceThread = 0;

  PortableServer::ObjectId_var oid;
  dataCharIn = new bulkio::InCharPort("dataCharIn" );
  oid = ossie::corba::RootPOA()->activate_object(dataCharIn );
  dataCharOut = new bulkio::OutCharPort("dataCharOut" );
  oid = ossie::corba::RootPOA()->activate_object(dataCharOut);

  registerInPort(dataCharIn);
  registerOutPort(dataCharOut, dataCharOut->_this());

  dataOctetIn = new bulkio::InOctetPort("dataOctetIn" );
  oid = ossie::corba::RootPOA()->activate_object(dataOctetIn);
  dataOctetOut = new bulkio::OutOctetPort("dataOctetOut" );
  oid = ossie::corba::RootPOA()->activate_object(dataOctetOut);

  registerInPort(dataOctetIn);
  registerOutPort(dataOctetOut, dataOctetOut->_this());

  dataShortIn = new bulkio::InShortPort("dataShortIn" );
  oid = ossie::corba::RootPOA()->activate_object(dataShortIn);
  dataShortOut = new bulkio::OutShortPort("dataShortOut");
  oid = ossie::corba::RootPOA()->activate_object(dataShortOut);

  registerInPort(dataShortIn);
  registerOutPort(dataShortOut, dataShortOut->_this());


  dataUShortIn = new bulkio::InUShortPort("dataUShortIn");
  oid = ossie::corba::RootPOA()->activate_object(dataUShortIn);
  dataUShortOut = new bulkio::OutUShortPort("dataUShortOut");
  oid = ossie::corba::RootPOA()->activate_object(dataUShortOut);

  registerInPort(dataUShortIn);
  registerOutPort(dataUShortOut, dataUShortOut->_this());

  dataLongIn = new bulkio::InLongPort("dataLongIn");
  oid = ossie::corba::RootPOA()->activate_object(dataLongIn);
  dataLongOut = new bulkio::OutLongPort("dataLongOut");
  oid = ossie::corba::RootPOA()->activate_object(dataLongOut);

  registerInPort(dataLongIn);
  registerOutPort(dataLongOut, dataLongOut->_this());


  dataULongIn = new bulkio::InULongPort("dataULongIn");
  oid = ossie::corba::RootPOA()->activate_object(dataULongIn);
  dataULongOut = new bulkio::OutULongPort("dataULongOut");
  oid = ossie::corba::RootPOA()->activate_object(dataULongOut);

  registerInPort(dataULongIn);
  registerOutPort(dataULongOut, dataULongOut->_this());


  dataLongLongIn = new bulkio::InLongLongPort("dataLongLongIn");
  oid = ossie::corba::RootPOA()->activate_object(dataLongLongIn);
  dataLongLongOut = new bulkio::OutLongLongPort("dataLongLongOut");
  oid = ossie::corba::RootPOA()->activate_object(dataLongLongOut);

  registerInPort(dataLongLongIn);
  registerOutPort(dataLongLongOut, dataLongLongOut->_this());


  dataULongLongIn = new bulkio::InULongLongPort("dataULongLongIn");
  oid = ossie::corba::RootPOA()->activate_object(dataULongLongIn);
  dataULongLongOut = new bulkio::OutULongLongPort("dataULongLongOut");
  oid = ossie::corba::RootPOA()->activate_object(dataULongLongOut);

  registerInPort(dataULongLongIn);
  registerOutPort(dataULongLongOut, dataULongLongOut->_this());



  dataFloatIn = new bulkio::InFloatPort("dataFloatIn");
  oid = ossie::corba::RootPOA()->activate_object(dataFloatIn);
  dataFloatOut = new bulkio::OutFloatPort("dataFloatOut");
  oid = ossie::corba::RootPOA()->activate_object(dataFloatOut);

  registerInPort(dataFloatIn);
  registerOutPort(dataFloatOut, dataFloatOut->_this());

  dataDoubleIn = new bulkio::InDoublePort("dataDoubleIn");
  oid = ossie::corba::RootPOA()->activate_object(dataDoubleIn);
  dataDoubleOut = new bulkio::OutDoublePort("dataDoubleOut");
  oid = ossie::corba::RootPOA()->activate_object(dataDoubleOut);

  registerInPort(dataDoubleIn);
  registerOutPort(dataDoubleOut, dataDoubleOut->_this());

  dataFileIn = new bulkio::InURLPort("dataFileIn");
  oid = ossie::corba::RootPOA()->activate_object(dataFileIn);
  dataFileOut = new bulkio::OutURLPort("dataFileOut");
  oid = ossie::corba::RootPOA()->activate_object(dataFileOut);

  registerInPort(dataFileIn);
  registerOutPort(dataFileOut, dataFileOut->_this());

  dataFileIn = new bulkio::InURLPort("dataFileIn");
  oid = ossie::corba::RootPOA()->activate_object(dataFileIn);
  dataFileOut = new bulkio::OutURLPort("dataFileOut");
  oid = ossie::corba::RootPOA()->activate_object(dataFileOut);

  registerInPort(dataFileIn);
  registerOutPort(dataFileOut, dataFileOut->_this());


  dataXMLIn = new bulkio::InXMLPort("dataXMLIn");
  oid = ossie::corba::RootPOA()->activate_object(dataXMLIn);
  dataXMLOut = new bulkio::OutXMLPort("dataXMLOut");
  oid = ossie::corba::RootPOA()->activate_object(dataXMLOut);

  registerInPort(dataXMLIn);
  registerOutPort(dataXMLOut, dataXMLOut->_this());

  dataSDDSIn = new bulkio::InSDDSPort("dataSDDSIn", &cb );
  oid = ossie::corba::RootPOA()->activate_object(dataSDDSIn);
  dataSDDSOut = new bulkio::OutSDDSPort("dataSDDSOut");
  oid = ossie::corba::RootPOA()->activate_object(dataSDDSOut);

  registerInPort(dataSDDSIn);
  registerOutPort(dataSDDSOut, dataSDDSOut->_this());


}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void CPP_Ports_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void CPP_Ports_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        dataCharIn->unblock();
        dataOctetIn->unblock();
        dataShortIn->unblock();
        dataUShortIn->unblock();
        dataLongIn->unblock();
        dataULongIn->unblock();
        dataFloatIn->unblock();
        dataDoubleIn->unblock();
        dataFileIn->unblock();
        dataXMLIn->unblock();
        serviceThread = new ProcessThread<CPP_Ports_base>(this, 0.1);
        serviceThread->start();
    }

    if (!Resource_impl::started()) {
            Resource_impl::start();
    }
}

void CPP_Ports_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        dataCharIn->block();
        dataOctetIn->block();
        dataShortIn->block();
        dataUShortIn->block();
        dataLongIn->block();
        dataULongIn->block();
        dataFloatIn->block();
        dataDoubleIn->block();
        dataFileIn->block();
        dataXMLIn->block();
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }

    if (Resource_impl::started()) {
            Resource_impl::stop();
    }
}

CORBA::Object_ptr CPP_Ports_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {
        if (!strcmp(_id,"dataCharIn")) {
          bulkio::InCharPort *ptr = dynamic_cast< bulkio::InCharPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }
        if (!strcmp(_id,"dataOctetIn")) {
          bulkio::InOctetPort *ptr = dynamic_cast< bulkio::InOctetPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataShortIn")) {
          bulkio::InShortPort *ptr = dynamic_cast< bulkio::InShortPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataUShortIn")) {
          bulkio::InUShortPort *ptr = dynamic_cast< bulkio::InUShortPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataLongIn")) {
          bulkio::InLongPort *ptr = dynamic_cast< bulkio::InLongPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataULongIn")) {
          bulkio::InULongPort *ptr = dynamic_cast< bulkio::InULongPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }


        if (!strcmp(_id,"dataLongLongIn")) {
          bulkio::InLongLongPort *ptr = dynamic_cast< bulkio::InLongLongPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataULongLongIn")) {
          bulkio::InULongLongPort *ptr = dynamic_cast< bulkio::InULongLongPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataDoubleIn")) {
          bulkio::InDoublePort *ptr = dynamic_cast< bulkio::InDoublePort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataFloatIn")) {
          bulkio::InFloatPort *ptr = dynamic_cast< bulkio::InFloatPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }
        if (!strcmp(_id,"dataDoubleIn")) {
          bulkio::InDoublePort *ptr = dynamic_cast< bulkio::InDoublePort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataFileIn")) {
          bulkio::InURLPort *ptr = dynamic_cast< bulkio::InURLPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataXMLIn")) {
          bulkio::InXMLPort *ptr = dynamic_cast< bulkio::InXMLPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

        if (!strcmp(_id,"dataSDDSIn")) {
          bulkio::InSDDSPort *ptr = dynamic_cast< bulkio::InSDDSPort * >(p_in->second);
          return ptr->_this()->_duplicate(ptr->_this());
        }

    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void CPP_Ports_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

    delete(dataCharIn);
    delete(dataOctetIn);
    delete(dataShortIn);
    delete(dataUShortIn);
    delete(dataLongIn);
    delete(dataULongIn);
    delete(dataLongLongIn);
    delete(dataULongLongIn);
    delete(dataFloatIn);
    delete(dataDoubleIn);
    delete(dataFileIn);
    delete(dataXMLIn);
    delete(dataSDDSIn);
    delete(dataCharOut);
    delete(dataOctetOut);
    delete(dataShortOut);
    delete(dataUShortOut);
    delete(dataLongOut);
    delete(dataULongOut);
    delete(dataLongLongOut);
    delete(dataULongLongOut);
    delete(dataFloatOut);
    delete(dataDoubleOut);
    delete(dataFileOut);
    delete(dataXMLOut);
    delete(dataSDDSOut);

    Resource_impl::releaseObject();
}

void CPP_Ports_base::loadProperties()
{
}
