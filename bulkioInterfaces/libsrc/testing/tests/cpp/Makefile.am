#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
# Rules for the test code (use `make check` to execute)
TESTS = Bulkio
check_PROGRAMS = $(TESTS)

Bulkio_SOURCES = Bulkio.cpp Bulkio_Helper_Fixture.cpp Bulkio_InPort_Fixture.cpp Bulkio_OutPort_Fixture.cpp Bulkio_MultiOut_Port.cpp
Bulkio_SOURCES += InStreamTest.h InStreamTest.cpp
Bulkio_CXXFLAGS = $(BULKIO_CFLAGS) $(BOOST_CPPFLAGS) $(OSSIE_CFLAGS) $(CPPUNIT_CFLAGS)
Bulkio_LDADD = $(BULKIO_LIBS) $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(OSSIE_LIBS) $(CPPUNIT_LIBS) $(LOG4CXX_LIBS)
