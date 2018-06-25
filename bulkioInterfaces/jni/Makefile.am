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

lib_LTLIBRARIES = libbulkiojni.la

BULKIO_IDLDIR = $(top_srcdir)/idl
BULKIO_CFLAGS = -I$(top_builddir)/src/cpp -I$(top_builddir)/src/cpp/ossie
BULKIO_LIBS = $(top_builddir)/libbulkioInterfaces.la

ossie/BULKIO :
	@mkdir -p $@

ossie/BULKIO/jni_%.cpp ossie/BULKIO/jni_%.h: $(BULKIO_IDLDIR)/ossie/BULKIO/%.idl | ossie/BULKIO
	$(AM_V_GEN)$(IDLJNICXX) -I $(BULKIO_IDLDIR) -I $(OSSIE_IDLDIR) -I $(OMNICOS_IDLDIR) -I $(OMNIORB_IDLDIR) -Wbincprefix=BULKIO -C ossie/BULKIO $<

libbulkiojni_la_CPPFLAGS = -Iossie/BULKIO -I. $(BULKIO_CFLAGS) $(JNI_CPPFLAGS) $(OSSIE_CFLAGS)
libbulkiojni_la_CXXFLAGS = -Wall
libbulkiojni_la_LIBADD = $(JNI_LIBS) $(BULKIO_LIBS)

nodist_libbulkiojni_la_SOURCES = \
	ossie/BULKIO/jni_bulkioDataTypes.cpp \
	ossie/BULKIO/jni_bio_runtimeStats.cpp \
	ossie/BULKIO/jni_bio_updateSRI.cpp \
	ossie/BULKIO/jni_bio_dataFloat.cpp \
	ossie/BULKIO/jni_bio_dataFile.cpp \
	ossie/BULKIO/jni_bio_dataXML.cpp \
	ossie/BULKIO/jni_bio_dataShort.cpp \
	ossie/BULKIO/jni_bio_dataDouble.cpp \
	ossie/BULKIO/jni_bio_dataBit.cpp \
	ossie/BULKIO/jni_bio_dataChar.cpp \
	ossie/BULKIO/jni_bio_dataOctet.cpp \
	ossie/BULKIO/jni_bio_dataUlong.cpp \
	ossie/BULKIO/jni_bio_dataUshort.cpp \
	ossie/BULKIO/jni_bio_dataLong.cpp \
	ossie/BULKIO/jni_bio_dataLongLong.cpp \
	ossie/BULKIO/jni_bio_dataUlongLong.cpp \
	ossie/BULKIO/jni_bio_dataSDDS.cpp \
        ossie/BULKIO/jni_bio_dataVITA49.cpp
nobase_nodist_include_HEADERS = \
	ossie/BULKIO/jni_bulkioDataTypes.h \
	ossie/BULKIO/jni_bio_runtimeStats.h \
	ossie/BULKIO/jni_bio_updateSRI.h \
	ossie/BULKIO/jni_bio_dataFloat.h \
	ossie/BULKIO/jni_bio_dataFile.h \
	ossie/BULKIO/jni_bio_dataXML.h \
	ossie/BULKIO/jni_bio_dataShort.h \
	ossie/BULKIO/jni_bio_dataDouble.h \
	ossie/BULKIO/jni_bio_dataBit.h \
	ossie/BULKIO/jni_bio_dataChar.h \
	ossie/BULKIO/jni_bio_dataOctet.h \
	ossie/BULKIO/jni_bio_dataUlong.h \
	ossie/BULKIO/jni_bio_dataUshort.h \
	ossie/BULKIO/jni_bio_dataLong.h \
	ossie/BULKIO/jni_bio_dataLongLong.h \
	ossie/BULKIO/jni_bio_dataUlongLong.h \
	ossie/BULKIO/jni_bio_dataSDDS.h \
	ossie/BULKIO/jni_bio_dataVITA49.h
CLEANFILES = $(nodist_libbulkiojni_la_SOURCES) $(nobase_nodist_include_HEADERS)
