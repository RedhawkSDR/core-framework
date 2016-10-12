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

SUBDIRS = COS

IDLFLAGS = -I$(OSSIE_IDLDIR) -I$(IDLDIR) -I$(OMNICOS_IDLDIR)

lib_LTLIBRARIES = libossiecfjni.la

nodist_libossiecfjni_la_SOURCES = ossie/CF/jni_LogInterfaces.cpp \
				  ossie/CF/jni_cf.cpp \
				  ossie/CF/jni_AggregateDevices.cpp \
				  ossie/CF/jni_QueryablePort.cpp \
				  ossie/CF/jni_StandardEvent.cpp \
				  ossie/CF/jni_WellKnownProperties.cpp \
				  ossie/CF/jni_PortTypes.cpp \
				  ossie/CF/jni_ExtendedEvent.cpp \
				  ossie/CF/jni_sandbox.cpp 

libossiecfjni_la_CPPFLAGS = $(OMNIJNI_CPPFLAGS) $(OSSIE_CFLAGS)
libossiecfjni_la_LDFLAGS = -version-info $(LIBOSSIECFJNI_VERSION_INFO)

nobase_nodist_include_HEADERS = ossie/CF/jni_LogInterfaces.h \
				ossie/CF/jni_cf.h \
				ossie/CF/jni_AggregateDevices.h \
				ossie/CF/jni_QueryablePort.h \
				ossie/CF/jni_StandardEvent.h \
				ossie/CF/jni_WellKnownProperties.h \
				ossie/CF/jni_PortTypes.h \
				ossie/CF/jni_ExtendedEvent.h \
				ossie/CF/jni_sandbox.h

# Ensure that the headers get built first to avoid potential race conditions,
# as many of the .cpp files depend on one or more other headers
BUILT_SOURCES = $(nobase_nodist_include_HEADERS)

libossiecfjni_la_LIBADD = COS/libcosjni.la $(OMNICOS_LIBS) $(OSSIEIDL_LIBS) $(OMNIJNI_LIBS)

clean-local:
	rm -rf ossie

ossie/CF:
	mkdir -p $@

# The COS modules are mapped to the org.omg package.
COSPKGS = CosEventComm CosNaming CosEventChannelAdmin
COSPREFIX = org.omg
IDLJNIFLAGS = $(COSPKGS:%=-Wbpkgprefix=%:$(COSPREFIX))

ossie/CF/jni_%.cpp ossie/CF/jni_%.h : $(OSSIE_IDLDIR)/ossie/CF/%.idl | ossie/CF
	$(AM_V_GEN)$(IDLJNICXX) -Wbincprefix=ossie/CF $(IDLJNIFLAGS) -C ossie/CF $(IDLFLAGS) $<
