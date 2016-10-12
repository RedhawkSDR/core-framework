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
# As a Redhawk convention this should always be 'redhawk'
LIBRARY_NAME = redhawk

# Include default settings from the IDE
# You may comment this line out and replace all _auto variables
# if you want manual control of the build process
include Makefile.am.ide

# IMPORTANT: Per REDHAWK conventions, there must be one and only
# one CORBA module/namespace declared in a given IDL Interfaces
# library.  By convention, this name is capitalized and concise.
# The module can have sub-modules.
IDL_MODULE := $(redhawk_IDL_MODULE_auto)

# A space separated list of IDL files to include in your library
IDL_FILES := $(redhawk_IDL_FILES_auto)

# External IDL namespaces that your IDLs reference, comment this line
# out if your IDL has no external references
IDL_EXTERNS := CF BULKIO

# In some cases, you may need to define python externs 
#     f:p   Assume Python stub file for file f is in package p.
PYTHON_EXTERNS := PortTypes:ossie.cf cf:ossie.cf bulkioDataTypes:bulkio.bulkioInterfaces LogInterfaces:ossie.cf CosNaming:omniORB.COS

# Relative path to the location of the IDL files
IDL_SOURCE_PATH := .

# A Qualifier to append to the version, by default this is a timestamp of the build
BUNDLE_QUALIFIER := v$(shell date +%Y%m%d%H%M%S)

# A user understandable name for the bundle
BUNDLE_NAME := $(IDL_MODULE) IDL Interface

# The Vendor that produced the bundle / idl
BUNDLE_VENDOR = REDHAWK

# Version of java the source files support
JAVA_SOURCE_VERSION = 1.5

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
lib_LTLIBRARIES = libfrontendInterfaces.la
libfrontendInterfaces_la_SOURCES = $(redhawk_IDL_SOURCES_auto)
libfrontendInterfaces_la_LDFLAGS = -version-info 1:0:1

###############################################################################
# DO NOT MODIFY ANY LINES BELOW HERE
###############################################################################
.PHONY: all-local all-python install-python clean-python all-java install-java clean-java compile-java zip-src
.INTERMEDIATE: build/java.files build/java.export setup.py 
LOWER_CASE_IDL_MODULE :=$(shell echo $(IDL_MODULE) | tr A-Z a-z)

idldir = $(datadir)/idl/$(LIBRARY_NAME)/$(IDL_MODULE)
dist_idl_DATA =  $(addprefix build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))

all-local: all-python all-java

install-exec-hook: install-python install-java install-java-src

clean-local: clean-python clean-java clean-cpp
	rm -rf build

build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl: $(IDL_SOURCE_PATH)/%.idl
	mkdir -p build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)
	cp $< $@

# Always build the current directory first (this is hack-ish, but the
# # alternative is to combine the Makefile.am's)
SUBDIRS = .

SUBDIRS += libsrc

###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = $(PACKAGE_NAME).pc

pkgincludedir = $(includedir)/$(LIBRARY_NAME)/$(IDL_MODULE)
pkginclude_HEADERS = $(filter %.h, $(lib$(LOWER_CASE_IDL_MODULE)Interfaces_la_SOURCES))

AM_CXXFLAGS = -Wall -I src/cpp -g $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)
AM_LIBS = $(OSSIE_LIBS)

src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%DynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%SK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%.h: build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl
	@mkdir -p "src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)"
	$(IDL) -I build/idl $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE) -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<

clean-cpp:
	rm -rf src/cpp
	rm -f $(PACKAGE_NAME).pc

$(PACKAGE_NAME).pc: Makefile.am
	@echo "prefix=$(prefix)" > $@
	@echo "exec_prefix=$(exec_prefix)" >> $@
	@echo "libdir=$(libdir)" >> $@
	@echo "includedir=$(includedir)" >> $@
	@echo "" >> $@
	@echo "Name: $(PACKAGE_NAME)" >> $@
	@echo "Description: $(PACKAGE_NAME) for REDHAWK" >> $@
	@echo "Requires:" >> $@
	@echo "Version: $(PACKAGE_VERSION)" >> $@
	@echo "Libs: -L\$${libdir} -l${LOWER_CASE_IDL_MODULE}Interfaces" >> $@
	@echo "Cflags: -I\$${includedir}/$(LIBRARY_NAME)" >> $@

###############################################################################
# Python

# Per redhawk conventions, CORBA modules will be installed to a module
# <libraryname>.<lowercasemodulename>Interfaces

