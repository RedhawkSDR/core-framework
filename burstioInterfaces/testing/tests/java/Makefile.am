#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
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
burstio_libsrc_dir=$(top_srcdir)/src
burstio_libsrc_java_dir=$(burstio_libsrc_dir)/java
BURSTIO_JARS=$(top_builddir)/src/java/burstio.jar:$(top_builddir)/src/java/BURSTIOInterfaces.jar

sed_dir = $(top_srcdir)/src/java/sed
sed_generate = $(AM_V_GEN)$(SED) -f $(sed_dir)/$*.sed $< > $@

InBurst%Port_Test.java : $(srcdir)/templates/InBurstPort_Test.template $(sed_dir)/%.sed 
	$(sed_generate)

OutBurst%Port_Test.java : $(srcdir)/templates/OutBurstPort_Test.template $(sed_dir)/%.sed 
	$(sed_generate)

Burst%Push_Test.java : $(srcdir)/templates/BurstPush_Test.template $(sed_dir)/%.sed 
	$(sed_generate)

BUILT_SOURCES = $(patsubst $(sed_dir)/%.sed,InBurst%Port_Test.java,$(wildcard $(sed_dir)/*.sed))
BUILT_SOURCES += $(patsubst $(sed_dir)/%.sed,OutBurst%Port_Test.java,$(wildcard $(sed_dir)/*.sed))
BUILT_SOURCES += $(patsubst $(sed_dir)/%.sed,Burst%Push_Test.java,$(wildcard $(sed_dir)/*.sed))

Burstio_CLASSES = Burstio_Utils_Test.class $(patsubst %.java,%.class,$(BUILT_SOURCES))
Burstio_CLASSPATH = $(BURSTIO_JARS):$(OSSIE_CLASSPATH):$(BULKIO_CLASSPATH):$(JUNIT_CLASSPATH)

TEST_CLASSES = $(patsubst %.class,%,$(Burstio_CLASSES))

TESTS = Burstio
check_SCRIPTS = Burstio

%.class : %.java
	$(AM_V_at)$(JAVAC) -d $(builddir) -cp $(Burstio_CLASSPATH) -g -Xlint $<

Burstio : $(Burstio_CLASSES) Makefile.am
	@echo "#!/bin/sh" > $@
	@echo "export LD_LIBRARY_PATH=$(top_builddir)/src/java/.libs:${LD_LIBRARY_PATH}" >> $@
	@echo "exec java -cp $(Burstio_CLASSPATH):. -Dlog4j.configuration=file:$(srcdir)/log4j_config.txt org.junit.runner.JUnitCore $(TEST_CLASSES)" >> $@
	@chmod +x $@

CLEANFILES = Burstio $(BUILT_SOURCES) *.class
