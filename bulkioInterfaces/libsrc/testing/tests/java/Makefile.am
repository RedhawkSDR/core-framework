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

Bulkio_CLASSPATH = $(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH)

TEST_CLASSES = $(patsubst %.class,%,$(Bulkio_CLASSES))

TESTS = Bulkio
check_SCRIPTS = Bulkio

%.class : %.java
	$(AM_V_at)$(JAVAC) -d $(builddir) -cp $(Bulkio_CLASSPATH) -g -Xlint $<

Bulkio : $(Bulkio_CLASSES) Makefile.am
	@echo "#!/bin/sh" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:${LD_LIBRARY_PATH}" >> $@
	@echo "exec java -cp $(Bulkio_CLASSPATH):. -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore $(TEST_CLASSES)" >> $@
	@chmod +x $@

CLEANFILES = Bulkio $(BUILT_SOURCES) *.class
