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
#ifndef OVERSIZED_FRAMEDATA_IMPL_H
#define OVERSIZED_FRAMEDATA_IMPL_H

#include "Oversized_framedata_base.h"

class Oversized_framedata_i : public Oversized_framedata_base
{
    ENABLE_LOGGING
    public:
        Oversized_framedata_i(const char *uuid, const char *label);
        ~Oversized_framedata_i();
        int serviceFunction();
        BULKIO::StreamSRI sri;
};

#endif // OVERSIZED_FRAMEDATA_IMPL_H
