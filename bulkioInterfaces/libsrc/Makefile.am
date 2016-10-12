#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bulkio.pc

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = libbulkio-@BULKIO_API_VERSION@.la
libbulkio_@BULKIO_API_VERSION@_la_LDFLAGS = -version-info $(BULKIO_SO_VERSION)

libbulkio_@BULKIO_API_VERSION@_la_SOURCES = \
    cpp/bulkio.cpp \
    cpp/bulkio_in_port.cpp \
    cpp/bulkio_out_port.cpp \
	cpp/bulkio_attachable_port.cpp \
    cpp/bulkio_sri_helpers.cpp \
    cpp/bulkio_time_helpers.cpp \
    cpp/bulkio_p.h

## Define the list of public header files and their install location.
library_includedir = $(includedir)/bulkio
library_include_HEADERS = cpp/bulkio.h \
	cpp/BULKIO_Interfaces.h \
	cpp/bulkio_base.h \
	cpp/bulkio_traits.h \
	cpp/bulkio_in_port.h \
	cpp/bulkio_out_port.h \
	cpp/bulkio_attachable_base.h \
	cpp/bulkio_compat.h

## The generated configuration header is installed in its own subdirectory of
## $(libdir).  The reason for this is that the configuration information put
## into this header file describes the target platform the installed library
## has been built for.  Thus the file must not be installed into a location
## intended for architecture-independent files, as defined by the Filesystem
## Hierarchy Standard (FHS).
## The nodist_ prefix instructs Automake to not generate rules for including
## the listed files in the distribution on 'make dist'.  Files that are listed
## in _HEADERS variables are normally included in the distribution, but the
## configuration header file is generated at configure time and should not be
## shipped with the source tarball.
#bulkio_libincludedir = $(libdir)/bulkio-$(BULKIO_API_VERSION)/include

libbulkio_@BULKIO_API_VERSION@_la_CXXFLAGS = -Wall -I./cpp -DLOGGING $(BULKIO_INF_INCLUDES) $(BOOST_CPPFLAGS) $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)

###############################################################################
# Python

# Mimic automake silent rules
OSSIE_V_pysetup = $(ossie__v_pysetup_$(V))
ossie__v_pysetup_ = $(ossie__v_pysetup__$(AM_DEFAULT_VERBOSITY))
ossie__v_pysetup_0 = --quiet
ossie__v_pysetup__0 = $(ossie__v_pysetup_0)

all-python: build-python

build-python:
	python setup.py $(OSSIE_V_pysetup) build

install-python:
	test -n "$(DESTDIR)" && buildroot="--root=$(DESTDIR)"; \
	  python setup.py install $$buildroot --prefix=$(prefix) --install-purelib=$(prefix)/lib/python --force

clean-python:
	python setup.py clean --all

###############################################################################
# Java

if HAVE_JAVASUPPORT

@rh_jarfile_rules@

JAVA_DIR := java
JAVA_SRCDIR := $(JAVA_DIR)/src

JAVA_SRCS := Const.java \
DataTransfer.java \
InCharPort.java \
InDoublePort.java \
InFilePort.java \
InFloatPort.java \
InInt16Port.java \
InInt32Port.java \
InInt64Port.java \
InInt8Port.java \
InLongLongPort.java \
InLongPort.java \
InOctetPort.java \
InSDDSPort.java \
InVITA49Port.java \
InShortPort.java \
InUInt16Port.java \
InUInt32Port.java \
InUInt64Port.java \
InUInt8Port.java \
InULongLongPort.java \
InULongPort.java \
InUShortPort.java \
InXMLPort.java \
linkStatistics.java \
OutCharPort.java \
OutDoublePort.java \
OutFilePort.java \
OutFloatPort.java \
OutInt16Port.java \
OutInt32Port.java \
OutInt64Port.java \
OutInt8Port.java \
OutLongLongPort.java \
OutLongPort.java \
OutOctetPort.java \
OutSDDSPort.java \
OutVITA49Port.java \
OutShortPort.java \
OutUInt16Port.java \
OutUInt32Port.java \
OutUInt64Port.java \
OutUInt8Port.java \
OutULongLongPort.java \
OutULongPort.java \
OutUShortPort.java \
OutXMLPort.java \
connection_descriptor_struct.java \
SriMapStruct.java \
queueSemaphore.java \
SizeOf.java \
SriListener.java \
ConnectionEventListener.java \
sriState.java \
utils.java \
sdds/SDDSStream.java \
sdds/SDDSStreamAttachment.java \
sdds/SDDSStreamContainer.java \
vita49/VITA49Stream.java \
vita49/VITA49StreamAttachment.java \
vita49/VITA49StreamContainer.java \
sri/Comparator.java \
sri/DefaultComparator.java \
sri/utils.java \
time/Comparator.java \
time/DefaultComparator.java \
time/utils.java

java_JARFILES = bulkio.jar
java_DATA = bulkio.src.jar

bulkio_jar_SOURCE = $(addprefix $(JAVA_SRCDIR)/bulkio/,$(JAVA_SRCS))
bulkio_jar_CLASSPATH = $(OSSIE_CLASSPATH):$(top_builddir)/BULKIOInterfaces.jar
bulkio_jar_MANIFEST = $(JAVA_DIR)/META-INF/MANIFEST.MF

bulkio.jar: $(top_builddir)/BULKIOInterfaces.jar

bulkio.src.jar: $(JAVA_DIR)/META-INF/MANIFEST.MF.src $(bulkio_jar_SOURCE)
	$(RH_V_JAR)$(JAR) cmf $< $@ -C $(JAVA_SRCDIR) .

CLEANFILES = bulkio.src.jar

endif

###############################################################################
# General
#

.PHONY: all-local all-python build-python install-python clean-python reallyclean

all-local: all-python

install-exec-hook: install-python

clean-local: clean-python

reallyclean: distclean
	rm -f Makefile.in
