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

#ifndef __bulkio_h
#define __bulkio_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

//
// SCA Framework class for Provides and Uses port definitions
//
#include "ossie/Port_impl.h"

//
// BULKIO Interface definitions produced from IDL compilations
//
#include "BULKIO_Interfaces.h"

//
// Base types and constants used by bulkio library classes
//
#include "bulkio_base.h"

//
// Overloaded operators for BULKIO::PrecisionUTCTime
//
#include "bulkio_time_operators.h"

//
// Port Trait definitions that define the Type Traits for Input and Output Bulkio Ports
//
#include "bulkio_traits.h"

//
// Input (Provides) Port template definitions for Sequences and String types
//
#include "bulkio_in_port.h"

//
// Output (Uses) Port template definitions for Sequences and String types
//
#include "bulkio_out_port.h"

//
// Input/Output Port definitions for managing SDDS streams
//
#include "bulkio_attachable_base.h"

#endif
