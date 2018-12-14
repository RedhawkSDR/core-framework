/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK throughput.
 *
 * REDHAWK throughput is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef READER_I_IMPL_H
#define READER_I_IMPL_H

#include "reader_base.h"

class OctetPort;

class reader_i : public reader_base
{
    ENABLE_LOGGING
    public:
        reader_i(const char *uuid, const char *label);
        ~reader_i();

        void constructor();

        int serviceFunction();

    private:
        OctetPort* dataOctet_in;

    double get_average_time();
};

#endif // READER_I_IMPL_H
