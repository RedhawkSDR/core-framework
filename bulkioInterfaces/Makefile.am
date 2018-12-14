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

ACLOCAL_AMFLAGS = -I m4 -I ${OSSIEHOME}/share/aclocal/ossie

###############################################################################
# CONFIGURE YOUR INTERFACES LIBRARY HERE
#
# vim: noet: softtabstop=0
#
# This Makefile.am will require GNUMake (because it uses special GNUMake
# functions).  As an alternative, all of the specical functions can be
# replaced with configure.ac macros and AC_SUBST calls.
#
###############################################################################
# As a Redhawk convention this should always be 'redhawk'. BulkIO does not
# follow the Redhawk conventione for legacy compatibility.
LIBRARY_NAME = ossie

# IMPORTANT: Per REDHAWK conventions, there must be one and only
# one CORBA module/namespace declared in a given IDL Interfaces
# library.  By convention, this name is capitalized and concise.
# The module can have sub-modules.
IDL_MODULE := BULKIO

# A space separated list of IDL files to include in your library
# We do not use the auto variable as order is important to us
IDL_FILES := bulkioDataTypes.idl \
bio_runtimeStats.idl \
bio_updateSRI.idl \
bio_dataFloat.idl \
bio_dataFile.idl \
bio_dataXML.idl \
bio_dataShort.idl \
bio_dataDouble.idl bio_dataChar.idl \
bio_dataOctet.idl bio_dataUlong.idl \
bio_dataUshort.idl \
bio_dataLong.idl \
bio_dataLongLong.idl \
bio_dataUlongLong.idl \
bio_dataBit.idl \
bio_dataSDDS.idl \
bio_dataVITA49.idl

# IDL files that are marked as "internal" are installed to a separate directory
# to discourage external use
IDL_INTERNAL_FILES := internal/bio_dataExt.idl

# External IDL namespaces that your IDLs reference, comment this line
# out if your IDL has no external references
IDL_EXTERNS := CF

# In some cases, you may need to define python externs 
#     f:p   Assume Python stub file for file f is in package p.
PYTHON_EXTERNS := PortTypes:ossie.cf DataType:ossie.cf Port:ossie.cf QueryablePort:ossie.cf NegotiablePort:ossie.cf bulkioDataTypes:bulkio.bulkioInterfaces

# A Qualifier to append to the version, by default this is a timestamp of the build
BUNDLE_QUALIFIER := v$(shell date +%Y%m%d%H%M%S)

# A user understandable name for the bundle
BUNDLE_NAME := $(IDL_MODULE) IDL Interface

# The Vendor that produced the bundle / idl
BUNDLE_VENDOR = United States Government

# Version of java the source files support
JAVA_SOURCE_VERSION = @JAVA_SOURCE_VERSION@

# Version of java to compile to
JAVA_TARGET_VERSION = $(JAVA_SOURCE_VERSION)

# The required execution envirornment, this should be a JRE version or OSGI environment specification
BUNDLE_REQUIRED_EXECUTION_ENVIRONMENT := J2SE-$(JAVA_TARGET_VERSION)

# You have to manually set the _SOURCES, automake/libtool does not support
# makefile variable substituion here...so unfortunately each IDL file
# needs to be listed here...for each input IDL you will need to add three
# sources.  For example:
#        some_file.idl becomes ->
#            src/cpp/some_fileSK.cpp src/cpp/some_fileDynSK.cpp src/cpp/some_file.h
lib_LTLIBRARIES = libbulkioInterfaces.la
libbulkioInterfaces_la_LDFLAGS = -version-info $(LIBBULKIOINTERFACES_VERSION_INFO)
libbulkioInterfaces_la_LIBADD = $(OSSIE_LIBS)

libbulkioInterfaces_la_SOURCES = \
	src/cpp/ossie/BULKIO/bulkioDataTypesSK.cpp \
	src/cpp/ossie/BULKIO/bulkioDataTypesDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_runtimeStatsSK.cpp \
	src/cpp/ossie/BULKIO/bio_runtimeStatsDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_updateSRISK.cpp \
	src/cpp/ossie/BULKIO/bio_updateSRIDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataFloatSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataFloatDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataFileSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataFileDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataXMLSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataXMLDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataShortSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataShortDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataDoubleSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataDoubleDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataCharSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataCharDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataOctetSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataOctetDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUlongSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUlongDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUshortSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUshortDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataLongSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataLongDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataLongLongSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataLongLongDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUlongLongSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataUlongLongDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataBitSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataBitDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataSDDSSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataSDDSDynSK.cpp \
	src/cpp/ossie/BULKIO/bio_dataVITA49SK.cpp \
	src/cpp/ossie/BULKIO/bio_dataVITA49DynSK.cpp \
	src/cpp/ossie/BULKIO/internal/bio_dataExtSK.cpp \
	src/cpp/ossie/BULKIO/internal/bio_dataExtDynSK.cpp 

