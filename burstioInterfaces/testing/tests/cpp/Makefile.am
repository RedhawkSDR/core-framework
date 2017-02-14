#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

burstio_include=$(top_srcdir)/src/cpp/include
Burstio_SOURCES = Burstio.cpp Burstio_InPort.cpp Burstio_OutPort.cpp Burstio_PushTest.cpp Burstio_Utils_Test.cpp
Burstio_SOURCES += LocalTest.h LocalTest.cpp
Burstio_INCLUDES = -I$(burstio_include) -I$(burstio_include)/burstio -I$(top_builddir)/src/cpp -I$(top_builddir)/src/cpp/redhawk
Burstio_CXXFLAGS = $(CPPUNIT_CFLAGS) $(Burstio_INCLUDES) $(BOOST_CPPFLAGS) $(BULKIO_CFLAGS)
Burstio_LDADD = $(BULKIO_LIBS) $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(CPPUNIT_LIBS) -llog4cxx -ldl
Burstio_LDADD += $(top_builddir)/src/cpp/libburstio.la $(top_builddir)/src/cpp/libburstioInterfaces.la

TESTS = Burstio
check_PROGRAMS=$(TESTS)
