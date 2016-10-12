Oversized_framedata_jar_CLASSPATH = $(CLASSPATH_SOFTPKG_DEP)$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(OSSIE_HOME)/lib/bulkio.jar:$(OSSIE_HOME)/lib/BULKIOInterfaces.jar

Oversized_framedata.jar: $(Oversized_framedata_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(Oversized_framedata_jar_CLASSPATH) -d bin $(Oversized_framedata_jar_SOURCES)
	$(JAR) cf ./Oversized_framedata.jar -C bin .

clean-local:
	rm -rf bin

distclean-local:
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
