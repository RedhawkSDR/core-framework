devj.jar$(EXEEXT): $(devj_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(OSSIE_CLASSPATH):../../../../dom/deps/java_dep1/java/java_dep1.jar -d bin $(devj_jar_SOURCES)
	$(JAR) cf ./devj.jar -C bin .

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

ossieName = devj
bindir = $(prefix)/dev/devices/devj/java/
noinst_PROGRAMS = devj.jar
devj_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dev/devices/devj/

devdir = $(prefix)/dev/devices/devj/java/
