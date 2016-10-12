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

 
#ifndef SIMPLECOMPONENT_CPP_IMPL1_IMPL_H
#define SIMPLECOMPONENT_CPP_IMPL1_IMPL_H

#include "SimpleComponent_cpp_impl1_base.h"

class SimpleComponent_cpp_impl1_i;


class SimpleComponent_cpp_impl1_i : public SimpleComponent_cpp_impl1_base
{
    public: 
        SimpleComponent_cpp_impl1_i(const char *uuid, const char *label);
        int serviceFunction();
};

#endif
