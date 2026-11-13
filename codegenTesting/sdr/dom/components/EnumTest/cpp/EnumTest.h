/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK codegenTesting.
 *
 * REDHAWK codegenTesting is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef ENUMTEST_I_IMPL_H
#define ENUMTEST_I_IMPL_H

#include "EnumTest_base.h"

#include <ossie/PropertyMap.h>

class EnumTest_i : public EnumTest_base
{
    ENABLE_LOGGING
    public:
        EnumTest_i(const char *uuid, const char *label);
        ~EnumTest_i();

        void constructor();

        int serviceFunction();

        void runTest(CORBA::ULong testid, CF::Properties& testValues);

    private:
        void runEnumTest(redhawk::PropertyMap& testValues);
};

#endif // ENUMTEST_I_IMPL_H
