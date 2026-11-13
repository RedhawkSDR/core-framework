#!/usr/bin/env python3
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

#
# This can be used by unittests to determine the build configuration of the framework.
#
# This is useful for unittests which test optional build-time behaviors

DEFS="""-DPACKAGE_NAME=\"ossie\" -DPACKAGE_TARNAME=\"ossie\" -DPACKAGE_VERSION=\"4.0.4\" -DPACKAGE_STRING=\"ossie\ 4.0.4\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DPACKAGE=\"ossie\" -DVERSION=\"4.0.4\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_SYS_WAIT_H=1 -DHAVE_OMNIORB4=1 -DHAVE_BOOST=1 -DHAVE_BOOST_SYSTEM=1 -DHAVE_BOOST_FILESYSTEM=1 -DHAVE_BOOST_SERIALIZATION=1 -DHAVE_BOOST_REGEX=1 -DHAVE_BOOST_THREAD=1 -DHAVE_BOOST_DATE_TIME=1 -DOMNIORB4_2=1 -DENABLE_EVENTS=1 -DHAVE_LIBNUMA=1 -DHAVE_LOG4CXX=1 -DRH_LOGGER=1 -DHAVE_SQLITE=1 -DENABLE_SQLITE_PERSISTENCE=1 -DHAVE_SYS_WAIT_H=1 -DHAVE_TIME_H=1 -DHAVE_DIRENT_H=1 -DHAVE_GCC_ABI_DEMANGLE=1 -DHAVE_LIBUUID=1"""

HAVE_JAVASUPPORT="""yes"""
