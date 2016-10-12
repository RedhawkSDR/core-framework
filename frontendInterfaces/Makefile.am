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
PYTHON_EXTERNS := bulkioDataTypes:bulkio.bulkioInterfaces DataType:ossie.cf

# A Qualifier to append to the version, by default this is a timestamp of the build
BUNDLE_QUALIFIER := v$(shell date +%Y%m%d%H%M%S)

# Version of java the source files support
JAVA_SOURCE_VERSION = 1.6

# Version of java to compile to
JAVA_TARGET_VERSION = $(JAVA_SOURCE_VERSION)

# You have to manually set the _SOURCES, automake/libtool does not support
# makefile variable substituion here...so unfortunately each IDL file
# needs to be listed here...for each input IDL you will need to add three
# sources.  For example:
#        some_file.idl becomes ->
#            src/cpp/some_fileSK.cpp src/cpp/some_fileDynSK.cpp src/cpp/some_file.h
lib_LTLIBRARIES = libfrontendInterfaces.la
libfrontendInterfaces_la_SOURCES = $(redhawk_IDL_SOURCES_auto)
libfrontendInterfaces_la_LIBADD = $(BULKIO_LIBS) $(OSSIE_LIBS)
libfrontendInterfaces_la_LDFLAGS = -version-info 1:0:1

###############################################################################
# DO NOT MODIFY ANY LINES BELOW HERE
###############################################################################
.PHONY: all-local all-python install-python clean-python all-java install-java clean-java compile-java zip-src
.INTERMEDIATE: build/java.files setup.py
LOWER_CASE_IDL_MODULE :=$(shell echo $(IDL_MODULE) | tr A-Z a-z)

idldir = $(datadir)/idl/$(LIBRARY_NAME)/$(IDL_MODULE)
dist_idl_DATA =  $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))

all-local: all-python all-java

install-exec-hook: install-python install-java

clean-local: clean-python clean-java clean-cpp
	rm -rf build

# Always build the current directory first (this is hack-ish, but the
# # alternative is to combine the Makefile.am's)
SUBDIRS = .

SUBDIRS += libsrc

###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = frontendInterfaces.pc

pkgincludedir = $(includedir)/$(LIBRARY_NAME)/$(IDL_MODULE)
pkginclude_HEADERS = $(filter %.h, $(lib$(LOWER_CASE_IDL_MODULE)Interfaces_la_SOURCES))

AM_CXXFLAGS = -Wall -I src/cpp -g $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)
AM_LIBS = $(OSSIE_LIBS) $(BULKIO_LIBS)

src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%DynSK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%SK.cpp src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)/%.h: idl/$(LIBRARY_NAME)/$(IDL_MODULE)/%.idl
	@mkdir -p "src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE)"
	$(IDL) -I idl $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -C src/cpp/$(LIBRARY_NAME)/$(IDL_MODULE) -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<

clean-cpp:
	rm -rf src/cpp

###############################################################################
# Python

# Per redhawk conventions, CORBA modules will be installed to a module
# <libraryname>.<lowercasemodulename>Interfaces

PYTHON_MODULE_NAME := $(LOWER_CASE_IDL_MODULE)Interfaces
PYTHON_PACKAGE := $(LIBRARY_NAME).$(PYTHON_MODULE_NAME)
PYTHON_BASE := $(subst .,/,$(PYTHON_PACKAGE))
PY_SRC_OUT_DIR := $(addprefix src/python/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_IDL_SRCS := $(PY_SRC_OUT_DIR)/__init__.py $(addprefix idl/$(LIBRARY_NAME)/$(IDL_MODULE)/, $(IDL_FILES))
PY_BUILD_OUT_DIR := $(addprefix build/lib/, $(subst .,/, $(PYTHON_PACKAGE)))
PY_BUILT_SRCS := $(addprefix $(PY_BUILD_OUT_DIR)/, $(subst .idl,_idl.py, $(IDL_FILES)))
PY_IDL_EXTERNS := $(addprefix -Wbextern=, $(subst ,, $(PYTHON_EXTERNS)))

PY_IDL_INCLUDES := -I idl

$(PY_SRC_OUT_DIR)/__init__.py: $(PY_IDL_SRCS)
	$(AM_v_at)mkdir -p $(PY_SRC_OUT_DIR)
	$(AM_v_gen)$(IDL) $(PY_IDL_INCLUDES) $(OSSIE_CFLAGS) $(OSSIE_IDLFLAGS) -C src/python -bpython -Wbpackage=$(PYTHON_PACKAGE) $(PY_IDL_EXTERNS) $^

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

$(PY_BUILD_OUT_DIR): $(PY_SRC_OUT_DIR)/__init__.py setup.py
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

@rh_idlj_rules@
@rh_jarfile_rules@

# BUNDLE_VERSION - Version of the OSGI Java Bundle produced, should be related to the version of the IDL,
BUNDLE_VERSION := $(PACKAGE_VERSION)

# IDLJ_SRC_DEST - Destination for files generated by IDLJ
IDLJ_SRC_DEST := src/java

manifest_edit = $(SED) \
  -e 's|@bundle_version[@]|$(BUNDLE_VERSION)|g' \
  -e 's|@bundle_qualifier[@]|$(BUNDLE_QUALIFIER)|g'

java_JARFILES = FRONTENDInterfaces.jar
IDLJFLAGS = -i idl -i $(OSSIE_IDLDIR)
IDLJ_BUILDDIR = $(IDLJ_SRC_DEST)

# For IDLJ rules, set VPATH for .idl files so that pattern rules can find them.
vpath %.idl idl/redhawk/FRONTEND
idlj_IDLSRC = $(IDL_FILES)

FRONTENDInterfaces_jar_SOURCE = $(idlj_SOURCE)
FRONTENDInterfaces_jar_CLASSPATH = $(OSSIE_CLASSPATH):$(BULKIO_CLASSPATH)
FRONTENDInterfaces_jar_MANIFEST = build/java/META-INF/MANIFEST.MF
FRONTENDInterfaces_jar_JARADD = -C $(IDLJ_SRC_DEST) .

build/java/META-INF/MANIFEST.MF: MANIFEST.MF.in Makefile
	@mkdir -p build/java/META-INF
	$(AM_V_GEN)$(manifest_edit) MANIFEST.MF.in > $@

CLEANFILES = build/java/META-INF/MANIFEST.MF

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
