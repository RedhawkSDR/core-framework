writeonly_java.jar$(EXEEXT): $(writeonly_java_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(OSSIE_CLASSPATH):../../../../dom/deps/java_dep1/java/java_dep1.jar -d bin $(writeonly_java_jar_SOURCES)
	$(JAR) cf ./writeonly_java.jar -C bin .

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

ossieName = writeonly_java
bindir = $(prefix)/dev/devices/writeonly_java/java/
noinst_PROGRAMS = writeonly_java.jar
writeonly_java_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dev/devices/writeonly_java/

devdir = $(prefix)/dev/devices/writeonly_java/java/
