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
#ifndef TESTCPPOPTIONALPROPS_IMPL_H
#define TESTCPPOPTIONALPROPS_IMPL_H

#include "TestCppOptionalProps_base.h"

class TestCppOptionalProps_i : public TestCppOptionalProps_base
{
    ENABLE_LOGGING
    public:
        TestCppOptionalProps_i(const char *uuid, const char *label);
        ~TestCppOptionalProps_i();
        int serviceFunction();

};

#endif // TESTCPPOPTIONALPROPS_IMPL_H
