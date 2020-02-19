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

#include "transport_api_test.h"

void setUp_custom_priority(const std::string &prior ) {
    setUp_transport_priority("custom",prior);
}

void setUp_transport_priority(const std::string &transport, const std::string &prior ) {
    std::string trans("BULKIO_"+transport+"_PRIORITY");
    std::transform(trans.begin(), trans.end(), trans.begin(), ::toupper);
    setenv(trans.c_str(), prior.c_str(),1);
}

void setUp_disable_transport( const std::string &transport ) {
    std::string trans("BULKIO_"+transport);
    std::transform(trans.begin(), trans.end(), trans.begin(), ::toupper);
    setenv(trans.c_str(), "disable",1);
}

void setUp_disable_env(const std::string &envv) {
    setenv(envv.c_str(), "disable",1);
}

void setUp_disable_custom() {
    setUp_disable_transport("custom");
}

void setUp_bulkio_test() {
   setenv("BULKIO_TEST","enable",1);
}

void tearDown_reset_env()
{
    unsetenv("BULKIO_TEST");
    unsetenv("BULKIO_LOCAL");
    unsetenv("BULKIO_CORBA");
    unsetenv("BULKIO_SHM");
    unsetenv("BULKIO_CUSTOM_CONFIG");
    unsetenv("BULKIO_CUSTOM_PRIORITY");
    unsetenv("BULKIO_CUSTOM");
}