pkgincludedir = $(includedir)/$(LIBRARY_NAME)/$(IDL_MODULE)
pkginclude_HEADERS = \
	src/cpp/ossie/BULKIO/bulkioDataTypes.h \
	src/cpp/ossie/BULKIO/bio_runtimeStats.h \
	src/cpp/ossie/BULKIO/bio_updateSRI.h \
	src/cpp/ossie/BULKIO/bio_dataFloat.h \
	src/cpp/ossie/BULKIO/bio_dataFile.h \
	src/cpp/ossie/BULKIO/bio_dataXML.h \
	src/cpp/ossie/BULKIO/bio_dataShort.h \
	src/cpp/ossie/BULKIO/bio_dataDouble.h \
	src/cpp/ossie/BULKIO/bio_dataChar.h \
	src/cpp/ossie/BULKIO/bio_dataOctet.h \
	src/cpp/ossie/BULKIO/bio_dataUlong.h \
	src/cpp/ossie/BULKIO/bio_dataUshort.h \
	src/cpp/ossie/BULKIO/bio_dataLong.h \
	src/cpp/ossie/BULKIO/bio_dataLongLong.h \
	src/cpp/ossie/BULKIO/bio_dataUlongLong.h \
	src/cpp/ossie/BULKIO/bio_dataBit.h \
	src/cpp/ossie/BULKIO/bio_dataSDDS.h \
	src/cpp/ossie/BULKIO/bio_dataVITA49.h

pkginternalincludedir = $(pkgincludedir)/internal
pkginternalinclude_HEADERS = \
	src/cpp/ossie/BULKIO/internal/bio_dataExt.h

BUILT_SOURCES = $(libbulkioInterfaces_la_SOURCES) $(pkginclude_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

###############################################################################
# DO NOT MODIFY ANY LINES BELOW HERE
###############################################################################
.PHONY: all-local all-python install-python clean-python clean-java zip-src
LOWER_CASE_IDL_MODULE :=$(shell echo $(IDL_MODULE) | tr A-Z a-z)

idldir = $(datadir)/idl/$(LIBRARY_NAME)/$(IDL_MODULE)
dist_idl_DATA =  $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))

idlinternaldir = $(idldir)/internal
dist_idlinternal_DATA = $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_INTERNAL_FILES))

all-local: all-python

install-exec-hook: install-python

clean-local: clean-python clean-java
	rm -rf build

distclean-local:
	rm -rf src

# Always build the current directory first (this is hack-ish, but the
# alternative is to combine the Makefile.am's)
SUBDIRS = . libsrc

###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = $(PACKAGE_NAME).pc

bulkio_builddir = src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)

libbulkioInterfaces_la_CXXFLAGS = -Wall -I src/cpp -g $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)
libbulkioInterfaces_la_LIBS = $(OSSIE_LIBS)

$(bulkio_builddir)/%DynSK.cpp $(bulkio_builddir)/%SK.cpp $(bulkio_builddir)/%.h: idl/ossie/BULKIO/%.idl
	$(AM_V_at)mkdir -p $(dir $@)
	$(AM_V_GEN)$(IDL) -I idl $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C $(dir $@) -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<

###############################################################################
# Python

# Per redhawk conventions, CORBA modules will be installed to a module
# <libraryname>.<lowercasemodulename>Interfaces
#
# We deviate again here in BulkIO for backwards-compatbilitiy reasons only

PYTHON_MODULE_NAME := $(LOWER_CASE_IDL_MODULE)Interfaces
PYTHON_PACKAGE := $(LOWER_CASE_IDL_MODULE).$(PYTHON_MODULE_NAME)
PYTHON_BASE := $(subst .,/,$(PYTHON_PACKAGE))
PY_SRC_OUT_DIR := $(addprefix src/python/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_IDL_SRCS := $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES) $(IDL_INTERNAL_FILES))
PY_IDL_EXTERNS := $(addprefix -Wbextern=, $(subst ,, $(PYTHON_EXTERNS)))

PY_IDL_INCLUDES := -I idl

$(PY_SRC_OUT_DIR)/__init__.py: $(PY_IDL_SRCS)
	$(AM_V_at)mkdir -p $(PY_SRC_OUT_DIR)
	$(AM_V_GEN)$(IDL) $(PY_IDL_INCLUDES) $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C src/python -bpython -Wbpackage=$(PYTHON_PACKAGE) $(PY_IDL_EXTERNS) $^
	$(AM_V_at)touch $@

# Mimic automake silent rules
OSSIE_V_pysetup = $(ossie__v_pysetup_$(V))
ossie__v_pysetup_ = $(ossie__v_pysetup__$(AM_DEFAULT_VERBOSITY))
ossie__v_pysetup_0 = --quiet
ossie__v_pysetup__0 = $(ossie__v_pysetup_0)

all-python: $(PY_SRC_OUT_DIR)/__init__.py

install-python: $(PY_SRC_OUT_DIR)/__init__.py setup.py
	python setup.py install -f --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix)

clean-python:
	rm -rf src/python
	python setup.py clean

