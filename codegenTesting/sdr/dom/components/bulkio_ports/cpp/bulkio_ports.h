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
#ifndef BULKIO_PORTS_IMPL_H
#define BULKIO_PORTS_IMPL_H

#include "bulkio_ports_base.h"

class bulkio_ports_i;

class bulkio_ports_i : public bulkio_ports_base
{
    ENABLE_LOGGING
    public:
        bulkio_ports_i(const char *uuid, const char *label);
        ~bulkio_ports_i();
        int serviceFunction();

        void mqdChanged (const std::string&);

    private:
};

#endif
