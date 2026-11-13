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

#ifndef FAKEAPPLICATION_H
#define FAKEAPPLICATION_H

#include <string>

#include <ossie/CF/cf.h>
#include <ossie/Logging_impl.h>

class Application_impl;

class FakeApplication : public virtual POA_CF::Application, public Logging_impl
{

public:
    FakeApplication (Application_impl* app);
    ~FakeApplication ();

    char* identifier ();
    CORBA::Boolean started ();
    void start ();
    void stop ();

    void initializeProperties (const CF::Properties& configProperties){};
    void configure (const CF::Properties& configProperties);
    void query (CF::Properties& configProperties);
    CF::Properties* metrics (const CF::StringSequence& components, const CF::StringSequence& attributes);
    char *registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval);
    void unregisterPropertyListener( const char *reg_id );

    void initialize ();
        
    void releaseObject ();
        
    CORBA::Object_ptr getPort (const char*);

    CF::PortSet::PortInfoSequence* getPortSet ();
        
    void runTest (CORBA::ULong, CF::Properties&);
    
    char* profile ();
    
    char* softwareProfile ();
    
    char* name ();
    
    bool aware ();
    
    CORBA::Float stopTimeout ();

    void stopTimeout (CORBA::Float timeout);
    
    CF::DeviceAssignmentSequence * componentDevices ();
    CF::Application::ComponentElementSequence * componentImplementations ();
    CF::Application::ComponentElementSequence * componentNamingContexts ();
    CF::Application::ComponentProcessIdSequence * componentProcessIds ();
    CF::Components * registeredComponents ();
    
protected:
    Application_impl* _app;
};

#endif // FAKEAPPLICATION_H
