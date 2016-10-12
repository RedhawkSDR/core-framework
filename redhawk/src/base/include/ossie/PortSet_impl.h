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


#ifndef PORTSET_IMPL_H
#define PORTSET_IMPL_H

#include "CF/cf.h"
#include "PortSupplier_impl.h"
#include "debug.h"
#include "ossie/Autocomplete.h"

/*
The port set provides additional functionality on top of CF::PortSupplier
*/

class PortSet_impl :
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    public virtual POA_CF::PortSet,
#endif
    public PortSupplier_impl
{
    ENABLE_LOGGING;

public:
    // Return the set of ports owned by this instance
    CF::PortSet::PortInfoSequence* getPortSet ();

protected:
    PortSet_impl ();
};

#endif
