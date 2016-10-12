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

SUBDIRS = CORBA

lib_LTLIBRARIES = libomnijni.la

libomnijni_la_CPPFLAGS = $(OMNIORB_CFLAGS) $(OMNIJNI_CPPFLAGS)
libomnijni_la_SOURCES = any.cpp \
			exception.cpp \
			object.cpp \
			omnijni.cpp \
			orb.cpp \
			sequence.cpp \
			utilities.cpp
libomnijni_la_LIBADD = $(OMNIORB_LIBS) CORBA/libcorbajni.la
