# Rules for the test code (use `make check` to execute)
ACLOCAL_AMFLAGS = -I m4 -I ../../../acinclude
TESTS = appender_test 
noinst_PROGRAMS = cleanmem proc_log
check_PROGRAMS = $(TESTS)
logger_top=../../../base/framework/logging
logger_libsrc=$(logger_top)
#logger_lib_INC=-I$(logger_libsrc)
logger_lib_SRC=../../../base/framework/logging/RH_SyncRollingAppender.cpp
logger_lib_INC=-I../../../base/framework/logging

appender_test_SOURCES = appender_test.cpp test_suites.cpp $(logger_lib_SRC) 

appender_test_boost_ldadd=$(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) -lboost_filesystem
appender_test_log4cxx_ldadd=-llog4cxx
appender_test_LIB = 
appender_test_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(logger_top)/include $(logger_lib_INC) $(logger_idl_INC) -I/usr/include/apr-1 $(BOOST_CPPFLAGS) -DDEBUG_ON -DHAVE_LOG4CXX
appender_test_LDADD = $(appender_test_LIB) $(logger_idl_LIB) $(appender_test_boost_ldadd) $(appender_test_log4cxx_ldadd)  
appender_test_LDFLAGS = $(CPPUNIT_LIBS)
appender_test_LDFLAGS += -ldl 


proc_log_SOURCES = proc_log.cpp $(logger_lib_SRC) 
proc_log_boost_ldadd=$(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) -lboost_filesystem
proc_log_log4cxx_ldadd=-llog4cxx
proc_log_LIB = 
proc_log_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(logger_top)/include $(logger_lib_INC) $(logger_idl_INC) -I/usr/include/apr-1 $(BOOST_CPPFLAGS) -DDEBUG_ON -DHAVE_LOG4CXX
proc_log_LDADD = $(proc_log_LIB) $(logger_idl_LIB) $(proc_log_boost_ldadd) $(proc_log_log4cxx_ldadd)  
proc_log_LDFLAGS = $(CPPUNIT_LIBS)
proc_log_LDFLAGS += -ldl 


cleanmem_SOURCES = cleanmem.cpp 
cleanmem_boost_ldadd=$(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) -lboost_filesystem
cleanmem_log4cxx_ldadd=-llog4cxx
cleanmem_LIB = 
cleanmem_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(logger_top)/include $(logger_lib_INC) $(logger_idl_INC) -I/usr/include/apr-1 $(BOOST_CPPFLAGS) -DDEBUG_ON -DHAVE_LOG4CXX
cleanmem_LDADD = $(cleanmem_LIB) $(logger_idl_LIB) $(cleanmem_boost_ldadd) $(cleanmem_log4cxx_ldadd)  
cleanmem_LDFLAGS = $(CPPUNIT_LIBS)
cleanmem_LDFLAGS += -ldl 

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
	rm -f COPYING INSTALL  *~ 
	rm -rf MP_RedhawkTest*


