
DevC.jar$(EXEEXT): $(DevC_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(OSSIE_CLASSPATH) -g -d bin $(DevC_jar_SOURCES)
	$(JAR) cf ./DevC.jar -C bin .

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

ossieName = DevC
bindir = $(prefix)/dev/devices/DevC/java/
noinst_PROGRAMS = DevC.jar
DevC_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dev/devices/DevC/
devdir = $(prefix)/dev/devices/DevC/java/

