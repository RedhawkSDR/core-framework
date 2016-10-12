
burstio_top_dir=../../../
burstio_idl_dir=$(burstio_top_dir)
burstio_idl_java_dir=$(burstio_top_dir)
burstio_libsrc_dir=$(burstio_top_dir)/src
burstio_libsrc_java_dir=$(burstio_libsrc_dir)/java
OSSIE_HOME=$(shell echo $(OSSIEHOME))
BURSTIO_JARS=$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(OSSIE_HOME)/lib/bulkio.jar:$(OSSIE_HOME)/lib/BULKIOInterfaces.jar:$(burstio_libsrc_java_dir)/burstio.jar:$(burstio_libsrc_java_dir)/BURSTIOInterfaces.jar 

JAVA = /usr/java/latest/bin/java
JDB = /usr/java/latest/bin/jdb
JAVAC = /usr/java/latest/bin/javac
JAVA_HOME = /usr/java/latest
JAVA_CP=$(CLASSPATH):.:/usr/share/java/junit4.jar:$(BURSTIO_JARS)

.SUFFIXES:  .java .class
.PHONEY: all check build-all clean tcheck

IN_PORTS=$(patsubst %.java,%.class,$(wildcard InBurst*.java))
OUT_PORTS=$(patsubst %.java,%.class,$(wildcard OutBurst*.java))
PUSH_TESTS=$(patsubst %.java,%.class,$(wildcard Burst*Push*.java))
UTILS=Burstio_Utils_Test.class
#MULTIOUT_PORTS=$(patsubst %.java,%.class,$(wildcard Multi*.java))

JTESTS=$(IN_PORTS:.class=) $(OUT_PORTS:.class=) $(PUSH_TESTS:.class=)  $(MULTIOUT_PORTS:.class=) $(UTILS:.class=) 
.java.class:
	$(JAVAC) -cp $(JAVA_CP) -Xlint $^

all: build-all check 

build-all: $(IN_PORTS) $(OUT_PORTS) $(PUSH_TESTS) $(MULTIOUT_PORTS) $(UTILS)

tcheck:
	$(JAVA) -cp $(JAVA_CP)   org.junit.runner.JUnitCore OutVectorPort_Test

check:
	@for jtest in "$(JTESTS)" ; do \
		$(JAVA) -cp $(JAVA_CP) -Dlog4j.configuration=file:log4j_config.txt org.junit.runner.JUnitCore $$jtest ; \
	done

debug:
	@for jtest in "$(JTESTS)" ; do \
		$(JDB) -sourcepath $(burstio_libsrc_java_dir)/BURSTIO:$(burstio_libsrc_java_dir)/burstio -classpath $(JAVA_CP) -Dlog4j.configuration=file:log4j_config.txt org.junit.runner.JUnitCore $$jtest ; \
	done

clean:
	-rm *.class
	-rm InBurst*_Test.java
	-rm OutBurst*_Test.java
	-rm Burst*Push_Test.java
	-rm Burstio_Utils_Test.java

