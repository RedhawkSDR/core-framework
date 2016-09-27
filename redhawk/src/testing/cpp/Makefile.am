TESTS = test_standalone test_libossiecf

AM_CPPFLAGS = -Wall

check_PROGRAMS = $(TESTS)

test_standalone_SOURCES = test_standalone.cpp SharedBufferTest.cpp SharedBufferTest.h
test_standalone_CXXFLAGS = $(CPPUNIT_CFLAGS) -I $(top_srcdir)/base/include
test_standalone_LDFLAGS = $(CPPUNIT_LIBS)

test_libossiecf_SOURCES = test_libossiecf.cpp
test_libossiecf_SOURCES += ValueTest.cpp ValueTest.h
test_libossiecf_SOURCES += ValueSequenceTest.cpp ValueSequenceTest.h
test_libossiecf_SOURCES += PropertyMapTest.cpp PropertyMapTest.h
test_libossiecf_SOURCES += MessagingTest.cpp MessagingTest.h
test_libossiecf_SOURCES += ExecutorServiceTest.cpp ExecutorServiceTest.h
test_libossiecf_CXXFLAGS = $(CPPUNIT_CFLAGS) -I $(top_srcdir)/base/include
test_libossiecf_LDFLAGS = $(CPPUNIT_LIBS) $(top_builddir)/base/framework/libossiecf.la $(top_builddir)/base/framework/idl/libossieidl.la
