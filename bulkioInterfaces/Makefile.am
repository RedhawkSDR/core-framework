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
bio_dataSDDS.idl \
bio_dataVITA49.idl

# External IDL namespaces that your IDLs reference, comment this line
# out if your IDL has no external references
IDL_EXTERNS := CF

# In some cases, you may need to define python externs 
#     f:p   Assume Python stub file for file f is in package p.
PYTHON_EXTERNS := PortTypes:ossie.cf DataType:ossie.cf Port:ossie.cf QueryablePort:ossie.cf bulkioDataTypes:bulkio.bulkioInterfaces

# Relative path to the location of the IDL files
IDL_SOURCE_PATH := .

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

# Again, we don't use the auto variable because order is important to us
libbulkioInterfaces_la_SOURCES = \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bulkioDataTypesSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bulkioDataTypesDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bulkioDataTypes.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_runtimeStatsSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_runtimeStatsDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_runtimeStats.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_updateSRISK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_updateSRIDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_updateSRI.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFloatSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFloatDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFloat.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFileSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFileDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataFile.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataXMLSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataXMLDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataXML.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataShortSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataShortDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataShort.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataDoubleSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataDoubleDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataDouble.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataCharSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataCharDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataChar.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataOctetSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataOctetDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataOctet.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlongSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlongDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlong.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUshortSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUshortDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUshort.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLongSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLongDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLong.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLongLongSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLongLongDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataLongLong.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlongLongSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlongLongDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataUlongLong.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataSDDSSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataSDDSDynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataSDDS.h \
	src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataVITA49SK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataVITA49DynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/bio_dataVITA49.h

###############################################################################
# DO NOT MODIFY ANY LINES BELOW HERE
###############################################################################
.PHONY: all-local all-python install-python clean-python clean-java zip-src
LOWER_CASE_IDL_MODULE :=$(shell echo $(IDL_MODULE) | tr A-Z a-z)

idldir = $(datadir)/idl/$(LIBRARY_NAME)/$(IDL_MODULE)
dist_idl_DATA =  $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))

all-local: all-python

install-exec-hook: install-python

clean-local: clean-python clean-java clean-cpp
	rm -rf build

# Always build the current directory first (this is hack-ish, but the
# alternative is to combine the Makefile.am's)
SUBDIRS = .

if BUILD_BASE_CLASSES
SUBDIRS += libsrc
endif

###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = $(PACKAGE_NAME).pc

pkgincludedir = $(includedir)/$(LIBRARY_NAME)/$(IDL_MODULE)
pkginclude_HEADERS = $(filter %.h, $(lib$(LOWER_CASE_IDL_MODULE)Interfaces_la_SOURCES))

AM_CXXFLAGS = -Wall -I src/cpp -g $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)
AM_LIBS = $(OSSIE_LIBS)

src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%DynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%SK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%.h: idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl
	$(AM_V_at)mkdir -p "src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)"
	$(AM_V_GEN)$(IDL) -I idl $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE) -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<

clean-cpp:
	rm -rf src/cpp

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
PY_IDL_SRCS := $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))
PY_BUILD_OUT_DIR := $(addprefix build/lib/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_BUILT_SRCS := $(addprefix $(PY_BUILD_OUT_DIR)/, $(subst .idl,_idl.py, $(IDL_FILES)))
PY_IDL_EXTERNS := $(addprefix -Wbextern=, $(subst ,, $(PYTHON_EXTERNS)))

PY_IDL_INCLUDES := -I idl

$(PY_SRC_OUT_DIR)/__init__.py: $(PY_IDL_SRCS)
	$(AM_V_at)mkdir -p $(PY_SRC_OUT_DIR)
	$(AM_V_GEN)$(IDL) $(PY_IDL_INCLUDES) $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C src/python -bpython -Wbpackage=$(PYTHON_PACKAGE) $(PY_IDL_EXTERNS) $^

# Mimic automake silent rules
OSSIE_V_pysetup = $(ossie__v_pysetup_$(V))
ossie__v_pysetup_ = $(ossie__v_pysetup__$(AM_DEFAULT_VERBOSITY))
ossie__v_pysetup_0 = --quiet
ossie__v_pysetup__0 = $(ossie__v_pysetup_0)

$(PY_BUILD_OUT_DIR): $(PY_SRC_OUT_DIR)/__init__.py
	python setup.py $(OSSIE_V_pysetup) build --build-lib build/lib
	$(AM_V_at)touch $(PY_BUILD_OUT_DIR)

all-python: $(PY_BUILD_OUT_DIR)

install-python: $(PY_BUILT_SRCS)
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
java_DATA = BULKIOInterfaces.src.jar

IDLJFLAGS = -i idl -i $(OSSIE_IDLDIR) -I $(OMNICOS_IDLDIR) -I $(OMNIORB_IDLDIR)
IDLJNIFLAGS = -I idl -I $(OSSIE_IDLDIR) -I $(OMNICOS_IDLDIR) -I $(OMNIORB_IDLDIR) -Wblibname=bulkiojni
IDLJ_BUILDDIR = $(IDLJ_SRC_DEST)
IDLJNI_BUILDDIR = $(IDLJ_BUILDDIR)

# For IDLJ/IDLJNI rules, set VPATH for .idl files so that pattern rules can
# find them.
vpath %.idl idl/ossie/BULKIO
idlj_IDLSRC = $(IDL_FILES)
idljni_IDLSRC = $(IDL_FILES)

BULKIOInterfaces_jar_SOURCE = $(idlj_SOURCE) $(idljni_SOURCE)
BULKIOInterfaces_jar_CLASSPATH = $(OSSIE_CLASSPATH)
BULKIOInterfaces_jar_MANIFEST = build/java/META-INF/MANIFEST.MF

src/java/META-INF/MANIFEST.MF: Makefile
	@mkdir -p src/java/META-INF
	@rm -f $@
	@echo "Bundle-ManifestVersion: 2" >> $@
	@echo "Bundle-Name: $(BUNDLE_NAME) Source" >> $@
	@echo "Bundle-SymbolicName: $(BUNDLE_SYMBOLIC_NAME).src" >> $@
	@echo "Bundle-Version: $(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@
	@echo "Bundle-Vendor: $(BUNDLE_VENDOR)" >> $@
	@echo "Eclipse-SourceBundle: $(BUNDLE_SYMBOLIC_NAME);version=$(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@

CLEANFILES =  BULKIOInterfaces.src.jar src/java/META-INF/MANIFEST.MF $(BULKIOInterfaces_jar_MANIFEST)

BULKIOInterfaces.src.jar: src/java/META-INF/MANIFEST.MF $(BULKIOInterfaces_jar_SOURCE)
	$(RH_V_JAR)$(JAR) cMf $@ -C $(IDLJ_SRC_DEST) .

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
