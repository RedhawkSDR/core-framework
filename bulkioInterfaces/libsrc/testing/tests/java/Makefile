
bulkio_top_dir=../../../..
bulkio_idl_dir=$(bulkio_top_dir)
bulkio_idl_java_dir=$(bulkio_top_dir)
bulkio_libsrc_dir=$(bulkio_top_dir)/libsrc
bulkio_libsrc_java_dir=$(bulkio_libsrc_dir)/java
OSSIE_HOME=$(shell echo $(OSSIEHOME))
BULKIO_JARS=$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(bulkio_libsrc_dir)/bulkio.jar:$(bulkio_idl_java_dir)/BULKIOInterfaces.jar 

JAVA = $(JAVA_HOME)/bin/java
JAVAC = $(JAVA_HOME)/bin/javac
JAVA_CP=$(CLASSPATH):.:/usr/share/java/junit4.jar:$(BULKIO_JARS)

.SUFFIXES:  .java .class
.PHONEY: all check build-all clean tcheck

IN_PORTS= InVectorPort_Test.class InStringPort_Test.class  InSDDSPort_Test.class BulkioHelpers_Test.class
OUT_PORTS= OutVectorPort_Test.class OutStringPort_Test.class OutSDDSPort_Test.class 
MULTIOUT_PORTS=MultiOutInt8_Test.class MultiOutInt16_Test.class  MultiOutInt32_Test.class MultiOutInt64_Test.class MultiOutFloat_Test.class MultiOutDouble_Test.class MultiOutUInt8_Test.class MultiOutUInt16_Test.class  MultiOutUInt32_Test.class MultiOutUInt64_Test.class

JTESTS=$(IN_PORTS:.class=) $(OUT_PORTS:.class=) $(MULTIOUT_PORTS:.class=)
.java.class:
	$(JAVAC) -cp $(JAVA_CP)  $^

all: build-all check 

build-all: $(IN_PORTS)  $(OUT_PORTS)  $(MULTIOUT_PORTS) 

tcheck:
	$(JAVA) -cp $(JAVA_CP)   org.junit.runner.JUnitCore OutVectorPort_Test

check:
	@for jtest in "$(JTESTS)" ; do \
		$(JAVA) -cp $(JAVA_CP) -Dlog4j.configuration=file:log4j_config.txt org.junit.runner.JUnitCore $$jtest ; \
	done

clean:
	-rm *.class
