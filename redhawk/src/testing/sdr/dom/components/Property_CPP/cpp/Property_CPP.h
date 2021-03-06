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
#ifndef PROPERTY_CPP_I_IMPL_H
#define PROPERTY_CPP_I_IMPL_H

#include "Property_CPP_base.h"

class Property_CPP_i : public Property_CPP_base
{
    ENABLE_LOGGING
    public:
        Property_CPP_i(const char *uuid, const char *label);
        ~Property_CPP_i();
        int serviceFunction();
        void initializeProperties(const CF::Properties& ctorProps)
          throw (CF::PropertySet::PartialConfiguration,
                 CF::PropertySet::InvalidConfiguration, CORBA::SystemException);
};

#endif // CPROPERTY_I_IMPL_H
