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

# Test helpers are generated for numeric types.
%TestHelper.java : sed/%.sed TestHelper.java.template
	$(AM_V_GEN)sed -f $^ > $@

BUILT_SOURCES = CharTestHelper.java InCharPortTest.java
BUILT_SOURCES += OctetTestHelper.java InOctetPortTest.java
BUILT_SOURCES += ShortTestHelper.java InShortPortTest.java
BUILT_SOURCES += UShortTestHelper.java InUShortPortTest.java
BUILT_SOURCES += LongTestHelper.java InLongPortTest.java
BUILT_SOURCES += ULongTestHelper.java InULongPortTest.java
BUILT_SOURCES += LongLongTestHelper.java InLongLongPortTest.java
BUILT_SOURCES += ULongLongTestHelper.java InULongLongPortTest.java
BUILT_SOURCES += FloatTestHelper.java InFloatPortTest.java
BUILT_SOURCES += DoubleTestHelper.java InDoublePortTest.java
BUILT_SOURCES += InBitPortTest.java InFilePortTest.java InXMLPortTest.java

Bulkio_CLASSES += InCharPortTest.class
Bulkio_CLASSES += InOctetPortTest.class
Bulkio_CLASSES += InShortPortTest.class
Bulkio_CLASSES += InUShortPortTest.class
Bulkio_CLASSES += InLongPortTest.class
Bulkio_CLASSES += InULongPortTest.class
Bulkio_CLASSES += InLongLongPortTest.class
Bulkio_CLASSES += InULongLongPortTest.class
Bulkio_CLASSES += InFloatPortTest.class
Bulkio_CLASSES += InDoublePortTest.class
Bulkio_CLASSES += InBitPortTest.class
Bulkio_CLASSES += InFilePortTest.class
Bulkio_CLASSES += InXMLPortTest.class

Helper_CLASSES = CharTestHelper.class
Helper_CLASSES += OctetTestHelper.class
Helper_CLASSES += ShortTestHelper.class
Helper_CLASSES += UShortTestHelper.class
Helper_CLASSES += LongTestHelper.class
Helper_CLASSES += ULongTestHelper.class
Helper_CLASSES += LongLongTestHelper.class
Helper_CLASSES += ULongLongTestHelper.class
Helper_CLASSES += FloatTestHelper.class
Helper_CLASSES += DoubleTestHelper.class
Helper_CLASSES += BitTestHelper.class
Helper_CLASSES += FileTestHelper.class
Helper_CLASSES += XMLTestHelper.class

%.class : %.java
	$(AM_V_at)$(JAVAC) -d $(builddir) -cp $(Bulkio_CLASSPATH) -g -Xlint $<

Bulkio : $(Bulkio_CLASSES) $(Helper_CLASSES) Makefile.am
	@echo "#!/bin/sh" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp $(Bulkio_CLASSPATH):. -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore $(TEST_CLASSES)" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES) *.class
