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

AM_CPPFLAGS = -I../../include

ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libossiecluster.la

distclean-local:
	rm -rf m4
	rm -rf .deps/ .libs/ Makefile Makefile.in *m4 autom4te.cache/ config.* configure depcomp install-sh libossiecluster* libtool ltmain.sh missing

libossiecluster_la_SOURCES = clustermgr.cpp EmptyResolver.cpp
libossiecluster_la_LIBADD = $(REDHAWK_LIBS) -lyaml-cpp
libossiecluster_la_CXXFLAGS = -Wall
libossiecluster_la_CXXFLAGS += $(REDHAWK_CFLAGS) $(BOOST_CPPFLAGS)
