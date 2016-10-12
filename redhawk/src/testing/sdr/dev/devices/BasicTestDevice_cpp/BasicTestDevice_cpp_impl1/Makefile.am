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
AM_LDFLAGS = -Wall $(OMNIORB_LIBS) $(BOOST_SYSTEM_LIB) $(OMNICOS_LIBS) $(OMNITHREAD_LIBS) $(OMNIDYNAMIC_LIBS)
INCLUDES = -I../../../../../../base/include -I../../../../../../base/parser -I../../../../../../base/include/ossie

ossieName = BasicTestDevice_cpp_impl1
noinst_PROGRAMS = BasicTestDevice_cpp_impl1

BasicTestDevice_cpp_impl1_SOURCES = BasicTestDevice_cpp_impl1.cpp BasicTestDevice_cpp_impl1_support.cpp BasicTestDevice_cpp_impl1.h main.cpp port_impl.cpp port_impl.h

BasicTestDevice_cpp_impl1_LDADD = ../../../../../../base/framework/libossiecf.la ../../../../../../base/parser/libossieparser.la ../../../../../../base/framework/idl/libossieidl.la
