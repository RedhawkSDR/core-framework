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

Bulkio_CLASSES = InVectorPort_Test.class InStringPort_Test.class  InSDDSPort_Test.class BulkioHelpers_Test.class
Bulkio_CLASSES += OutVectorPort_Test.class OutStringPort_Test.class OutSDDSPort_Test.class 
Bulkio_CLASSES += MultiOutInt8_Test.class MultiOutInt16_Test.class  MultiOutInt32_Test.class MultiOutInt64_Test.class MultiOutFloat_Test.class MultiOutDouble_Test.class MultiOutUInt8_Test.class MultiOutUInt16_Test.class  MultiOutUInt32_Test.class MultiOutUInt64_Test.class

Bulkio_CLASSPATH = .:$(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH)

TEST_CLASSES = $(patsubst %.class,%,$(Bulkio_CLASSES))

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
BUILT_SOURCES += InBitPortTest.java InFilePortTest.java InXMLPortTest.java

Bulkio_CLASSES += InCharPortTest.class OutCharPortTest.class
Bulkio_CLASSES += InOctetPortTest.class OutOctetPortTest.class
Bulkio_CLASSES += InShortPortTest.class OutShortPortTest.class
Bulkio_CLASSES += InUShortPortTest.class OutUShortPortTest.class
Bulkio_CLASSES += InLongPortTest.class OutLongPortTest.class
Bulkio_CLASSES += InULongPortTest.class OutULongPortTest.class
Bulkio_CLASSES += InLongLongPortTest.class OutLongLongPortTest.class
Bulkio_CLASSES += InULongLongPortTest.class OutULongLongPortTest.class
Bulkio_CLASSES += InFloatPortTest.class OutFloatPortTest.class
Bulkio_CLASSES += InDoublePortTest.class OutDoublePortTest.class
Bulkio_CLASSES += InBitPortTest.class
Bulkio_CLASSES += InFilePortTest.class
Bulkio_CLASSES += InXMLPortTest.class

Helper_CLASSES = helpers/CharTestHelper.class stubs/InCharPortStub.class
Helper_CLASSES += helpers/OctetTestHelper.class stubs/InOctetPortStub.class
Helper_CLASSES += helpers/ShortTestHelper.class stubs/InShortPortStub.class
Helper_CLASSES += helpers/UShortTestHelper.class stubs/InUShortPortStub.class
Helper_CLASSES += helpers/LongTestHelper.class stubs/InLongPortStub.class
Helper_CLASSES += helpers/ULongTestHelper.class stubs/InULongPortStub.class
Helper_CLASSES += helpers/LongLongTestHelper.class stubs/InLongLongPortStub.class
Helper_CLASSES += helpers/ULongLongTestHelper.class stubs/InULongLongPortStub.class
Helper_CLASSES += helpers/FloatTestHelper.class stubs/InFloatPortStub.class
Helper_CLASSES += helpers/DoubleTestHelper.class stubs/InDoublePortStub.class
Helper_CLASSES += helpers/BitTestHelper.class
Helper_CLASSES += helpers/FileTestHelper.class
Helper_CLASSES += helpers/XMLTestHelper.class

%.class : %.java
	$(AM_V_at)$(JAVAC) -d $(builddir) -cp $(Bulkio_CLASSPATH) -g -Xlint $<

Bulkio : $(Bulkio_CLASSES) $(Helper_CLASSES) Makefile.am
	@echo "#!/bin/sh" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp $(Bulkio_CLASSPATH):. -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore $(TEST_CLASSES)" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES) *.class
