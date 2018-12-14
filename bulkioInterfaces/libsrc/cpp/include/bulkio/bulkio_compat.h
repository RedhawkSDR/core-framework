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
#ifndef __bulkio_compat_h
#define __bulkio_compat_h

//
// Compatibility aliases mapping BULKIO base classes to legacy generated class
// names. This header should only be included in projects that are migrating to
// BULKIO base classes from pre-1.9 code generators.
//
// For forward-compatibility with future releases, references to the old names
// should be replaced with the corresponding base class.
//

#warning "This project uses deprecated interfaces; BULKIO class references should be updated to bulkio namespace"

#include "bulkio.h"

typedef bulkio::InCharPort      BULKIO_dataChar_In_i;
typedef bulkio::InOctetPort     BULKIO_dataOctet_In_i;
typedef bulkio::InShortPort     BULKIO_dataShort_In_i;
typedef bulkio::InUShortPort    BULKIO_dataUshort_In_i;
typedef bulkio::InLongPort      BULKIO_dataLong_In_i;
typedef bulkio::InULongPort     BULKIO_dataUlong_In_i;
typedef bulkio::InLongLongPort  BULKIO_dataLongLong_In_i;
typedef bulkio::InULongLongPort BULKIO_dataUlongLong_In_i;
typedef bulkio::InFloatPort     BULKIO_dataFloat_In_i;
typedef bulkio::InDoublePort    BULKIO_dataDouble_In_i;
typedef bulkio::InFilePort      BULKIO_dataFile_In_i;
typedef bulkio::InXMLPort       BULKIO_dataXML_In_i;
typedef bulkio::InSDDSPort      BULKIO_dataSDDS_In_i;

typedef bulkio::OutCharPort      BULKIO_dataChar_Out_i;
typedef bulkio::OutOctetPort     BULKIO_dataOctet_Out_i;
typedef bulkio::OutShortPort     BULKIO_dataShort_Out_i;
typedef bulkio::OutUShortPort    BULKIO_dataUShort_Out_i;
typedef bulkio::OutLongPort      BULKIO_dataLong_Out_i;
typedef bulkio::OutULongPort     BULKIO_dataULong_Out_i;
typedef bulkio::OutLongLongPort  BULKIO_dataLongLong_Out_i;
typedef bulkio::OutULongLongPort BULKIO_dataULongLong_Out_i;
typedef bulkio::OutFloatPort     BULKIO_dataFloat_Out_i;
typedef bulkio::OutDoublePort    BULKIO_dataDouble_Out_i;
typedef bulkio::OutFilePort      BULKIO_dataFile_Out_i;
typedef bulkio::OutXMLPort       BULKIO_dataXML_Out_i;
typedef bulkio::OutSDDSPort      BULKIO_dataSDDS_Out_i;

#endif
