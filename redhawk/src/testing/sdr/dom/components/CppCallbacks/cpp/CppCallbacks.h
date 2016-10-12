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
#ifndef CPPCALLBACKS_IMPL_H
#define CPPCALLBACKS_IMPL_H

#include "CppCallbacks_base.h"

class CppCallbacks_i;

class CppCallbacks_i : public CppCallbacks_base
{
    ENABLE_LOGGING
    public:
        CppCallbacks_i(const char *uuid, const char *label);
        ~CppCallbacks_i();
        int serviceFunction();

    private:
        void count_changed(const CORBA::ULong* oldValue, const CORBA::ULong* newValue);
        void constellation_changed(const std::vector<std::complex<float> >* oldValue, const std::vector<std::complex<float> >* newValue);
        void station_changed(const station_struct* oldValue, const station_struct* newValue);
        void servers_changed(const std::vector<endpoint_struct>* oldValue, const std::vector<endpoint_struct>* newValue);
};

#endif
