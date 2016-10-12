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

 
#ifndef TICKET2093_IMPL_H
#define TICKET2093_IMPL_H

#include "ticket2093_base.h"

class ticket2093_i;

class ticket2093_i : public ticket2093_base
{
    ENABLE_LOGGING
    public: 
        ticket2093_i(const char *uuid, const char *label);
        ~ticket2093_i();
        int serviceFunction();
};

#endif
