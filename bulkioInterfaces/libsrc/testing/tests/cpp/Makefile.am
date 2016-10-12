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
bulkio_top=../../../../
bulkio_libsrc_top=$(bulkio_top)/libsrc
Bulkio_SOURCES = Bulkio.cpp Bulkio_Helper_Fixture.cpp Bulkio_InPort_Fixture.cpp Bulkio_OutPort_Fixture.cpp Bulkio_MultiOut_Port.cpp
Bulkio_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(bulkio_libsrc_top)/cpp  -I$(bulkio_top)/src/cpp -I$(bulkio_top)/src/cpp/ossie  $(BOOST_CPPFLAGS) $(RH_DEPS_CFLAGS)
Bulkio_LDADD = -L$(bulkio_libsrc_top)/.libs -L$(bulkio_top)/.libs -lbulkio-2.0 -lbulkioInterfaces $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(RH_DEPS_LIBS) $(CPPUNIT_LIBS) -llog4cxx 
