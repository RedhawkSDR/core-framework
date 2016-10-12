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

# vim: noet: softtabstop=0

include $(top_srcdir)/aminclude/idlj.am
include $(top_srcdir)/aminclude/jarfile.am

SUBDIRS = jni . ossie

CF_IDLDIR = $(top_srcdir)/idl

# The COS modules are mapped to the org.omg package.
COSPKGS = CosEventComm CosNaming CosEventChannelAdmin
COSPREFIX = org.omg

# Suppress warnings due to unrecognized omniORB pragmas in COS packages
IDLJFLAGS = -noWarn $(COSPKGS:%=-pkgPrefix % $(COSPREFIX)) -i $(IDLDIR) -i $(OMNICOS_IDLDIR) -i $(CF_IDLDIR)
IDLJNIFLAGS = -Wblibname=ossiecfjni $(COSPKGS:%=-Wbpkgprefix=%:$(COSPREFIX)) -I $(IDLDIR) -I $(OMNICOS_IDLDIR) -I $(CF_IDLDIR)
IDLJ_BUILDDIR = cf/src
IDLJNI_BUILDDIR = $(IDLJ_BUILDDIR)

# Source location for CF IDL, to generate Java and Java-side JNI classes
vpath %.idl $(CF_IDLDIR)/ossie/CF

# The ExtendedEvent IDL depends on COS packages that are not provided in the
# default JRE CORBA implementation, so they must be compiled into our jar.
vpath Cos%.idl $(OMNICOS_IDLDIR)

idlj_IDLSRC = CosEventComm.idl CosEventChannelAdmin.idl \
	      LogInterfaces.idl EventChannelManager.idl \
	      cf.idl DataType.idl Port.idl PortTypes.idl StandardEvent.idl AggregateDevices.idl \
	      ExtendedEvent.idl QueryablePort.idl WellKnownProperties.idl sandbox.idl

# CosNaming is included with the JRE, so only build JNI bindings.
idljni_IDLSRC = CosNaming.idl $(idlj_IDLSRC)

.INTERMEDIATE: META-INF/MANIFEST.MF packageList.txt

packageList.txt: $(CFInterfaces_jar_SOURCE)
	$(AM_V_at)echo -n "Export-Package:" > $@
	$(AM_V_at)find $(IDLJ_BUILDDIR) -mindepth 1 -type d | $(SED) -e 's|^$(IDLJ_BUILDDIR)/| |' -e 's/\//\./g' -e '$$ ! s/$$/,/' >> $@

META-INF/MANIFEST.MF: META-INF/MANIFEST.MF.in packageList.txt Makefile
	$(AM_V_GEN)stamp=`date +%Y%m%d%H%M`; \
	$(SED) \
	  -e '/@EXPORT_LIST[@]/r packageList.txt' \
	  -e '/@EXPORT_LIST[@]/d' \
	  -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|' \
	  -e "s|@DATE_TIME_STAMP[@]|$$stamp|" \
	  $@.in > $@

CFInterfaces_jar_SOURCE = $(idlj_SOURCE) $(idljni_SOURCE)
CFInterfaces_jar_JAVACFLAGS = -g
CFInterfaces_jar_CLASSPATH = apache-commons-lang-2.4.jar:$(omnijni_builddir)/src/java
CFInterfaces_jar_MANIFEST = META-INF/MANIFEST.MF
CFInterfaces_jar_JARADD = -C $(IDLJ_BUILDDIR) . -C $(omnijni_builddir)/src/java omnijni

java_JARFILES = CFInterfaces.jar
java_DATA = log4j-1.2.15.jar apache-commons-lang-2.4.jar

clean-local:
	rm -rf $(IDLJ_BUILDDIR)
