#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
@rh_idlj_rules@
@rh_jarfile_rules@

idl_srcdir = $(top_srcdir)/src/idl

###############################################################################
# Java IDL library
###############################################################################
java_JARFILES = BURSTIOInterfaces.jar

IDLJFLAGS = -i $(idl_srcdir) -i $(OSSIE_IDLDIR) -i $(BULKIO_IDLDIR)
IDLJNIFLAGS = -I $(idl_srcdir) -I $(OSSIE_IDLDIR) -I $(BULKIO_IDLDIR)

IDLJ_BUILDDIR = src
IDLJNI_BUILDDIR = $(IDLJ_BUILDDIR)

vpath %.idl $(idl_srcdir)/redhawk/BURSTIO

IDLSRC =  burstioDataTypes.idl
IDLSRC += burstio_burstByte.idl
IDLSRC += burstio_burstDouble.idl
IDLSRC += burstio_burstFloat.idl
IDLSRC += burstio_burstLong.idl
IDLSRC += burstio_burstLongLong.idl
IDLSRC += burstio_burstShort.idl
IDLSRC += burstio_burstUbyte.idl
IDLSRC += burstio_burstUlong.idl
IDLSRC += burstio_burstUlongLong.idl
IDLSRC += burstio_burstUshort.idl

idlj_IDLSRC = $(IDLSRC)
idljni_IDLSRC = $(IDLSRC)

nodist_BURSTIOInterfaces_jar_SOURCE = $(idlj_SOURCE) $(idljni_SOURCE)
BURSTIOInterfaces_jar_CLASSPATH = $(OSSIE_CLASSPATH):$(BULKIO_CLASSPATH)
BURSTIOInterfaces_jar_JAVACFLAGS = -g
BURSTIOInterfaces_jar_JARADD = -C $(IDLJ_BUILDDIR) .

###############################################################################
# Java implementation library
###############################################################################
burstio_jar_SOURCE =  burstio/BurstPacket.java
burstio_jar_SOURCE += burstio/ConnectionAdapter.java
burstio_jar_SOURCE += burstio/ConnectionListener.java
burstio_jar_SOURCE += burstio/InPort.java
burstio_jar_SOURCE += burstio/InPortImpl.java
burstio_jar_SOURCE += burstio/OutPort.java
burstio_jar_SOURCE += burstio/OutputPolicy.java
burstio_jar_SOURCE += burstio/RoutingMode.java
burstio_jar_SOURCE += burstio/Utils.java
burstio_jar_SOURCE += burstio/stats/Analysis.java
burstio_jar_SOURCE += burstio/stats/AbstractStatistics.java
burstio_jar_SOURCE += burstio/stats/ReceiverAnalysis.java
burstio_jar_SOURCE += burstio/stats/ReceiverStatistics.java
burstio_jar_SOURCE += burstio/stats/ReceiverStatPoint.java
burstio_jar_SOURCE += burstio/stats/SenderStatistics.java
burstio_jar_SOURCE += burstio/stats/StatPoint.java
burstio_jar_SOURCE += burstio/traits/BurstTraits.java

burstio_jar_BUILT_SOURCE =  burstio/BurstByteIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstDoubleIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstFloatIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstLongLongIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstLongIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstShortIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstUbyteIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstUlongIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstUlongLongIn.java
burstio_jar_BUILT_SOURCE += burstio/BurstUshortIn.java

burstio_jar_BUILT_SOURCE += burstio/BurstByteOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstDoubleOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstFloatOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstLongLongOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstLongOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstShortOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstUbyteOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstUlongOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstUlongLongOut.java
burstio_jar_BUILT_SOURCE += burstio/BurstUshortOut.java

burstio_jar_BUILT_SOURCE += burstio/BytePacket.java
burstio_jar_BUILT_SOURCE += burstio/DoublePacket.java
burstio_jar_BUILT_SOURCE += burstio/FloatPacket.java
burstio_jar_BUILT_SOURCE += burstio/LongLongPacket.java
burstio_jar_BUILT_SOURCE += burstio/LongPacket.java
burstio_jar_BUILT_SOURCE += burstio/ShortPacket.java
burstio_jar_BUILT_SOURCE += burstio/UbytePacket.java
burstio_jar_BUILT_SOURCE += burstio/UlongPacket.java
burstio_jar_BUILT_SOURCE += burstio/UlongLongPacket.java
burstio_jar_BUILT_SOURCE += burstio/UshortPacket.java

burstio_jar_BUILT_SOURCE += burstio/traits/ByteTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/DoubleTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/FloatTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/LongLongTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/LongTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/ShortTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/UbyteTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/UlongTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/UlongLongTraits.java
burstio_jar_BUILT_SOURCE += burstio/traits/UshortTraits.java

