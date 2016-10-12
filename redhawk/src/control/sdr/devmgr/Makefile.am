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

AM_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) $(OMNIORB_CFLAGS) $(LOG4CXX_FLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(OMNIORB_LIBS) $(LOG4CXX_LIBS)
INCLUDES = -I../../include -I../../parser

DeviceManager_SOURCES = main.cpp DeviceManager_impl.cpp
DeviceManager_LDADD = ../../framework/libossiecf.la ../../parser/libossieparser.la ../../framework/idl/libossieidl.la
DeviceManager_LDFLAGS = -static

devmgrdir = $(SDR_ROOT)/dev/mgr
dist_devmgr_DATA = DeviceManager.spd.xml DeviceManager.scd.xml DeviceManager.prf.xml DeviceManager.Linux.x86.prf.xml DeviceManager.Linux.x86_64.prf.xml
devmgr_PROGRAMS = DeviceManager
