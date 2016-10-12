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
#include <fstream>
#include <sstream>

#include "BasicTestDevice_cpp_impl1.h"

#include "port_impl.h"
#include <uuid/uuid.h>


/*******************************************************************************************

    These are auto-generated support functions. You should never have to mess with these
     functions.

*******************************************************************************************/


void BasicTestDevice_cpp_impl1_i::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    while ((*this->devices()).length() > 0) {
        CF::Device_ptr dev = (*this->devices())[0];
        dev->releaseObject();
    }
    

    
    
    // This function clears the component running condition so main shuts down everything
    Device_impl::releaseObject();
}

void BasicTestDevice_cpp_impl1_i::loadProperties()
{

	addProperty(memCapacity,
			"DCE:7aeaace8-350e-48da-8d77-f97c2e722e06",
			"memCapacity",
			"readwrite",
			"bytes",
			"external",
			"allocation");

	addProperty(BogoMipsCapacity,
			"DCE:bbdf708f-ce05-469f-8aed-f5c93e353e14",
			"BogoMipsCapacity",
			"readwrite",
			"bytes",
			"external",
			"allocation");

	addProperty(propOne,
			"DCE:9607a8db-2ce1-4e71-9dee-9bb18377127c",
			"propOne",
			"readwrite",
			"",
			"external",
			"configure,execparam");

	addProperty(propTwo,
			"DCE:5bf37e47-afa5-4865-9653-9d427ffa55d2",
			"propTwo",
			"readwrite",
			"",
			"external",
			"execparam");
    
}
