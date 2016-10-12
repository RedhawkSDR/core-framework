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

#include "FakeApplication.h"
#include "Application_impl.h"

FakeApplication::FakeApplication (Application_impl* app) :
    _app(app)
{
}

FakeApplication::~FakeApplication () {
}

char* FakeApplication::identifier ()
{
    return _app->identifier();
}

CORBA::Boolean FakeApplication::started ()
{
    return _app->started();
}

void FakeApplication::start ()
{
    throw CF::Resource::StartError();
}

void FakeApplication::stop ()
{
    throw CF::Resource::StopError();
}

void FakeApplication::configure (const CF::Properties& configProperties)
{
    throw CF::UnknownProperties();
}

void FakeApplication::query (CF::Properties& configProperties)
{
    throw CF::UnknownProperties();
}

char * FakeApplication::registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval) 
  throw(CF::UnknownProperties, CF::InvalidObjectReference)
{
  throw CF::UnknownProperties();
}

void  FakeApplication::unregisterPropertyListener( const char *reg_id )   
  throw(CF::InvalidIdentifier)
{
  throw CF::InvalidIdentifier();
}

void FakeApplication::initialize ()
{
    throw CF::LifeCycle::InitializeError();
}

void FakeApplication::releaseObject ()
{
    throw CF::LifeCycle::ReleaseError();
}

CORBA::Object_ptr FakeApplication::getPort (const char*)
{
    throw CF::PortSupplier::UnknownPort();
}

CF::PortSet::PortInfoSequence* FakeApplication::getPortSet ()
{
	return new CF::PortSet::PortInfoSequence();
}

void FakeApplication::runTest (CORBA::ULong, CF::Properties&)
{
    throw CF::TestableObject::UnknownTest();
}

char* FakeApplication::profile ()
{
    return CORBA::string_dup("");
}

char* FakeApplication::softwareProfile ()
{
    return CORBA::string_dup("");
}

char* FakeApplication::name ()
{
    return _app->name();
}

bool FakeApplication::aware ()
{
    return false;
}

CF::DeviceAssignmentSequence * FakeApplication::componentDevices ()
{
    return new CF::DeviceAssignmentSequence();
}

CF::Application::ComponentElementSequence * FakeApplication::componentImplementations ()
{
    return new CF::Application::ComponentElementSequence();
}

CF::Application::ComponentElementSequence * FakeApplication::componentNamingContexts ()
{
    return new CF::Application::ComponentElementSequence();
}

CF::Application::ComponentProcessIdSequence * FakeApplication::componentProcessIds ()
{
    return new CF::Application::ComponentProcessIdSequence();
}

CF::Components * FakeApplication::registeredComponents () {
    return new CF::Components();
}

