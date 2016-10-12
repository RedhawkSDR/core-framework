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

 
AM_CXXFLAGS = -Wall $(OMNIORB_CFLAGS) $(OMNITHREAD_CFLAGS) $(OMNIDYNAMIC_CFLAGS)
AM_LDFLAGS = -Wall $(OMNIORB_LIBS) $(OMNITHREAD_LIBS) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)
INCLUDES = -I../../../../../../base/include -I../../../../../../base/parser -I../../../../../../base/include/ossie

ossieName = props_test_device_cpp_impl1
noinst_PROGRAMS = props_test_device_cpp_impl1

props_test_device_cpp_impl1_SOURCES = props_test_device_cpp_impl1.cpp props_test_device_cpp_impl1_base.cpp props_test_device_cpp_impl1.h main.cpp port_impl.cpp port_impl.h

props_test_device_cpp_impl1_LDADD = ../../../../../../base/framework/libossiecf.la ../../../../../../base/parser/libossieparser.la ../../../../../../base/framework/idl/libossieidl.la














