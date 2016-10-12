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

# This library is a stub to allow older code to link against it, but does not
# provide any functionality. May be removed entirely prior to 1.9 release.
CFDIR=$(top_srcdir)/base
lib_LTLIBRARIES = libredhawk_log4_appender.la
libredhawk_log4_appender_la_SOURCES = corba.cpp \
				RH_LogEventAppender.cpp

#libredhawk_log4_appender_la_CXXFLAGS = -Wall -I. -I$(CFDIR)/include $(BOOST_CPPFLAGS)  $(OMNIORB_CFLAGS) $(LOG4CXX_FLAGS) -DDEBUG_ON
libredhawk_log4_appender_la_CXXFLAGS = -Wall -I. -I$(CFDIR)/include $(BOOST_CPPFLAGS)  $(OMNIORB_CFLAGS) $(LOG4CXX_FLAGS) 
libredhawk_log4_appender_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(BOOST_REGEX_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(OMNIORB_LIBS) $(LOG4CXX_LIBS)
libredhawk_log4_appender_la_LDFLAGS = -Wall -version-info $(LIBREDHAWK_LOGGING_VERSION_INFO)
