#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

@rh_jarfile_rules@

TESTS = Bulkio
check_SCRIPTS = Bulkio

NUMERIC_TYPES = Char Octet Short UShort Long ULong LongLong ULongLong Float Double
ALL_TYPES = $(NUMERIC_TYPES) Bit File XML

# Tests are generated via sed for ease of maintenance.
%PortTest.java : PortTest.java.template
	$(AM_V_GEN)sed -e 's/@name@/$*/g' -e 's/@base@/$(TEST_BASE)$*/g' $< > $@

impl/In%PortTestImpl.java : impl/InPortTestImpl.java.template
	$(AM_V_GEN)sed 's/@name@/$*/g' $< > $@

impl/Out%PortTestImpl.java : impl/OutPortTestImpl.java.template
	$(AM_V_GEN)sed 's/@name@/$*/g' $< > $@

impl/ChunkingOut%PortTestImpl.java : impl/ChunkingOutPortTestImpl.java.template
	$(AM_V_GEN)sed 's/@name@/$*/g' $< > $@

# Test helpers and port stubs are generated for numeric types.
helpers/%TestHelper.java : sed/%.sed helpers/TestHelper.java.template
	$(AM_V_GEN)sed -f $^ > $@

stubs/In%PortStub.java : sed/%.sed stubs/InPortStub.java.template
	$(AM_V_GEN)sed -f $^ > $@

# Override the test base class for numeric classes to include the extended
# tests for data chunking
NUMERIC_TESTS = $(patsubst %,Out%PortTest.java,$(NUMERIC_TYPES))
$(NUMERIC_TESTS) : TEST_BASE=Chunking

BUILT_SOURCES = $(patsubst %,impl/In%PortTestImpl.java,$(ALL_TYPES))
BUILT_SOURCES += $(patsubst %,impl/Out%PortTestImpl.java,$(ALL_TYPES))
BUILT_SOURCES += $(patsubst %,impl/ChunkingOut%PortTestImpl.java,$(NUMERIC_TYPES))
BUILT_SOURCES += $(patsubst %,helpers/%TestHelper.java,$(NUMERIC_TYPES))
BUILT_SOURCES += $(patsubst %,stubs/In%PortStub.java,$(NUMERIC_TYPES))

BUILT_SOURCES += $(patsubst %,In%PortTest.java,$(ALL_TYPES))
BUILT_SOURCES += $(patsubst %,Out%PortTest.java,$(ALL_TYPES))

noinst_java_JARFILES = bulkio-tests.jar

bulkio_tests_jar_SOURCE = $(BUILT_SOURCES)
bulkio_tests_jar_SOURCE += stubs/Packet.java
bulkio_tests_jar_SOURCE += stubs/InBitPortStub.java
bulkio_tests_jar_SOURCE += stubs/InFilePortStub.java
bulkio_tests_jar_SOURCE += stubs/InXMLPortStub.java
bulkio_tests_jar_SOURCE += helpers/BitTestHelper.java
bulkio_tests_jar_SOURCE += helpers/FileTestHelper.java
bulkio_tests_jar_SOURCE += helpers/XMLTestHelper.java
bulkio_tests_jar_SOURCE += helpers/ConnectionListener.java
bulkio_tests_jar_SOURCE += AllTests.java
bulkio_tests_jar_SOURCE += InSDDSPort_Test.java BulkioHelpers_Test.java
bulkio_tests_jar_SOURCE += OutSDDSPort_Test.java 
bulkio_tests_jar_CLASSPATH = $(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH):.
bulkio_tests_jar_JAVACFLAGS = -g -Xlint

Bulkio : bulkio-tests.jar Makefile.am
	@echo "#!/bin/bash" > $@
	@echo "if [[ \$$# == 0 ]]; then" >> $@
	@echo "  tests=AllTests" >> $@
	@echo "else" >> $@
	@echo "  tests=\"\$$*\"" >> $@
	@echo "fi" >> $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp bulkio-tests.jar:$(bulkio_tests_jar_CLASSPATH) -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore \$$tests" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES)
