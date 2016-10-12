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

 
#ifndef SIMPLEDEVICE_CPP_IMPL1_IMPL_H
#define SIMPLEDEVICE_CPP_IMPL1_IMPL_H

#include "SimpleDevice_cpp_impl1_base.h"

class SimpleDevice_cpp_impl1_i;

class SimpleDevice_cpp_impl1_i : public SimpleDevice_cpp_impl1_base
{
    public:
        SimpleDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        SimpleDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        SimpleDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        SimpleDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~SimpleDevice_cpp_impl1_i();
        int serviceFunction();
};

#endif
