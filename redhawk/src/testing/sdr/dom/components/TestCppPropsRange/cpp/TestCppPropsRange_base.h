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
#ifndef TESTCPPPROPSRANGE_IMPL_BASE_H
#define TESTCPPPROPSRANGE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class TestCppPropsRange_base : public Component, protected ThreadedComponent
{
    public:
        TestCppPropsRange_base(const char *uuid, const char *label);
        ~TestCppPropsRange_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        unsigned char my_octet_name;
        short my_short_name;
        unsigned short my_ushort_name;
        CORBA::Long my_long_name;
        CORBA::ULong my_ulong_name;
        CORBA::LongLong my_longlong_name;
        CORBA::ULongLong my_ulonglong_name;
        std::vector<unsigned char> seq_octet_name;
        std::vector<short> seq_short_name;
        std::vector<unsigned short> seq_ushort_name;
        std::vector<CORBA::Long> seq_long_name;
        std::vector<CORBA::ULong> seq_ulong_name;
        std::vector<CORBA::LongLong> seq_longlong_name;
        std::vector<CORBA::ULongLong> seq_ulonglong_name;
        std::vector<char> seq_char_name;
        my_struct_name_struct my_struct_name;
        std::vector<ss_struct_name_struct> my_structseq_name;

    private:
};
#endif // TESTCPPPROPSRANGE_IMPL_BASE_H
