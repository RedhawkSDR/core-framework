#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK frontendInterfaces.
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

TESTS = Frontend
check_SCRIPTS = Frontend

noinst_java_JARFILES = frontend-tests.jar

frontend_tests_jar_SOURCE = utils/TestFilter.java
frontend_tests_jar_SOURCE += utils/ChainFilter.java
frontend_tests_jar_SOURCE += utils/TextListener.java
frontend_tests_jar_SOURCE += AllTests.java
frontend_tests_jar_SOURCE += Main.java
frontend_tests_jar_SOURCE += ValidateRequestTest.java

frontend_tests_jar_CLASSPATH = $(BULKIO_CLASSPATH):$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH):../../../frontend.jar:../../../../FRONTENDInterfaces.jar:.
frontend_tests_jar_JAVACFLAGS = -g -Xlint

Frontend : frontend-tests.jar Makefile
	@echo "#!/bin/bash" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/jni/.libs:$(OSSIE_HOME)/lib:$(OSSIE_HOME)/lib64" >> $@
	@echo "exec java -cp frontend-tests.jar:$(frontend_tests_jar_CLASSPATH) -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt Main \$$*" >> $@
	@chmod +x $@

CLEANFILES = Frontend
