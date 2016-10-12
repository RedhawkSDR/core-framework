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
#ifndef SRI_CHANGED_CPP_IMPL_H
#define SRI_CHANGED_CPP_IMPL_H

#include "sri_changed_cpp_base.h"

class sri_changed_cpp_i : public sri_changed_cpp_base
{
    ENABLE_LOGGING
    public:
        sri_changed_cpp_i(const char *uuid, const char *label);
        ~sri_changed_cpp_i();
        int serviceFunction();
};

#endif // SRI_CHANGED_CPP_IMPL_H
