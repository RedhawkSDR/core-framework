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

nodist_libossiecfjni_la_SOURCES = ossie/CF/jni_cf.cpp \
				  ossie/CF/jni_AggregateDevices.cpp \
				  ossie/CF/jni_QueryablePort.cpp \
				  ossie/CF/jni_StandardEvent.cpp \
				  ossie/CF/jni_WellKnownProperties.cpp \
				  ossie/CF/jni_PortTypes.cpp \
				  ossie/CF/jni_ExtendedEvent.cpp \
				  ossie/CF/jni_sandbox.cpp
libossiecfjni_la_CPPFLAGS = $(OMNIJNI_CPPFLAGS) $(OSSIE_CFLAGS)

nobase_nodist_include_HEADERS = ossie/CF/jni_cf.h \
				ossie/CF/jni_AggregateDevices.h \
				ossie/CF/jni_QueryablePort.h \
				ossie/CF/jni_StandardEvent.h \
				ossie/CF/jni_WellKnownProperties.h \
				ossie/CF/jni_PortTypes.h \
				ossie/CF/jni_ExtendedEvent.h \
				ossie/CF/jni_sandbox.h

libossiecfjni_la_LIBADD = COS/libcosjni.la $(OMNICOS_LIBS) $(OSSIEIDL_LIBS) $(OMNIJNI_LIBS)

clean-local:
	rm -rf ossie

ossie/CF:
	mkdir -p $@

ossie/CF/jni_%.cpp ossie/CF/jni_%.h : $(OSSIE_IDLDIR)/ossie/CF/%.idl | ossie/CF
	$(OMNIJNI_IDLJNI) -Wbincprefix=ossie/CF -Wbpkgprefix=CosNaming:COS -Wbpkgprefix=CosEventComm:COS -C ossie/CF $(IDLFLAGS) $<
