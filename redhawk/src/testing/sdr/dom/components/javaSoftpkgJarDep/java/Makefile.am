ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

javaSoftpkgJarDep_jar_CLASSPATH = ../../../deps/java_dep1/java/java_dep1.jar:$(OSSIE_CLASSPATH):

javaSoftpkgJarDep.jar$(EXEEXT): $(javaSoftpkgJarDep_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(javaSoftpkgJarDep_jar_CLASSPATH) -g -d bin $(javaSoftpkgJarDep_jar_SOURCES)
	$(JAR) cf ./javaSoftpkgJarDep.jar -C bin .
	$(JAR) uf ./javaSoftpkgJarDep.jar -C src .

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

ossieName = javaSoftpkgJarDep
bindir = $(prefix)/dom/components/javaSoftpkgJarDep/java/
bin_PROGRAMS = javaSoftpkgJarDep.jar
javaSoftpkgJarDep_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dom/components/javaSoftpkgJarDep/
dist_xml_DATA = ../javaSoftpkgJarDep.scd.xml ../javaSoftpkgJarDep.prf.xml ../javaSoftpkgJarDep.spd.xml

domdir = $(prefix)/dom/components/javaSoftpkgJarDep/java/
dist_dom_SCRIPTS = startJava.sh
