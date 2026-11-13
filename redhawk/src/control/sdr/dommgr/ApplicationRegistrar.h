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

#ifndef APPLICATIONREGISTRAR_H
#define	APPLICATIONREGISTRAR_H

#include <ossie/CF/cf.h>
#include <ossie/Logging_impl.h>

class Application_impl;
class FakeApplication;

class ApplicationRegistrar_impl : public virtual POA_CF::ApplicationRegistrar {
public:
    ApplicationRegistrar_impl(CosNaming::NamingContext_ptr WaveformContext, Application_impl *app);
    virtual ~ApplicationRegistrar_impl();
    CF::Application_ptr app();
    CF::DomainManager_ptr domMgr();
    void registerComponent(const char * Name, CF::Resource_ptr obj);
    
    // CosNaming::NamingContext interface (supported))
    void bind(const CosNaming::Name &Name, CORBA::Object_ptr obj);
    void unbind(const CosNaming::Name &Name);
    void rebind(const CosNaming::Name &Name, CORBA::Object_ptr obj);
    
    // CosNaming::NamingContext interface (unsupported)
    void bind_context(const CosNaming::Name &Name, CosNaming::NamingContext_ptr obj) {};
    void rebind_context(const CosNaming::Name &Name, CosNaming::NamingContext_ptr obj) {};
    CORBA::Object_ptr resolve(const CosNaming::Name &Name) {return CORBA::Object::_nil();};
    CosNaming::NamingContext_ptr new_context() {return CosNaming::NamingContext::_nil();};
    CosNaming::NamingContext_ptr bind_new_context(const CosNaming::Name &Name) {return CosNaming::NamingContext::_nil();};
    void destroy() {};
    void list(CORBA::ULong length, CosNaming::BindingList_out out, CosNaming::BindingIterator_out iterator) {};
    
private:
    CosNaming::NamingContext_var _context;
    Application_impl *_application;
};

#endif	/* APPLICATIONREGISTRAR_H */

