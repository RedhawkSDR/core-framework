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

CFDIR = ../../../../../../base
AM_CPPFLAGS = -I$(CFDIR)/include

ossieName = BasicDevWithExecParam_cpp_impl1
noinst_PROGRAMS = BasicDevWithExecParam_cpp_impl1

BasicDevWithExecParam_cpp_impl1_SOURCES = BasicDevWithExecParam_cpp_impl1.cpp BasicDevWithExecParam_cpp_impl1_base.cpp BasicDevWithExecParam_cpp_impl1_base.h BasicDevWithExecParam_cpp_impl1.h main.cpp port_impl.cpp port_impl.h
BasicDevWithExecParam_cpp_impl1_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) $(OMNIORB_CFLAGS) $(OMNITHREAD_CFLAGS) $(OMNIDYNAMIC_CFLAGS)
BasicDevWithExecParam_cpp_impl1_LDADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNICOS_LIBS) $(OMNITHREAD_LIBS) $(OMNIDYNAMIC_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la
BasicDevWithExecParam_cpp_impl1_LDFLAGS = -Wall
