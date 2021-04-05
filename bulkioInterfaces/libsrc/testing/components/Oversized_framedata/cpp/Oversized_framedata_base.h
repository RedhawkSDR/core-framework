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
#ifndef OVERSIZED_FRAMEDATA_IMPL_BASE_H
#define OVERSIZED_FRAMEDATA_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class Oversized_framedata_base : public Resource_impl, protected ThreadedComponent
{
    public:
        Oversized_framedata_base(const char *uuid, const char *label);
        ~Oversized_framedata_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:

        // Ports
        bulkio::OutShortPort *dataShort_out;

    private:
};
#endif // OVERSIZED_FRAMEDATA_IMPL_BASE_H
