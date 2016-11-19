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
noinst_PROGRAMS = check_cwd_cpp
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie

distclean-local:
	rm -rf m4
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

check_cwd_cpp_SOURCES = check_cwd_cpp.cpp check_cwd_cpp.h check_cwd_cpp_base.cpp check_cwd_cpp_base.h main.cpp
check_cwd_cpp_CXXFLAGS = -Wall $(PROJECTDEPS_CFLAGS) -I. $(BOOST_CPPFLAGS) $(INTERFACEDEPS_CFLAGS) $(redhawk_INCLUDES_auto)
check_cwd_cpp_LDADD = $(PROJECTDEPS_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) -lboost_iostreams $(INTERFACEDEPS_LIBS) $(redhawk_LDADD_auto)
check_cwd_cpp_LDFLAGS = -Wall $(redhawk_LDFLAGS_auto)
