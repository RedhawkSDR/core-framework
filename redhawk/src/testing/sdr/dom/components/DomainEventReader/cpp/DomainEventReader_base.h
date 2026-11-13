/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any 
 * later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef DOMAINEVENTREADER_BASE_IMPL_BASE_H
#define DOMAINEVENTREADER_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class DomainEventReader_base : public Component, protected ThreadedComponent
{
    public:
        DomainEventReader_base(const char *uuid, const char *label);
        ~DomainEventReader_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: num_add_events
        CORBA::LongLong num_add_events;
        /// Property: num_remove_events
        CORBA::LongLong num_remove_events;

    private:
};
#endif // DOMAINEVENTREADER_BASE_IMPL_BASE_H
