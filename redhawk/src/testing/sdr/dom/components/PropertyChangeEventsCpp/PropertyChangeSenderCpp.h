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

#ifndef MESSAGERECEIVERCPP__H
#define MESSAGERECEIVERCPP__H

#include <ossie/PropertyInterface.h>

#include <string>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"

class PropertyChangeSenderCpp : public Resource_impl
{
    ENABLE_LOGGING;
    
    friend class PropertyEventSupplier;

public:
    PropertyChangeSenderCpp (const char* uuid, const char* label);
    ~PropertyChangeSenderCpp (void);

    CORBA::Object_ptr getPort (const char*);

    void releaseObject (void);

    void start();

private:
    PropertyEventSupplier* propEvent;
    short somevalue;
    CORBA::Long myprop;
    CORBA::Long anotherprop;
    std::vector<float> seqprop;
    some_struct_struct some_struct;
    std::vector<some_struct_struct> structseq_prop;
    short anothervalue;
};

#endif
