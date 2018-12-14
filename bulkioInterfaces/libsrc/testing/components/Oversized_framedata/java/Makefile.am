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

Oversized_framedata_jar_CLASSPATH = $(SOFTPKG_CLASSPATH):$(REDHAWK_CLASSPATH):$(BULKIO_CLASSPATH)

Oversized_framedata.jar$(EXEEXT): $(Oversized_framedata_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(Oversized_framedata_jar_CLASSPATH) -g -d bin $(Oversized_framedata_jar_SOURCES)
	$(JAR) cf ./Oversized_framedata.jar -C bin .
	$(JAR) uf ./Oversized_framedata.jar -C src .

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

ossieName = Oversized_framedata
bindir = $(prefix)/dom/components/Oversized_framedata/java/
bin_PROGRAMS = Oversized_framedata.jar
Oversized_framedata_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dom/components/Oversized_framedata/
dist_xml_DATA = ../Oversized_framedata.scd.xml ../Oversized_framedata.prf.xml ../Oversized_framedata.spd.xml

domdir = $(prefix)/dom/components/Oversized_framedata/java/
dist_dom_SCRIPTS = startJava.sh