# Template for sed script generation of Java files
sed_generate = mkdir -p $(@D); sed -f $(srcdir)/sed/$*.sed $< > $@

burstio/traits/%Traits.java : burstio/traits/Traits.java.template sed/%.sed
	$(AM_V_GEN)$(sed_generate)

burstio/Burst%In.java : burstio/BurstIn.java.template sed/%.sed
	$(AM_V_GEN)$(sed_generate)

burstio/Burst%Out.java : burstio/BurstOut.java.template sed/%.sed
	$(AM_V_GEN)$(sed_generate)

burstio/%Packet.java : burstio/Packet.java.template sed/%.sed
	$(AM_V_GEN)$(sed_generate)

java_JARFILES += burstio.jar

nodist_burstio_jar_SOURCE = $(burstio_jar_BUILT_SOURCE)
burstio_jar_CLASSPATH = $(OSSIE_CLASSPATH):$(BULKIO_CLASSPATH):BURSTIOInterfaces.jar
burstio_jar_JAVACFLAGS = -g -Xlint

# Evaluates to a non-empty string if the build is out-of-place; build is always
# ".", so if srcdir is just ".", the resulting string is empty (i.e., false).
outofplace = $(subst .,,$(srcdir))

# Add the source, and if the build is out-of-place, the generated source
burstio_jar_JARADD = -C $(srcdir) burstio $(if $(outofplace), $(burstio_jar_BUILT_SOURCE))

burstio.jar : BURSTIOInterfaces.jar

# Add the sed scripts and Java templates to the distribution
EXTRA_DIST = sed
EXTRA_DIST += burstio/BurstIn.java.template
EXTRA_DIST += burstio/BurstOut.java.template
EXTRA_DIST += burstio/Packet.java.template
EXTRA_DIST += burstio/traits/Traits.java.template
CLEANFILES = $(burstio_jar_BUILT_SOURCE)

###############################################################################
# JNI C++ library
###############################################################################
lib_LTLIBRARIES = libburstiojni.la

redhawk/BURSTIO:
	mkdir -p $@

redhawk/BURSTIO/jni_%.cpp redhawk/BURSTIO/jni_%.h: $(idl_srcdir)/redhawk/BURSTIO/%.idl | redhawk/BURSTIO
	$(AM_V_GEN)$(IDLJNICXX) -I $(idl_srcdir) -I $(OSSIE_IDLDIR) -I $(BULKIO_IDLDIR) -Wbincprefix=redhawk/BURSTIO -C redhawk/BURSTIO $<

libburstiojni_la_CPPFLAGS = -I $(top_builddir)/src/cpp $(OSSIE_CFLAGS) $(BULKIO_CFLAGS) $(JNI_CPPFLAGS)
libburstiojni_la_LIBADD = $(top_builddir)/src/cpp/libburstioInterfaces.la $(JNI_LIBS)

nodist_libburstiojni_la_SOURCES = \
	redhawk/BURSTIO/jni_burstioDataTypes.cpp \
	redhawk/BURSTIO/jni_burstio_burstByte.cpp \
	redhawk/BURSTIO/jni_burstio_burstDouble.cpp \
	redhawk/BURSTIO/jni_burstio_burstFloat.cpp \
	redhawk/BURSTIO/jni_burstio_burstLong.cpp \
	redhawk/BURSTIO/jni_burstio_burstLongLong.cpp \
	redhawk/BURSTIO/jni_burstio_burstShort.cpp \
	redhawk/BURSTIO/jni_burstio_burstUbyte.cpp \
	redhawk/BURSTIO/jni_burstio_burstUlong.cpp \
	redhawk/BURSTIO/jni_burstio_burstUlongLong.cpp \
	redhawk/BURSTIO/jni_burstio_burstUshort.cpp

nobase_nodist_include_HEADERS = \
	redhawk/BURSTIO/jni_burstioDataTypes.h \
	redhawk/BURSTIO/jni_burstio_burstByte.h \
	redhawk/BURSTIO/jni_burstio_burstDouble.h \
	redhawk/BURSTIO/jni_burstio_burstFloat.h \
	redhawk/BURSTIO/jni_burstio_burstLong.h \
	redhawk/BURSTIO/jni_burstio_burstLongLong.h \
	redhawk/BURSTIO/jni_burstio_burstShort.h \
	redhawk/BURSTIO/jni_burstio_burstUbyte.h \
	redhawk/BURSTIO/jni_burstio_burstUlong.h \
	redhawk/BURSTIO/jni_burstio_burstUlongLong.h \
	redhawk/BURSTIO/jni_burstio_burstUshort.h

BUILT_SOURCES = $(nodist_libburstiojni_la_SOURCES) $(nobase_nodist_include_HEADERS)
CLEANFILES += $(BUILT_SOURCES)
