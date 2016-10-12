PropertyChange_J1_jar_CLASSPATH = $(CLASSPATH_SOFTPKG_DEP)$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar

PropertyChange_J1.jar: $(PropertyChange_J1_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(OSSIE_CLASSPATH) -d bin $(PropertyChange_J1_jar_SOURCES)
	$(JAR) cf ./PropertyChange_J1.jar -C bin .

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



noinst_PROGRAMS = PropertyChange_J1.jar
PropertyChange_J1_jar_SOURCES := $(shell find ./src -name "*.java")
