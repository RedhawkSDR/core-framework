#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

TESTS = test_libossiecf

AM_CPPFLAGS = -I $(top_srcdir)/base/include
AM_LDFLAGS = $(top_builddir)/base/framework/libossiecf.la $(top_builddir)/base/framework/idl/libossieidl.la -no-install

check_PROGRAMS = $(TESTS)

test_libossiecf_SOURCES = test_libossiecf.cpp
test_libossiecf_SOURCES += SharedBufferTest.cpp SharedBufferTest.h
test_libossiecf_SOURCES += AnyUtilsTest.cpp AnyUtilsTest.h
test_libossiecf_SOURCES += ValueTest.cpp ValueTest.h
test_libossiecf_SOURCES += ValueSequenceTest.cpp ValueSequenceTest.h
test_libossiecf_SOURCES += PropertyMapTest.cpp PropertyMapTest.h
test_libossiecf_SOURCES += MessagingTest.cpp MessagingTest.h
test_libossiecf_SOURCES += ExecutorServiceTest.cpp ExecutorServiceTest.h
test_libossiecf_SOURCES += BufferManagerTest.cpp BufferManagerTest.h
test_libossiecf_SOURCES += CallbackTest.cpp CallbackTest.h
test_libossiecf_SOURCES += PortManager.cpp PortManager.h
test_libossiecf_SOURCES += BitopsTest.cpp BitopsTest.h
test_libossiecf_SOURCES += BitBufferTest.cpp BitBufferTest.h
test_libossiecf_SOURCES += ServiceInterruptTest.cpp ServiceInterruptTest.h
test_libossiecf_CXXFLAGS = -Wall $(CPPUNIT_CFLAGS)
test_libossiecf_LDFLAGS = $(CPPUNIT_LIBS) $(AM_LDFLAGS)

# Benchmark program for bit operations
noinst_PROGRAMS = benchmark_bitops

benchmark_bitops_SOURCES = benchmark_bitops.cpp
benchmark_bitops_CXXFLAGS = -Wall

CLEANFILES = libossiecf-cppunit-results.xml