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
#ifndef CPPTESTDEVICE_IMPL_H
#define CPPTESTDEVICE_IMPL_H

#include "CppTestDevice_base.h"

class CppTestDevice_i;

class CppTestDevice_i : public CppTestDevice_base
{
    ENABLE_LOGGING
    public:
		CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~CppTestDevice_i();
        int serviceFunction();

        void initialize();

    private:
        bool allocate_memory(const memory_allocation_struct& capacity);
        void deallocate_memory(const memory_allocation_struct& capacity);

        bool allocate_load(const float& capacity);
        void deallocate_load(const float& capacity);
};

#endif
