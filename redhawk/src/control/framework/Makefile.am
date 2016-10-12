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

AM_CPPFLAGS = -I../include -I$(top_srcdir)/base/include

noinst_LTLIBRARIES = libossiedomain.la
libossiedomain_la_SOURCES = CorbaGC.cpp \
                            File_impl.cpp \
                            FileManager_impl.cpp \
                            FileSystem_impl.cpp \
                            GCThread.cpp \
                            helperFunctions.cpp \
                            POACreator.cpp \
                            prop_utils.cpp

libossiedomain_la_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) $(OMNICOS_CFLAGS) $(OMNIORB_CFLAGS) $(LOG4CXX_FLAGS) $(PERSISTENCE_CFLAGS)
libossiedomain_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(OMNICOS_LIBS) $(OMNIORB_LIBS) $(LOG4CXX_LIBS) $(PERSISTENCE_LIBS)
libossiedomain_la_LDFLAGS = -Wall

bin_PROGRAMS = nodeBooter
nodeBooter_SOURCES = nodebooter.cpp
nodeBooter_CXXFLAGS = $(BOOST_CPPFLAGS) 
nodeBooter_LDADD = $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) ../parser/libossieparser.la ./libossiedomain.la $(top_builddir)/base/framework/libossiecf.la $(top_builddir)/base/framework/idl/libossieidl.la  $(OMNICOS_LIBS) $(OMNIORB_LIBS)
nodeBooter_LDFLAGS = -static
bin_SCRIPTS = nodeCleanup.py

