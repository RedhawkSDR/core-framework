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
#ifndef TESTLOGGINGAPI_IMPL_BASE_H
#define TESTLOGGINGAPI_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>


class TestLoggingAPI_base : public Resource_impl, protected ThreadedComponent
{
    public:
        TestLoggingAPI_base(const char *uuid, const char *label);
        ~TestLoggingAPI_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: new_log_level
        CORBA::Long new_log_level;
        /// Property: new_log_cfg
        std::string new_log_cfg;
        /// Property: disable_cb
        bool disable_cb;

    private:
};
#endif // TESTLOGGINGAPI_IMPL_BASE_H
