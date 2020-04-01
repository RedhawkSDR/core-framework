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

#include "StreamSRITest.h"

#include <bulkio/bulkio.h>

CPPUNIT_TEST_SUITE_REGISTRATION(StreamSRITest);

void StreamSRITest::testCreate()
{
    BULKIO::StreamSRI sri = bulkio::sri::create();
}

void StreamSRITest::testCompare()
{
    BULKIO::StreamSRI A = bulkio::sri::create();
    BULKIO::StreamSRI B = bulkio::sri::create();
    BULKIO::StreamSRI C = bulkio::sri::create();

    CPPUNIT_ASSERT_MESSAGE("Identical SRIs should compare equal",
                           bulkio::sri::DefaultComparator(A, B));

    C.streamID = std::string("No Match").c_str();
    CPPUNIT_ASSERT_MESSAGE("SRIs with different streamIDs should compare unequal",
                           ! bulkio::sri::DefaultComparator(A, C));

    C.streamID = A.streamID;
    CPPUNIT_ASSERT_MESSAGE("Identical SRIs should compare equal (restored streamID)",
                           bulkio::sri::DefaultComparator(A, B));

    A.blocking = false;
    C.blocking = true;
    CPPUNIT_ASSERT_MESSAGE("SRIs with different blocking should compare unequal",
                           ! bulkio::sri::DefaultComparator(A, C));
}
