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

ACLOCAL_AMFLAGS = -I m4 -I ${OSSIEHOME}/share/aclocal/ossie

SUBDIRS = src/idl src/cpp src/python
if HAVE_JAVASUPPORT
  SUBDIRS += src/java
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = burstioInterfaces.pc burstio.pc
