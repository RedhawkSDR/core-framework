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

include $(top_srcdir)/aminclude/jarfile.am

TESTS = test_ossie
check_SCRIPTS = test_ossie

noinst_java_JARFILES = ossie-tests.jar

ossie_tests_jar_SOURCE = Main.java
ossie_tests_jar_SOURCE += AllTests.java
ossie_tests_jar_SOURCE += MessagingTest.java
ossie_tests_jar_SOURCE += PropertyTest.java
ossie_tests_jar_SOURCE += PortManager.java
ossie_tests_jar_SOURCE += utils/Assert.java
ossie_tests_jar_SOURCE += utils/ChainFilter.java
ossie_tests_jar_SOURCE += utils/TestFilter.java
ossie_tests_jar_SOURCE += utils/TextListener.java

ossie_tests_jar_CLASSPATH=$(OSSIE_CLASSPATH):$(JUNIT_CLASSPATH):.
ossie_tests_jar_JAVACFLAGS = -g -Xlint

test_ossie : ossie-tests.jar Makefile
	@echo "#!/bin/bash" > $@
	@echo "exec java -cp ossie-tests.jar:$(ossie_tests_jar_CLASSPATH) Main \$$*" >> $@
	@chmod +x $@

CLEANFILES = test_ossie libossiecf-junit-results.xml
