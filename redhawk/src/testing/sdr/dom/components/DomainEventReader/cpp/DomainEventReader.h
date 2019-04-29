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
#ifndef DOMAINEVENTREADER_I_IMPL_H
#define DOMAINEVENTREADER_I_IMPL_H

#include "DomainEventReader_base.h"
#include "Listener.h"


class DomainEventReader_i : public DomainEventReader_base
{
    ENABLE_LOGGING
    private:
        Listener listener;
        redhawk::events::DomainEventReader * reader;
    public:
        DomainEventReader_i(const char *uuid, const char *label);
        ~DomainEventReader_i();

        void constructor();

        int serviceFunction();

        void increment_add_events();
        void increment_remove_events();
};

#endif // DOMAINEVENTREADER_I_IMPL_H
