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

#if HAVE_GCC_ABI_DEMANGLE
#include <cxxabi.h>
#include <stdlib.h>
#endif

#include <ossie/type_traits.h>

const std::string ossie::internal::demangle (const std::string& name)
{
#if HAVE_GCC_ABI_DEMANGLE
    int status = 0;
    char* c_name = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
    std::string result;
    if (c_name) {
        result = c_name;
        free(c_name);
    }
    return result;
#else
    return name;
#endif
}
