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

# DO NOT PUT ANY CORBA DEPENDENCIES INTO THE PARSER LIBRARY!!!

noinst_LTLIBRARIES = libossieparser.la
libossieparser_la_SOURCES = Properties.cpp \
			    SoftPkg.cpp \
			    PropertyRef.cpp \
			    DomainManagerConfiguration.cpp \
			    ComponentDescriptor.cpp \
			    DeviceManagerConfiguration.cpp \
			    SoftwareAssembly.cpp \
			    componentProfile.cpp \
			    ParserLogs.cpp \
			    internal/prf-parser.cpp \
			    internal/dmd-parser.cpp \
			    internal/dcd-parser.cpp \
			    internal/sad-parser.cpp \
			    internal/scd-parser.cpp \
			    internal/spd-parser.cpp \
			    internal/prf-pskel.cpp \
			    internal/dmd-pskel.cpp \
			    internal/dcd-pskel.cpp \
			    internal/dpd-pskel.cpp \
			    internal/spd-pskel.cpp \
			    internal/sad-pskel.cpp \
			    internal/scd-pskel.cpp \
			    internal/profile-pskel.cpp \
			    internal/dmd-pimpl.cpp \
			    internal/dcd-pimpl.cpp \
			    internal/dpd-pimpl.cpp \
			    internal/spd-pimpl.cpp \
			    internal/sad-pimpl.cpp \
			    internal/scd-pimpl.cpp \
			    internal/prf-pimpl.cpp \
			    internal/profile-pimpl.cpp

BUILT_SOURCES = internal/dcd-pskel.cpp \
                internal/dmd-pskel.cpp \
                internal/dpd-pskel.cpp \
                internal/prf-pskel.cpp \
                internal/profile-pskel.cpp \
                internal/sad-pskel.cpp \
                internal/scd-pskel.cpp \
                internal/spd-pskel.cpp

CLEANFILES = internal/dcd-pskel.h internal/dcd-pskel.cpp \
             internal/dmd-pskel.h internal/dmd-pskel.cpp \
             internal/dpd-pskel.h internal/dpd-pskel.cpp \
             internal/prf-pskel.h internal/prf-pskel.cpp \
             internal/profile-pskel.h internal/profile-pskel.cpp \
             internal/sad-pskel.h internal/sad-pskel.cpp \
             internal/scd-pskel.h internal/scd-pskel.cpp \
             internal/spd-pskel.h internal/spd-pskel.cpp

libossieparser_la_CXXFLAGS = -Wall $(EXPAT_CFLAGS) $(BOOST_CPPFLAGS) 
libossieparser_la_LIBADD = $(EXPAT_LIBS)
libossieparser_la_LDFLAGS = $(EXPAT_LDFLAGS)

AM_CPPFLAGS = -I../include -I. -I$(top_srcdir)/base/include

XSDFLAGS = --hxx-suffix .h --cxx-suffix .cpp --xml-parser expat --output-dir internal --generate-validation

# Pattern rule to generate *-pskel files using the corresponding .xsd and .map
# files. We need to keep xmlns items in the XSD, but we have to remove them
# from the generated parser's validation (this is what the sed command is for).
internal/%-pskel.cpp: $(top_srcdir)/xml/xsd/%.xsd internal/%.map
	$(AM_V_GEN)$(XSD) cxx-parser --type-map internal/$*.map $(XSDFLAGS) $<
	$(AM_V_at)$(SED) -i 's/ns == "urn:mil:jpeojtrs:sca:$*"/ns.empty()/g' $@
	$(AM_V_at)$(SED) -i 's/"urn:mil:jpeojtrs:sca:$*"/""/g' $@

# The DPD and profile parsers aren't used by the framework
internal/dpd-pskel.cpp: $(top_srcdir)/xml/xsd/dpd.xsd
	$(AM_V_GEN)$(XSD) cxx-parser --root-element devicepkg $(XSDFLAGS) $<

internal/profile-pskel.cpp: $(top_srcdir)/xml/xsd/profile.xsd
	$(AM_V_GEN)$(XSD) cxx-parser --root-element profile $(XSDFLAGS) $<

.PHONY: generate_noop_impl generate_test_driver

generate_noop_impl:
	-$(XSD) cxx-parser --output-dir internal --root-element domainmanagerconfiguration --type-map internal/dmd.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/dmd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element properties --type-map internal/prf.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/prf.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softwarecomponent --type-map internal/scd.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/scd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softpkg --type-map internal/spd.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/spd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element deviceconfiguration --type-map internal/dcd.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/dcd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softwareassembly --type-map internal/sad.map $(XSDFLAGS) --generate-noop-impl $(top_srcdir)/xml/xsd/sad.xsd

generate_test_driver:
	rm -f internal/*-driver.cpp
	-$(XSD) cxx-parser --output-dir internal --root-element domainmanagerconfiguration --type-map internal/dmd.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/dmd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element properties --type-map internal/prf.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/prf.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softwarecomponent --type-map internal/scd.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/scd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softpkg --type-map internal/spd.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/spd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element deviceconfiguration --type-map internal/dcd.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/dcd.xsd
	-$(XSD) cxx-parser --output-dir internal --root-element softwareassembly --type-map internal/sad.map $(XSDFLAGS) --generate-test-driver $(top_srcdir)/xml/xsd/sad.xsd

clean-local:
	rm -f internal/*-driver.cpp