PYTHON_MODULE_NAME := $(LOWER_CASE_IDL_MODULE)Interfaces
PYTHON_PACKAGE := $(LIBRARY_NAME).$(PYTHON_MODULE_NAME)
PYTHON_BASE := $(subst .,/,$(PYTHON_PACKAGE))
PY_SRC_OUT_DIR := $(addprefix src/python/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_SRCS := $(addprefix $(PY_SRC_OUT_DIR)/, $(subst .idl,_idl.py, $(IDL_FILES)))
PY_BUILD_OUT_DIR := $(addprefix build/lib/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_BUILT_SRCS := $(addprefix $(PY_BUILD_OUT_DIR)/, $(subst .idl,_idl.py, $(IDL_FILES)))
PY_IDL_EXTERNS := $(addprefix -Wbextern=, $(subst ,, $(PYTHON_EXTERNS)))

PY_IDL_INCLUDES := -I build/idl

$(PY_SRC_OUT_DIR)/%_idl.py: build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl
	mkdir -p $(PY_SRC_OUT_DIR)
	$(IDL) $(PY_IDL_INCLUDES) $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -I$(OMNICOS_IDLDIR) -I$(OMNIORB_IDLDIR) -C src/python -bpython -Wbpackage=$(PYTHON_PACKAGE) $(PY_IDL_EXTERNS) $<

setup.py: Makefile.am
# Build up a setup.py line-by-line
	@echo "#!/usr/bin/env python" > $@
	@echo "# AUTO-GENERATED - DO NOT MODIFY THIS FILE IS PRODUCED BY make" >> $@
	@echo "from distutils.core import setup" >> $@
	@echo "import os" >> $@
	@echo "" >> $@
	@echo "_packages = ['$(PYTHON_PACKAGE)'," >> $@
	@echo "            ]" >> $@
	@echo "" >> $@
	@echo "base = '$(PYTHON_BASE)'" >> $@
	@echo "path = os.path.join('src/python',base)" >> $@
	@echo "pkgs = []" >> $@
	@echo "for root, dirs, files in os.walk(path):" >> $@
	@echo "  for f in files:" >> $@
	@echo "    if f == '__init__.py' and root!=path:" >> $@
	@echo "      tmp = os.path.join(base, root[len(path)+1:])" >> $@
	@echo "      if tmp not in pkgs: pkgs.append(tmp)" >> $@
	@echo "" >> $@
	@echo "for pkg in pkgs: _packages.append(pkg)" >> $@
	@echo "" >> $@
	@echo "setup(" >> $@
	@echo "      name='$(PYTHON_MODULE_NAME)'," >> $@
	@echo "      version='$(PACKAGE_VERSION)'," >> $@
	@echo "      package_dir = {'': 'src/python'}," >> $@
	@echo "      packages=_packages" >> $@
	@echo ")" >> $@

$(PY_BUILD_OUT_DIR): $(PY_SRCS) setup.py
	python setup.py build --build-lib build/lib
	touch $(PY_BUILD_OUT_DIR)

all-python: $(PY_BUILD_OUT_DIR)

install-python: $(PY_BUILT_SRCS) setup.py
	python setup.py install -f --$(PYTHON_INSTALL_SCHEME)=$(DESTDIR)$(prefix)

clean-python: setup.py
	rm -rf src/python
	python setup.py clean
	rm setup.py

###############################################################################
# Java
if HAVE_JAVASUPPORT

# BUNDLE_PREFIX - Prefix at the start of the bundle symbolic name
BUNDLE_PREFIX = idl

# BUNDLE_SYMBOLIC_NAME - Used by OSGI, this is the identifer of the bundle
BUNDLE_SYMBOLIC_NAME := $(BUNDLE_PREFIX).$(LOWER_CASE_IDL_MODULE)

# JAVA_OUTPUT_JAR - The name of the jar file to export the idl bundle into
JAVA_OUTPUT_JAR := $(IDL_MODULE)Interfaces.jar
#JAVA_OUTPUT_JAR := $(BUNDLE_SYMBOLIC_NAME).jar

# JAVA_SRC_JAR - The jar that contains the src code that was generated by IDLJ
JAVA_SRC_JAR := $(IDL_MODULE)Interfaces.src.jar
#JAVA_SRC_JAR := $(BUNDLE_SYMBOLIC_NAME).src.jar

# BUNDLE_VERSION - Version of the OSGI Java Bundle produced, should be related to the version of the IDL,
BUNDLE_VERSION := $(PACKAGE_VERSION)

# IDLJ_INCLUDES - The IDL include path used by IDLJ
IDLJ_INCLUDES := -i build/idl -i ${OSSIE_HOME}/share/idl -i /usr/share/idl -i /usr/share/idl/omniORB

# IDLJ_SRC_DEST - Destination for files generated by IDLJ
IDLJ_SRC_DEST := src/java

# IDL_TO_JAVA_TARGETS - Markers for IDL conversions
IDL_TO_JAVA_TARGETS := \
	$(addprefix src/java/, \
		$(subst .idl,.idlj, $(IDL_FILES)) \
 	)

# JAVA_SRCS - Temp file for holding source file lists
JAVA_SRCS = build/java.files

# JAVA_CP - Setup the classpath used in java compilation
JAVA_CP := \
	$(if $(IDL_EXTERNS), \
		-cp $(subst : ,:, \
			$(addprefix $(OSSIE_HOME)/lib/, \
				$(addsuffix Interfaces.jar:, $(IDL_EXTERNS))\
 			)\
 		),\
	)

# IDL_EXTERNS_LOWER - Translates the external required IDJLs to lower case
IDL_EXTERNS_LOWER := $(shell echo $(IDL_EXTERNS) | tr A-Z a-z) 

REQUIRE_BUNDLES := $(shell echo $(IDL_EXTERNS_LOWER) | sed 's/^/$(BUNDLE_PREFIX)./' | sed 's/\//./g' | sed 's/$$/;visibility:=reexport/' | sed -e '$$ ! s/$$/,/')

$(IDL_TO_JAVA_TARGETS): src/java/%.idlj: build/idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl
	@mkdir -p src/java
	$(IDLJ) -fall -td $(IDLJ_SRC_DEST) $(IDLJ_INCLUDES) $<
	$(IDLJ) -fallTie -td $(IDLJ_SRC_DEST) $(IDLJ_INCLUDES) $<
	touch $@

all-java: $(JAVA_OUTPUT_JAR) $(JAVA_SRC_JAR)

clean-java:
	rm -rf src/java
	rm -f $(JAVA_OUTPUT_JAR)
	rm -f $(JAVA_SRC_JAR)

build/java.files: $(IDL_TO_JAVA_TARGETS)
	@mkdir -p build/java
	find src/java -name '*.java' > $@
	$(JAVAC) $(JAVA_CP) -target $(JAVA_TARGET_VERSION) -source $(JAVA_SOURCE_VERSION) -d build/java @$@

build/java.export: build/java.files 
	@mkdir -p build/java
	@echo -n "Export-Package:" > $@
	find src/java -mindepth 1 -type d | sed 's/src\/java\///' | sed 's/\//./g' | sed 's/^/ /' | sed -e '$$ ! s/$$/,/' >> $@

build/java/META-INF/MANIFEST.MF: Makefile.am build/java.export
	@mkdir -p build/java/META-INF
	@rm -f $@
	@echo "Manifest-Version: 1.0" >> $@
	@echo "Bundle-ManifestVersion: 2" >> $@
	@echo "Bundle-Name: $(BUNDLE_NAME)" >> $@
	@echo "Bundle-SymbolicName: $(BUNDLE_SYMBOLIC_NAME)" >> $@
	@echo "Bundle-Version: $(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@
	@echo "Bundle-RequiredExecutionEnvironment: $(BUNDLE_REQUIRED_EXECUTION_ENVIRONMENT)" >> $@
	@echo "Bundle-Vendor: $(BUNDLE_VENDOR)" >> $@
	@echo "Require-Bundle: $(REQUIRE_BUNDLES)" >> $@
	@cat build/java.export >> $@
 
$(JAVA_OUTPUT_JAR): build/java.files build/java/META-INF/MANIFEST.MF
	@mkdir -p build/java
	${JAR} cMf $@ -C build/java META-INF/MANIFEST.MF -C build/java $(IDL_MODULE)

src/java/META-INF/MANIFEST.MF: Makefile.am
	@mkdir -p src/java/META-INF
	@rm -f $@
	@echo "Manifest-Version: 1.0" >> $@
	@echo "Bundle-ManifestVersion: 2" >> $@
	@echo "Bundle-Name: $(BUNDLE_NAME) Source" >> $@
	@echo "Bundle-SymbolicName: $(BUNDLE_SYMBOLIC_NAME).src" >> $@
	@echo "Bundle-Version: $(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@
	@echo "Bundle-Vendor: $(BUNDLE_VENDOR)" >> $@
	@echo "Eclipse-SourceBundle: $(BUNDLE_SYMBOLIC_NAME);version=$(BUNDLE_VERSION).$(BUNDLE_QUALIFIER)" >> $@

$(JAVA_SRC_JAR): build/java.files src/java/META-INF/MANIFEST.MF
	${JAR} cMf $@ -C $(IDLJ_SRC_DEST) .

install-java: $(JAVA_OUTPUT_JAR)
	cp $< $(DESTDIR)$(prefix)/lib

install-java-src: $(JAVA_SRC_JAR)
	cp $< $(DESTDIR)$(prefix)/lib

###############################################################################
# No Java - stubs
else

all-java install-java install-java-src clean-java:
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
