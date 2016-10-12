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

#if HAVE_OMNIORB4
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#endif

#include "ossie/CorbaUtils.h"
#include "ossie/ossieSupport.h"

using namespace std;
using namespace ossie;

// Initialize static class members

CORBA::ORB_var ORB::orb = CORBA::ORB::_nil();
PortableServer::POA_var ORB::poa;
PortableServer::POAManager_var ORB::pman;
CosNaming::NamingContext_var ORB::inc;

ORB::ORB()

{
    if (CORBA::is_nil(ORB::orb)) {
        orb = ossie::corba::OrbInit(0, 0, false);
        init();
    }
}

ORB::ORB(int argc, char* argv[])

{
    if (CORBA::is_nil(ORB::orb)) {
        orb = ossie::corba::OrbInit(argc, argv, false);
        init();
    }
}

ORB::~ORB()

{
}


void ORB::init()

{
    // For backwards compatibility, set the public static fields based on the real interface.
    poa = ossie::corba::RootPOA();
    pman = poa->the_POAManager();
    pman->activate();
    inc = ossie::corba::InitialNamingContext();
}

CORBA::Object_ptr ORB::get_object_from_name(const char* name)

{
    CORBA::Object_var obj;
    CosNaming::Name_var cosName = string_to_CosName(name);

    obj = inc->resolve(cosName);

    return obj._retn();
}

CosNaming::Name_var ORB::string_to_CosName(const char* name)

{


    // Most generic solution, but makes call on remote object
    // Use ORB optimized solution if possible

#if HAVE_OMNIORB4
    CosNaming::Name_var cosName = omni::omniURI::stringToName(name);
#endif

    return cosName._retn();
}

void ORB::bind_object_to_name(CORBA::Object_ptr obj, const char* name)

{
    CosNaming::Name_var cosName = string_to_CosName(name);

    try {
        inc->rebind(cosName, obj);   // SCA says not to use rebind, sue me
    } catch (...) {
        cerr << "Failed to bind object to the name : " << name << endl;
        throw;
    }

}

void ORB::bind_object_to_name(CORBA::Object_ptr obj, const CosNaming::NamingContext_ptr nc, const char* name)

{
    CosNaming::Name_var cosName = string_to_CosName(name);

    try {
        nc->rebind(cosName, obj);   // SCA says not to use rebind, sue me
    } catch (...) {
        cerr << "Failed to bind object to the name : " << name << endl;
        throw;
    }

}

void ORB::unbind_name(const char* name)
{
    CosNaming::Name_var cosName = string_to_CosName(name);
    try {
        inc->unbind(cosName);
    } catch (...) {
        cerr << "Failed to unbind name : " << name << endl;
    }
}

void ORB::unbind_name(const CosNaming::NamingContext_ptr nc, const char* name)
{
    CosNaming::Name_var cosName = string_to_CosName(name);
    try {
        nc->unbind(cosName);
    } catch (...) {
        cerr << "Failed to unbind name : " << name << endl;
    }
}

void ORB::unbind_all_from_context(CosNaming::NamingContext_ptr nc)

{
    ///\todo Add support for deleting more than 100 names
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    nc->list(CHUNK, bl, it);

    for (unsigned int i = 0; i < bl->length(); i++) {
        nc->unbind(bl[i].binding_name);
    }
}
