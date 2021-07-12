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

#ifndef CPP_PORTS_IMPL_H
#define CPP_PORTS_IMPL_H

#include "CPP_Ports_base.h"

class CPP_Ports_i;

class CPP_Ports_i : public CPP_Ports_base
{

    ENABLE_LOGGING
    public:
        CPP_Ports_i(const char *uuid, const char *label);
        ~CPP_Ports_i();
        int serviceFunction();
        void initialize();

 public:
    void newStreamCallback( BULKIO::StreamSRI &sri ) {
    }
};

#endif
