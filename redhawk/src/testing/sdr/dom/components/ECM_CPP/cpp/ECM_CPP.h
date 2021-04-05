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
#ifndef ECM_CPP_IMPL_H
#define ECM_CPP_IMPL_H

#include "ECM_CPP_base.h"

class ECM_CPP_i : public ECM_CPP_base
{
    ENABLE_LOGGING
    public:
        ECM_CPP_i(const char *uuid, const char *label);
        ~ECM_CPP_i();
        int serviceFunction();
        void initialize();
        void releaseObject();

        void dataArrived( const CORBA::Any &data );
        void enableChanged( const bool *ov, bool const *nv );

        redhawk::events::ManagerPtr ecm;
        redhawk::events::PublisherPtr pub;
        redhawk::events::SubscriberPtr sub;
        int p_msgid;
};

#endif // ECM_CPP_IMPL_H
