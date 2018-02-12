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

# Tests are generated via sed for ease of maintenance.
In%PortTest.java : InPortTest.java.template
	$(AM_V_GEN)sed 's/@name@/$*/g' $< > $@

Out%PortTest.java : OutPortTest.java.template
	$(AM_V_GEN)sed 's/@name@/$*/g' $< > $@

# Test helpers and port stubs are generated for numeric types.
helpers/%TestHelper.java : sed/%.sed helpers/TestHelper.java.template
	$(AM_V_GEN)sed -f $^ > $@

stubs/In%PortStub.java : sed/%.sed stubs/InPortStub.java.template
	$(AM_V_GEN)sed -f $^ > $@

BUILT_SOURCES = helpers/CharTestHelper.java stubs/InCharPortStub.java InCharPortTest.java OutCharPortTest.java
BUILT_SOURCES += helpers/OctetTestHelper.java stubs/InOctetPortStub.java InOctetPortTest.java OutOctetPortTest.java
BUILT_SOURCES += helpers/ShortTestHelper.java stubs/InShortPortStub.java InShortPortTest.java OutShortPortTest.java
BUILT_SOURCES += helpers/UShortTestHelper.java stubs/InUShortPortStub.java InUShortPortTest.java OutUShortPortTest.java
BUILT_SOURCES += helpers/LongTestHelper.java stubs/InLongPortStub.java InLongPortTest.java OutLongPortTest.java
BUILT_SOURCES += helpers/ULongTestHelper.java stubs/InULongPortStub.java InULongPortTest.java OutULongPortTest.java
BUILT_SOURCES += helpers/LongLongTestHelper.java stubs/InLongLongPortStub.java InLongLongPortTest.java OutLongLongPortTest.java
BUILT_SOURCES += helpers/ULongLongTestHelper.java stubs/InULongLongPortStub.java InULongLongPortTest.java OutULongLongPortTest.java
BUILT_SOURCES += helpers/FloatTestHelper.java stubs/InFloatPortStub.java InFloatPortTest.java OutFloatPortTest.java
BUILT_SOURCES += helpers/DoubleTestHelper.java stubs/InDoublePortStub.java InDoublePortTest.java OutDoublePortTest.java
BUILT_SOURCES += InBitPortTest.java OutBitPortTest.java
BUILT_SOURCES += InFilePortTest.java OutFilePortTest.java
BUILT_SOURCES += InXMLPortTest.java OutXMLPortTest.java

noinst_java_JARFILES = bulkio_test.jar

bulkio_test_jar_SOURCE = $(BUILT_SOURCES)
bulkio_test_jar_SOURCE += stubs/Packet.java
bulkio_test_jar_SOURCE += stubs/InBitPortStub.java
bulkio_test_jar_SOURCE += stubs/InFilePortStub.java
bulkio_test_jar_SOURCE += stubs/InXMLPortStub.java
bulkio_test_jar_SOURCE += helpers/BitTestHelper.java
bulkio_test_jar_SOURCE += helpers/FileTestHelper.java
bulkio_test_jar_SOURCE += helpers/XMLTestHelper.java
bulkio_test_jar_SOURCE += AllTests.java
bulkio_test_jar_SOURCE += InVectorPort_Test.java InStringPort_Test.java  InSDDSPort_Test.java BulkioHelpers_Test.java
bulkio_test_jar_SOURCE += OutVectorPort_Test.java OutStringPort_Test.java OutSDDSPort_Test.java 

bulkio_test_jar_CLASSPATH = $(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH):.
bulkio_test_jar_JAVACFLAGS = -g -Xlint

Bulkio : bulkio_test.jar Makefile.am
	@echo "#!/bin/bash" > $@
	@echo "if [[ \$$# == 0 ]]; then" >> $@
	@echo "  tests=AllTests" >> $@
	@echo "else" >> $@
	@echo "  tests=\"\$$*\"" >> $@
	@echo "fi" >> $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp bulkio_test.jar:$(bulkio_test_jar_CLASSPATH) -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore \$$tests" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES)
