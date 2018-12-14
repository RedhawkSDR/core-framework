#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

TestLargePush_jar_CLASSPATH = $(SOFTPKG_CLASSPATH):$(REDHAWK_CLASSPATH):$(BULKIO_CLASSPATH)

TestLargePush.jar$(EXEEXT): $(TestLargePush_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(TestLargePush_jar_CLASSPATH) -g -d bin $(TestLargePush_jar_SOURCES)
	$(JAR) cf ./TestLargePush.jar -C bin .
	$(JAR) uf ./TestLargePush.jar -C src .

clean-local:
	rm -rf bin

distclean-local:
	rm -rf m4
	rm -f config.*
	rm -rf autom4te.cache
	rm -f acinclude.m4
	rm -f aclocal.m4
	rm -f configure
	rm -f depcomp
	rm -f install-sh
	rm -f ltmain.sh
	rm -f Makefile.in
	rm -f missing
	rm -rf .deps

ossieName = TestLargePush
bindir = $(prefix)/dom/components/TestLargePush/java/
bin_PROGRAMS = TestLargePush.jar
TestLargePush_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dom/components/TestLargePush/
dist_xml_DATA = ../TestLargePush.scd.xml ../TestLargePush.prf.xml ../TestLargePush.spd.xml

domdir = $(prefix)/dom/components/TestLargePush/java/
dist_dom_SCRIPTS = startJava.sh
