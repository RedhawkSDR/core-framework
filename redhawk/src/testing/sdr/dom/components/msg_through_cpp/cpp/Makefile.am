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
CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = msg_through_cpp

distclean-local:
	rm -f config.*
	rm -rf autom4te.cache
	rm -f acinclude.m4
	rm -f aclocal.m4
	rm -f configure
	rm -f depcomp
	rm -f install-sh
	rm -f ltmain.sh
	rm -f Makefile.in
	rm -f missing
	rm -rf .deps

msg_through_cpp_SOURCES = msg_through_cpp.cpp msg_through_cpp.h msg_through_cpp_base.cpp msg_through_cpp_base.h main.cpp struct_props.h
msg_through_cpp_CXXFLAGS = -Wall $(PROJECTDEPS_CFLAGS) $(BOOST_CPPFLAGS) $(INTERFACEDEPS_CFLAGS) $(redhawk_INCLUDES_auto)  -I$(CFDIR)/include -I$(CFDIR)/include/ossie
msg_through_cpp_LDADD = $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(INTERFACEDEPS_LIBS) $(OMNIDYNAMIC_LIBS)
msg_through_cpp_LDFLAGS = -Wall $(redhawk_LDFLAGS_auto)