###############################################################################
# Java
if HAVE_JAVASUPPORT

@rh_idlj_rules@
@rh_jarfile_rules@

# BUNDLE_PREFIX - Prefix at the start of the bundle symbolic name
BUNDLE_PREFIX = idl

# BUNDLE_SYMBOLIC_NAME - Used by OSGI, this is the identifer of the bundle
BUNDLE_SYMBOLIC_NAME := $(BUNDLE_PREFIX).$(LOWER_CASE_IDL_MODULE)

# BUNDLE_VERSION - Version of the OSGI Java Bundle produced, should be related to the version of the IDL,
BUNDLE_VERSION := $(PACKAGE_VERSION)

# IDLJ_SRC_DEST - Destination for files generated by IDLJ
IDLJ_SRC_DEST := src/java

# IDL_EXTERNS_LOWER - Translates the external required IDJLs to lower case
IDL_EXTERNS_LOWER := $(shell echo $(IDL_EXTERNS) | tr A-Z a-z) 

REQUIRE_BUNDLES := $(shell echo $(IDL_EXTERNS_LOWER) | sed 's/^/$(BUNDLE_PREFIX)./' | sed 's/\//./g' | sed 's/$$/;visibility:=reexport/' | sed -e '$$ ! s/$$/,/')

clean-java:
	rm -rf src/java

build/java/META-INF/MANIFEST.MF: Makefile.am BULKIOInterfaces.filelist
	@mkdir -p build/java/META-INF
	@rm -f $@
	@echo "Bundle-ManifestVersion: 2" >> $@
	@echo "Bundle-Name: $(BUNDLE_NAME)" >> $@
	@echo "Bundle-SymbolicName: $(BUNDLE_SYMBOLIC_NAME)" >> $@
	@echo "Bundle-Version: $(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@
	@echo "Bundle-RequiredExecutionEnvironment: $(BUNDLE_REQUIRED_EXECUTION_ENVIRONMENT)" >> $@
	@echo "Bundle-Vendor: $(BUNDLE_VENDOR)" >> $@
	@echo "Require-Bundle: $(REQUIRE_BUNDLES)" >> $@
	@echo -n "Export-Package:" >> $@
	@find $(IDLJ_SRC_DEST) -mindepth 1 -type d | sed 's/src\/java\///' | sed 's/\//./g' | sed 's/^/ /' | sed -e '$$ ! s/$$/,/' >> $@

java_JARFILES = BULKIOInterfaces.jar

IDLJFLAGS = -i idl -i $(OSSIE_IDLDIR) -I $(OMNICOS_IDLDIR) -I $(OMNIORB_IDLDIR)
IDLJNIFLAGS = -I idl -I $(OSSIE_IDLDIR) -I $(OMNICOS_IDLDIR) -I $(OMNIORB_IDLDIR) -Wblibname=bulkiojni
IDLJ_BUILDDIR = $(IDLJ_SRC_DEST)
IDLJNI_BUILDDIR = $(IDLJ_BUILDDIR)

# For IDLJ/IDLJNI rules, set VPATH for .idl files so that pattern rules can
# find them.
vpath %.idl idl/ossie/BULKIO
# Only include internal interfaces in pure Java; do not build them for Java-
# side JNI, because we will not be generating the C++-side
idlj_IDLSRC = $(IDL_FILES) $(IDL_INTERNAL_FILES)
idljni_IDLSRC = $(IDL_FILES)

BULKIOInterfaces_jar_SOURCE = $(idlj_SOURCE) $(idljni_SOURCE)
BULKIOInterfaces_jar_CLASSPATH = $(OSSIE_CLASSPATH)
BULKIOInterfaces_jar_MANIFEST = build/java/META-INF/MANIFEST.MF
BULKIOInterfaces_jar_JAVACFLAGS = -g
BULKIOInterfaces_jar_JARADD = -C $(IDLJ_SRC_DEST) .

CLEANFILES += $(BULKIOInterfaces_jar_MANIFEST)

# JNI library must be built after the current directory (see SUBDIRS above)
SUBDIRS += jni

###############################################################################
# No Java - stubs
else

clean-java:
	@echo "Java support is disabled"
endif


###############################################################################
# General

zip-src:
	zip 	$(LOWER_CASE_IDL_MODULE)Interfaces.zip \
		$(IDL_FILES) \
		Makefile.am \
		configure.ac \
		reconf \
		reconf.launch \
		.project \
		.autotools \
		.cproject \
		.project \
		.settings/org.eclipse.cdt.managedbuilder.core.prefs \
		README


reallyclean: clean
	rm -f configure
	rm -f Makefile.in
	rm -f config.log
	rm -f depcomp
	rm -f config.guess
	rm -f config.sub
	rm -f ltmain.sh
	rm -rf .deps
	rm -f Makefile
	rm -f config.status
	rm -f libtool
	rm -rf acinclude.m4
	rm -f missing
	rm -rf aclocal.m4
	rm -f install-sh
	rm -rf autom4te.cache
	rm -f *.lo
	rm -f *.o
	rm -rf src
