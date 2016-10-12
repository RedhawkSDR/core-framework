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

# Build a separate library for JNI interface to COS; it will be linked directly
# into ossiecfjni rather than installed. The headers, however, are installed to
# ${includedir}/ossie/COS.
noinst_LTLIBRARIES = libcosjni.la

# The COS modules are mapped to the org.omg package.
COSPKGS = CosEventComm CosNaming CosEventChannelAdmin
COSPREFIX = org.omg
IDLJNIFLAGS = $(COSPKGS:%=-Wbpkgprefix=%:$(COSPREFIX))

jni_CosEvent%.cpp jni_CosEvent%.h : $(OMNICOS_IDLDIR)/CosEvent%.idl
	$(AM_V_GEN)$(IDLJNICXX) -Wbincprefix=COS -Wbincext=.hh $(IDLJNIFLAGS) -I$(OMNICOS_IDLDIR) $<

jni_CosNam%.cpp jni_CosNam%.h : $(OMNICOS_IDLDIR)/CosNam%.idl
	$(AM_V_GEN)$(IDLJNICXX) -Wbstandalone $(IDLJNIFLAGS) -I$(OMNICOS_IDLDIR) $<

jni_CosEventChannelAdmin.o : jni_CosEventComm.h

nodist_libcosjni_la_SOURCES = jni_CosEventComm.cpp \
			      jni_CosEventChannelAdmin.cpp \
			      jni_CosNaming.cpp
libcosjni_la_CPPFLAGS = $(OMNIJNI_CPPFLAGS)

cosincludedir = $(pkgincludedir)/COS
nodist_cosinclude_HEADERS = jni_CosEventComm.h \
			    jni_CosEventChannelAdmin.h \
			    jni_CosNaming.h

CLEANFILES = jni_*.h jni_*.cpp
