ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

dev_snk_jar_CLASSPATH = $(SOFTPKG_CLASSPATH):$(REDHAWK_CLASSPATH):$(OSSIE_HOME)/lib/bulkio.jar:$(OSSIE_HOME)/lib/BULKIOInterfaces.jar

dev_snk.jar$(EXEEXT): $(dev_snk_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(dev_snk_jar_CLASSPATH) -g -d bin $(dev_snk_jar_SOURCES)
	$(JAR) cf ./dev_snk.jar -C bin .
	$(JAR) uf ./dev_snk.jar -C src .

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

ossieName = dev_snk
bindir = $(prefix)/dev/devices/dev_snk/java/
bin_PROGRAMS = dev_snk.jar
dev_snk_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dev/devices/dev_snk/
dist_xml_DATA = ../dev_snk.scd.xml ../dev_snk.prf.xml ../dev_snk.spd.xml

devdir = $(prefix)/dev/devices/dev_snk/java/
dist_dev_SCRIPTS = startJava.sh
