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

lib_LTLIBRARIES = libburstioInterfaces.la

nodist_libburstioInterfaces_la_SOURCES = redhawk/BURSTIO/burstioDataTypesSK.cpp \
					 redhawk/BURSTIO/burstioDataTypesDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstByteSK.cpp \
					 redhawk/BURSTIO/burstio_burstByteDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstDoubleSK.cpp \
					 redhawk/BURSTIO/burstio_burstDoubleDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstFloatSK.cpp \
					 redhawk/BURSTIO/burstio_burstFloatDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstLongSK.cpp \
					 redhawk/BURSTIO/burstio_burstLongDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstLongLongSK.cpp \
					 redhawk/BURSTIO/burstio_burstLongLongDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstShortSK.cpp \
					 redhawk/BURSTIO/burstio_burstShortDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstUbyteSK.cpp \
					 redhawk/BURSTIO/burstio_burstUbyteDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstUlongSK.cpp \
					 redhawk/BURSTIO/burstio_burstUlongDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstUlongLongSK.cpp \
					 redhawk/BURSTIO/burstio_burstUlongLongDynSK.cpp \
					 redhawk/BURSTIO/burstio_burstUshortSK.cpp \
					 redhawk/BURSTIO/burstio_burstUshortDynSK.cpp

nobase_nodist_include_HEADERS = redhawk/BURSTIO/burstioDataTypes.h \
				redhawk/BURSTIO/burstio_burstByte.h \
				redhawk/BURSTIO/burstio_burstDouble.h \
				redhawk/BURSTIO/burstio_burstFloat.h \
				redhawk/BURSTIO/burstio_burstLong.h \
				redhawk/BURSTIO/burstio_burstLongLong.h \
				redhawk/BURSTIO/burstio_burstShort.h \
				redhawk/BURSTIO/burstio_burstUbyte.h \
				redhawk/BURSTIO/burstio_burstUlong.h \
				redhawk/BURSTIO/burstio_burstUlongLong.h \
				redhawk/BURSTIO/burstio_burstUshort.h

libburstioInterfaces_la_CPPFLAGS = -I . $(OSSIE_CFLAGS)

BUILT_SOURCES = $(nobase_nodist_include_HEADERS) $(nodist_libburstioInterfaces_la_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

idl_srcdir = $(top_srcdir)/src/idl
burstio_idldir = $(idl_srcdir)/redhawk/BURSTIO
burstio_builddir = redhawk/BURSTIO

$(burstio_builddir):
	mkdir -p $@

$(burstio_builddir)/%DynSK.cpp $(burstio_builddir)/%SK.cpp $(burstio_builddir)/%.h: $(burstio_idldir)/%.idl | $(burstio_builddir)
	$(AM_V_GEN)$(IDL) -I $(idl_srcdir) -I $(OSSIE_IDLDIR) -I $(BULKIO_IDLDIR) -C $(burstio_builddir) -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<


lib_LTLIBRARIES += libburstio.la

libburstio_la_SOURCES =  lib/BurstByteIn.cpp
libburstio_la_SOURCES += lib/BurstByteOut.cpp
libburstio_la_SOURCES += lib/BurstDoubleIn.cpp
libburstio_la_SOURCES += lib/BurstDoubleOut.cpp
libburstio_la_SOURCES += lib/BurstFloatIn.cpp
libburstio_la_SOURCES += lib/BurstFloatOut.cpp
libburstio_la_SOURCES += lib/BurstLongIn.cpp
libburstio_la_SOURCES += lib/BurstLongLongIn.cpp
libburstio_la_SOURCES += lib/BurstLongLongOut.cpp
libburstio_la_SOURCES += lib/BurstLongOut.cpp
libburstio_la_SOURCES += lib/BurstShortIn.cpp
libburstio_la_SOURCES += lib/BurstShortOut.cpp
libburstio_la_SOURCES += lib/BurstStatistics.cpp
libburstio_la_SOURCES += lib/BurstUbyteIn.cpp
libburstio_la_SOURCES += lib/BurstUbyteOut.cpp
libburstio_la_SOURCES += lib/BurstUlongIn.cpp
libburstio_la_SOURCES += lib/BurstUlongLongIn.cpp
libburstio_la_SOURCES += lib/BurstUlongLongOut.cpp
libburstio_la_SOURCES += lib/BurstUlongOut.cpp
libburstio_la_SOURCES += lib/BurstUshortIn.cpp
libburstio_la_SOURCES += lib/BurstUshortOut.cpp
libburstio_la_SOURCES += lib/debug_impl.h
libburstio_la_SOURCES += lib/InPortImpl.h
libburstio_la_SOURCES += lib/OutPortImpl.h
libburstio_la_SOURCES += lib/utils.cpp

libburstio_la_CPPFLAGS = -I $(srcdir)/include -I redhawk $(OSSIE_CFLAGS) $(BOOST_CPPFLAGS)
libburstio_la_LIBADD = $(BOOST_LDFLAGS)
libburstio_la_LDFLAGS = -lburstioInterfaces 

libincludedir = $(includedir)/redhawk/burstio
libinclude_HEADERS =  include/burstio/burstio.h
libinclude_HEADERS += include/burstio/BurstPacket.h
libinclude_HEADERS += include/burstio/BurstStatistics.h
libinclude_HEADERS += include/burstio/ExecutorService.h
libinclude_HEADERS += include/burstio/InPortDecl.h
libinclude_HEADERS += include/burstio/OutPortDecl.h
libinclude_HEADERS += include/burstio/PortTraits.h
libinclude_HEADERS += include/burstio/UsesPort.h
libinclude_HEADERS += include/burstio/utils.h
libinclude_HEADERS += include/burstio/debug.h
