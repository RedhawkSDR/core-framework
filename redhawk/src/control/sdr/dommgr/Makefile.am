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

domainmgrdir = $(SDR_ROOT)/dom/mgr
dist_domainmgr_DATA = DomainManager.spd.xml DomainManager.scd.xml DomainManager.prf.xml
domainmgr_PROGRAMS = DomainManager

DomainManager_SOURCES = applicationSupport.cpp \
                        connectionSupport.cpp \
                        AllocationManager_impl.cpp \
                        Application_impl.cpp \
                        ApplicationFactory_impl.cpp \
                        DomainManager_Channels.cpp \
                        DomainManager_impl.cpp \
                        main.cpp
DomainManager_CPPFLAGS = -I../../include -I../../parser -I$(top_srcdir)/base/include -I$(top_srcdir)/base $(BOOST_CPPFLAGS) $(OMNIORB_CFLAGS) $(LOG4CXX_FLAGS)
DomainManager_CXXFLAGS = -Wall
DomainManager_LDADD = ../../framework/libossiedomain.la ../../parser/libossieparser.la $(top_builddir)/base/framework/libossiecf.la $(top_builddir)/base/framework/idl/libossieidl.la  $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(BOOST_SERIALIZATION_LIB)  $(BOOST_REGEX_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(OMNICOS_LIBS) $(OMNIORB_LIBS) $(LOG4CXX_LIBS) $(PERSISTENCE_LIBS)
DomainManager_LDFLAGS = -static

