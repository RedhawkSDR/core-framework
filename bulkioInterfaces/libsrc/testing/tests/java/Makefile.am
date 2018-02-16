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
In%PortTest.java : sed/%.sed InPortTest.java.template
	$(AM_V_GEN)sed -f $^ > $@

# Numeric out port tests are generated.
Out%PortTest.java : sed/%.sed NumericOutPortTest.java.template
	$(AM_V_GEN)sed -f $^ > $@

# Explicitly disable out port test generation for bit, file and XML.
OutBitPortTest.java OutFilePortTest.java OutXMLPortTest.java : ;

# Test helpers are generated for numeric types.
helpers/%TestHelper.java : sed/%.sed helpers/NumericTestHelper.java.template
	$(AM_V_GEN)sed -f $^ > $@

# Explicitly disable helper generation for bit, file and XML.
helpers/BitTestHelper.java helpers/FileTestHelper.java helpers/XMLTestHelper.java : ;

# Most port stubs are generated.
stubs/In%PortStub.java : sed/%.sed stubs/InPortStub.java.template
	$(AM_V_GEN)sed -f $^ > $@

# Explicitly disable stub generation for XML.
stubs/stub/InXMLPortStub.java : ;


BUILT_SOURCES := $(patsubst %,helpers/%TestHelper.java,$(NUMERIC_TYPES))
BUILT_SOURCES += $(patsubst %,stubs/In%PortStub.java,$(NUMERIC_TYPES) Bit File)
BUILT_SOURCES += $(patsubst %,In%PortTest.java,$(ALL_TYPES))
BUILT_SOURCES += $(patsubst %,Out%PortTest.java,$(NUMERIC_TYPES))

noinst_java_JARFILES = bulkio-tests.jar

bulkio_tests_jar_SOURCE = $(BUILT_SOURCES)
bulkio_tests_jar_SOURCE += stubs/Packet.java
bulkio_tests_jar_SOURCE += stubs/Stub.java
bulkio_tests_jar_SOURCE += stubs/InXMLPortStub.java
bulkio_tests_jar_SOURCE += helpers/ArrayData.java
bulkio_tests_jar_SOURCE += helpers/TestHelper.java
bulkio_tests_jar_SOURCE += helpers/BitTestHelper.java
bulkio_tests_jar_SOURCE += helpers/FileTestHelper.java
bulkio_tests_jar_SOURCE += helpers/XMLTestHelper.java
bulkio_tests_jar_SOURCE += helpers/ConnectionListener.java
bulkio_tests_jar_SOURCE += utils/TestFilter.java
bulkio_tests_jar_SOURCE += utils/ChainFilter.java
bulkio_tests_jar_SOURCE += utils/TextListener.java
bulkio_tests_jar_SOURCE += AllTests.java
bulkio_tests_jar_SOURCE += Main.java
bulkio_tests_jar_SOURCE += OutBitPortTest.java
bulkio_tests_jar_SOURCE += OutFilePortTest.java
bulkio_tests_jar_SOURCE += OutXMLPortTest.java
bulkio_tests_jar_SOURCE += InSDDSPort_Test.java
bulkio_tests_jar_SOURCE += OutSDDSPort_Test.java
bulkio_tests_jar_SOURCE += PrecisionUTCTimeTest.java
bulkio_tests_jar_SOURCE += StreamSRITest.java
bulkio_tests_jar_SOURCE += impl/InPortTestImpl.java
bulkio_tests_jar_SOURCE += impl/OutPortTestImpl.java
bulkio_tests_jar_SOURCE += impl/ChunkingOutPortTestImpl.java
bulkio_tests_jar_SOURCE += impl/NumericOutPortTestImpl.java

bulkio_tests_jar_CLASSPATH = $(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH):.
bulkio_tests_jar_JAVACFLAGS = -g -Xlint

Bulkio : bulkio-tests.jar Makefile
	@echo "#!/bin/bash" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp bulkio-tests.jar:$(bulkio_tests_jar_CLASSPATH) -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt Main \$$*" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES)
